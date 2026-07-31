// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "builtinregistry.h"
#include "base/textureproject.h"
#include "blending.h"
#include "boxblur.h"
#include "bricks.h"
#include "checkboard.h"
#include "circle.h"
#include "cutout.h"
#include "displacementmap.h"
#include "fill.h"
#include "fire.h"
#include "gaussianblur.h"
#include "glow.h"
#include "gradient.h"
#include "greyscale.h"
#include "invert.h"
#include "lens.h"
#include "lines.h"
#include "merge.h"
#include "mirror.h"
#include "modifylevels.h"
#include "noise.h"
#include "normalmap.h"
#include "perlinnoise.h"
#include "pixelate.h"
#include "pointillism.h"
#include "setchannels.h"
#include "shadow.h"
#include "sineplasma.h"
#include "sinetransform.h"
#include "square.h"
#include "stackblur.h"
#include "star.h"
#include "text.h"
#include "transform.h"
#include "whirl.h"

void registerBuiltInGenerators(TextureProject& project) {
   project.addGenerator(TextureGeneratorPtr(new BlendingTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new BoxBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new BricksTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new CheckboardTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new CircleTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new CutoutTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new DisplacementMapTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new FillTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new FireTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GaussianBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GlowTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GradientTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new GreyscaleTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new InvertTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new LinesTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new LensTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new MergeTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new MirrorTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new ModifyLevelsTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new NoiseTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new NormalMapTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new PerlinNoiseTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new PixelateTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new PointillismTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SetChannelsTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new ShadowTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SinePlasmaTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SineTransformTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new SquareTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new StarTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new StackBlurTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new TextTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new TransformTextureGenerator()));
   project.addGenerator(TextureGeneratorPtr(new WhirlTextureGenerator()));
}
