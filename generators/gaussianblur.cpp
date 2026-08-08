
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gaussianblur.h"
#include <QtMath>
#include <cmath>

GaussianBlurTextureGenerator::GaussianBlurTextureGenerator() {
   TextureGeneratorSetting neighbourssetting;
   neighbourssetting.defaultvalue = QVariant((int)1);
   neighbourssetting.name = "Blur radius (px)";
   neighbourssetting.description =
       "Sets the number of neighbouring pixels sampled in each direction.";
   neighbourssetting.min = QVariant(1);
   neighbourssetting.max = QVariant(30);
   neighbourssetting.id = "numneighbours";
   configurables.append(neighbourssetting);

   TextureGeneratorSetting weightsetting;
   weightsetting.defaultvalue = QVariant((double)1);
   weightsetting.name = "Centre weight";
   weightsetting.description =
       "Controls how strongly pixels near the centre of the kernel are favoured.";
   weightsetting.min = QVariant(0);
   weightsetting.max = QVariant(4);
   weightsetting.id = "weight";
   configurables.append(weightsetting);
}

float* GaussianBlurTextureGenerator::ComputeGaussianKernel(const int inRadius,
                                                           const float radiusModifier) const {
   int mem_amount = (inRadius * 2) + 1;
   auto* gaussian_kernel = new float[mem_amount];

   float twoRadiusSquaredRecip = 0.5 / (inRadius * inRadius);
   float sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(M_PI * 2) * inRadius);

   // Create Gaussian Kernel
   int r = -inRadius;
   float sum = 0;
   for (int i = 0; i < mem_amount; i++) {
      float x = r * radiusModifier;
      x = x * x;
      float v = sqrtTwoPiTimesRadiusRecip * std::exp(-x * twoRadiusSquaredRecip);
      gaussian_kernel[i] = v;
      sum += v;
      r++;
   }
   // Normalize distribution
   float div = sum;
   for (int i = 0; i < mem_amount; i++) {
      gaussian_kernel[i] /= div;
   }
   return gaussian_kernel;
}

/// @brief Calculates the Gaussian Blur and stores the result on the height map given
void GaussianBlurTextureGenerator::generate(QSize size, TexturePixel* destimage,
                                            const QMap<QString, TextureImagePtr>& sourceimages,
                                            const TextureNodeSettings& settings) const {
   if (!destimage || !size.isValid()) {
      return;
   }
   int numNeightbours = settings.value("numneighbours").toInt();
   float inWeight = settings.value("weight").toFloat();

   memset(destimage, 0, size.width() * size.height() * sizeof(TexturePixel));

   if (!sourceimages.contains(QStringLiteral("Image"))) {
      return;
   }
   TexturePixel* sourceImage = sourceimages.value(QStringLiteral("Image")).data()->getData();

   int pixels_on_row = 1 + (numNeightbours * 2);

   float* gaussian_kernel = ComputeGaussianKernel(numNeightbours, inWeight);

   for (int y = 0; y < size.height(); y++) {
      int row = y * size.width();
      for (int x = 0; x < size.width(); x++) {
         TexturePixel blurred_value(0, 0, 0, 0);
         for (int xoffset = 0; xoffset < pixels_on_row; xoffset++) {
            int sx = x - numNeightbours + xoffset;
            if (sx < 0) {
               sx = 0;
            } else if (sx > size.width() - 1) {
               sx = size.width() - 1;
            }
            // Calculate newly blurred value
            int pixelPos = row + sx;
            blurred_value.r += gaussian_kernel[xoffset] * sourceImage[pixelPos].r;
            blurred_value.g += gaussian_kernel[xoffset] * sourceImage[pixelPos].g;
            blurred_value.b += gaussian_kernel[xoffset] * sourceImage[pixelPos].b;
            blurred_value.a += gaussian_kernel[xoffset] * sourceImage[pixelPos].a;
         }
         // Set our calculated value to our temp map
         destimage[y * size.width() + x] = blurred_value;
      }
   }
   for (int y = 0; y < size.height(); y++) {
      for (int x = 0; x < size.width(); x++) {
         TexturePixel blurred_value(0, 0, 0, 0);
         for (int yoffset = 0; yoffset < pixels_on_row; yoffset++) {
            int sy = y - numNeightbours + yoffset;
            if (sy < 0) {
               sy = 0;
            } else if (sy > size.height() - 1) {
               sy = size.height() - 1;
            }
            int pixelPos = sy * size.width() + x;
            // Calculate blurred value
            blurred_value.r += gaussian_kernel[yoffset] * destimage[pixelPos].r;
            blurred_value.g += gaussian_kernel[yoffset] * destimage[pixelPos].g;
            blurred_value.b += gaussian_kernel[yoffset] * destimage[pixelPos].b;
            blurred_value.a += gaussian_kernel[yoffset] * destimage[pixelPos].a;
         }

         // Set the original height map value to our computed value
         // This is the actual blurred value (previously scaled)
         destimage[y * size.width() + x] = blurred_value;
      }
   }
   delete[] gaussian_kernel;
}
