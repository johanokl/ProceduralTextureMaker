#include "base/jstexgen.h"
#include <QCoreApplication>
#include <QElapsedTimer>
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
   int inputCount = 0;
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
   if (benchmark.inputCount > 0) {
      inputs.insert(QStringLiteral("First"), inputImage(size, TexturePixel(17, 63, 127, 255)));
   }
   if (benchmark.inputCount > 1) {
      inputs.insert(QStringLiteral("Second"), inputImage(size, TexturePixel(211, 157, 91, 223)));
   }
   TextureImagePtr output = TextureImage::create(size);
   TextureNodeSettings settings;
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
       {QStringLiteral("sourceCopies"), benchmark.inputCount},
       {QStringLiteral("outputCopies"), 1},
       {QStringLiteral("validationNs"), validationNanoseconds},
       {QStringLiteral("coldInvocationNs"), coldNanoseconds},
       {QStringLiteral("warmInvocationNs"), warmNanoseconds},
       {QStringLiteral("revision"), QString::fromLatin1(generator.contentRevision().toHex())}};
   QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Compact) << Qt::endl;
}

}  // namespace

int main(int argc, char** argv) {
   QCoreApplication application(argc, argv);
   const QString descriptorStart = QStringLiteral(
       "const generator={apiVersion:1,name:'Benchmark',type:'generator',inputs:%1,settings:{},"
       "generate(size,settings,output,inputs){void settings;");
   const QList<BenchmarkCase> cases{
       {QStringLiteral("fill"),
        descriptorStart.arg(QStringLiteral("[]")) +
            QStringLiteral("output.data.fill(127);void size;void inputs;}};"),
        0},
       {QStringLiteral("copy"),
        descriptorStart.arg(QStringLiteral("['First']")) +
            QStringLiteral("output.data.set(inputs.First.data);void size;}};"),
        1},
       {QStringLiteral("neighborhood"),
        descriptorStart.arg(QStringLiteral("['First']")) +
            QStringLiteral(
                "const src=inputs.First.data,dst=output.data;for(let y=1;y<size.height-1;++y)"
                "for(let x=1;x<size.width-1;++x){const p=y*output.stride+x*4;"
                "for(let c=0;c<4;++c)dst[p+c]=(src[p-4+c]+src[p+c]+src[p+4+c])/3;}}};"),
        1},
       {QStringLiteral("blend"),
        descriptorStart.arg(QStringLiteral("['First','Second']")) +
            QStringLiteral("const a=inputs.First.data,b=inputs.Second.data,d=output.data;void size;"
                           "for(let i=0;i<d.length;++i)d[i]=(a[i]+b[i])>>1;}};"),
        2}};
   const QList<int> sizes{256, 512, 1024, 2048};
   for (const BenchmarkCase& benchmark : cases) {
      for (const int size : sizes) {
         runCase(benchmark, QSize(size, size));
      }
   }
   return 0;
}
