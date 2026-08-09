// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "builtinregistry.h"
#include "base/jstexgenmanager.h"
#include "base/textureproject.h"
#include "boxblur.h"
#include "cutout.h"
#include "displacementmap.h"
#include "gaussianblur.h"
#include "gradient.h"
#include "greyscale.h"
#include "invert.h"
#include "lens.h"
#include "merge.h"
#include "mirror.h"
#include "modifylevels.h"
#include "normalmap.h"
#include "pointillism.h"
#include "setchannels.h"
#include "sinetransform.h"
#include "stackblur.h"
#include "star.h"
#include "text.h"
#include <stdexcept>

void registerBuiltInGenerators(TextureProject& project) {
   project.addGenerator(TextureGeneratorPtr(new BoxBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new CutoutTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new DisplacementMapTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GaussianBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GradientTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GreyscaleTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new InvertTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new LensTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new MergeTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new MirrorTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new ModifyLevelsTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new NormalMapTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new PointillismTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SetChannelsTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SineTransformTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new StarTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new StackBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new TextTextureGenerator()));
   const QStringList javaScriptErrors = registerBundledJavaScriptGenerators(project);
   if (!javaScriptErrors.isEmpty()) {
      throw std::runtime_error(QStringLiteral("Bundled JavaScript generator failure:\n%1")
                                   .arg(javaScriptErrors.join(QLatin1Char('\n')))
                                   .toStdString());
   }
}
