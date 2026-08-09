#include "base/jstexgen.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTextStream>
#include <QtGlobal>
#include <algorithm>

namespace {

struct BenchmarkCase {
   QString name;
   QString script;
   QStringList inputs;
};

TextureImagePtr inputImage(const QSize size, const TexturePixel color) {
   TextureImagePtr image = TextureImage::create(size);
   std::fill_n(image->data(), image->pixelCount(), color);
   return image;
}

void runCase(const BenchmarkCase& benchmark, const QSize size) {
   QElapsedTimer timer;
   timer.start();
   JsTexGen generator(benchmark.script, QStringLiteral("<benchmark:%1>").arg(benchmark.name));
   const qint64 validationNanoseconds = timer.nsecsElapsed();
   if (!generator.isValid()) {
      QTextStream(stderr) << generator.validationError() << Qt::endl;
      return;
   }

   QMap<QString, TextureImagePtr> inputs;
   for (qsizetype index = 0; index < benchmark.inputs.size(); ++index) {
      const TexturePixel color =
          index == 0 ? TexturePixel(17, 63, 127, 255) : TexturePixel(211, 157, 91, 255);
      inputs.insert(benchmark.inputs.at(index), inputImage(size, color));
   }
   TextureNodeSettings settings;
   for (const TextureGeneratorSetting& setting : generator.getSettings()) {
      settings.insert(setting.id, setting.defaultvalue);
   }
   TextureImagePtr output = TextureImage::create(size);
   timer.restart();
   generator.generate(size, output->data(), inputs, settings);
   const qint64 coldNanoseconds = timer.nsecsElapsed();
   timer.restart();
   generator.generate(size, output->data(), inputs, settings);
   const qint64 warmNanoseconds = timer.nsecsElapsed();

   QJsonObject result{
       {QStringLiteral("case"), benchmark.name},
       {QStringLiteral("width"), size.width()},
       {QStringLiteral("height"), size.height()},
       {QStringLiteral("qtVersion"), QString::fromLatin1(qVersion())},
       {QStringLiteral("compiler"), QString::fromLatin1(__VERSION__)},
#ifdef NDEBUG
       {QStringLiteral("buildType"), QStringLiteral("release")},
#else
       {QStringLiteral("buildType"), QStringLiteral("debug")},
#endif
       {QStringLiteral("cpuArchitecture"), QSysInfo::currentCpuArchitecture()},
       {QStringLiteral("workerCount"), 1},
       {QStringLiteral("sourceCopies"), benchmark.inputs.size()},
       {QStringLiteral("outputCopies"), 1},
       {QStringLiteral("validationNs"), validationNanoseconds},
       {QStringLiteral("coldInvocationNs"), coldNanoseconds},
       {QStringLiteral("warmInvocationNs"), warmNanoseconds},
       {QStringLiteral("revision"), QString::fromLatin1(generator.contentRevision().toHex())}};
   QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Compact) << Qt::endl;
}

QString bundledGeneratorScript(const QString& name) {
   QFile file(QStringLiteral(":/generators/%1.js").arg(name));
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return {};
   }
   return QString::fromUtf8(file.readAll());
}

}  // namespace

int main(int argc, char** argv) {
   QCoreApplication application(argc, argv);
   Q_INIT_RESOURCE(generators);
   const QString descriptorStart = QStringLiteral(
       "const generator={apiVersion:1,name:'Benchmark',type:'generator',inputs:%1,settings:[],"
       "generate(size,settings,output,inputs){void settings;");
   const QList<BenchmarkCase> cases{
       {QStringLiteral("fill"),
        descriptorStart.arg(QStringLiteral("[]")) +
            QStringLiteral("output.data.fill(127);void size;void inputs;}};"),
        {}},
       {QStringLiteral("copy"),
        descriptorStart.arg(QStringLiteral("['First']")) +
            QStringLiteral("output.data.set(inputs.First.data);void size;}};"),
        {QStringLiteral("First")}},
       {QStringLiteral("neighborhood"),
        descriptorStart.arg(QStringLiteral("['First']")) +
            QStringLiteral(
                "const src=inputs.First.data,dst=output.data;for(let y=1;y<size.height-1;++y)"
                "for(let x=1;x<size.width-1;++x){const p=y*output.stride+x*4;"
                "for(let c=0;c<4;++c)dst[p+c]=(src[p-4+c]+src[p+c]+src[p+4+c])/3;}}};"),
        {QStringLiteral("First")}},
       {QStringLiteral("blend"),
        descriptorStart.arg(QStringLiteral("['First','Second']")) +
            QStringLiteral("const a=inputs.First.data,b=inputs.Second.data,d=output.data;void size;"
                           "for(let i=0;i<d.length;++i)d[i]=(a[i]+b[i])>>1;}};"),
        {QStringLiteral("First"), QStringLiteral("Second")}},
       {QStringLiteral("blending-normal-opaque"),
        bundledGeneratorScript(QStringLiteral("blending")),
        {QStringLiteral("Base"), QStringLiteral("Blend")}}};
   const QList<int> sizes{256, 512, 1024, 2048};
   for (const BenchmarkCase& benchmark : cases) {
      for (const int size : sizes) {
         runCase(benchmark, QSize(size, size));
      }
   }
   return 0;
}
