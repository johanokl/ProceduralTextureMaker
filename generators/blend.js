/*
 * The metadata describes the generator and the controls shown in the application.
 * Each setting's "id" is also its name in the settings object used by generate().
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes. The byte
 * position of pixel (x, y) is y * output.stride + x * 4.
 *
 * Think of the inputs as sheets of paper. Foreground is placed over Background.
 * Swap the input connections to reverse their order. Blend modes change how their
 * colours mix; foreground opacity controls how much of the top image is visible.
 */
const generator = {
  apiVersion: 1,
  name: "Blend",
  description: "Places Foreground over Background using the selected blend mode and opacity.",
  type: "combiner",
  inputs: ["Background", "Foreground"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "mode",
      type: "choice",
      name: "Blend mode",
      description: "Normal layers images; Add and Screen brighten; Multiply darkens; "
        + "Overlay adds contrast; Difference compares colours.",
      values: ["Normal", "Add", "Multiply", "Screen", "Overlay", "Difference"],
      default: "Normal",
    },
    {
      id: "alpha",
      type: "real",
      name: "Foreground opacity (%)",
      description: "Controls the opacity of Foreground before it is combined with Background.",
      default: 100,
      min: 0,
      max: 100,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: read the fixed layers and convert foreground opacity to a fraction.
    // For example, 25% becomes 0.25. Clamp hand-edited values to the valid range.
    const outputPixels = output.data;
    const lowerImage = inputs.Background;
    const upperImage = inputs.Foreground;
    const upperOpacity = Math.max(0, Math.min(1, settings.alpha / 100));

    // Step 2: a missing or invisible Foreground leaves Background unchanged.
    // With neither image, return transparent black.
    if (!upperImage || upperOpacity === 0) {
      if (lowerImage) outputPixels.set(lowerImage.data);
      else outputPixels.fill(0);
      return;
    }

    // Without Background, keep Foreground's colours and apply its opacity to
    // alpha. Blend modes need two visible layers, so they have no effect here.
    if (!lowerImage) {
      outputPixels.set(upperImage.data);
      for (let offset = 3; offset < outputPixels.length; offset += 4) {
        outputPixels[offset] = Math.trunc(upperImage.data[offset] * upperOpacity);
      }
      return;
    }

    // Step 3: read the pixel buffers. Background is always the lower layer and
    // Foreground is always the upper layer in the calculations below.
    const lowerPixels = lowerImage.data;
    const upperPixels = upperImage.data;
    const lowerStride = lowerImage.stride;
    const upperStride = upperImage.stride;
    const outputStride = output.stride;
    const width = size.width;
    const height = size.height;
    const blendMode = settings.mode;

    // Step 4: handle the usual case quickly and plainly. In Normal mode at 100%,
    // the upper image can be copied in one operation. Most opaque pixels are then
    // already finished; only transparent pixels need the lower image.
    if (blendMode === "Normal" && upperOpacity === 1) {
      outputPixels.set(upperPixels);

      // Visit the alpha byte of every pixel. An alpha value of 255 means fully
      // opaque, so the copied upper pixel needs no further work.
      for (let y = 0; y < height; ++y) {
        let lowerOffset = y * lowerStride;
        let upperOffset = y * upperStride;
        let outputOffset = y * outputStride;

        for (let x = 0; x < width; ++x) {
          const upperAlphaByte = upperPixels[upperOffset + 3];
          if (upperAlphaByte < 255) {
            const lowerAlphaByte = lowerPixels[lowerOffset + 3];

            // An alpha value of 0 means fully transparent. It reveals the lower
            // pixel unchanged. If both pixels are transparent, write transparent
            // black so invisible colour values do not remain in the output.
            if (upperAlphaByte === 0) {
              if (lowerAlphaByte === 0) {
                outputPixels[outputOffset] = 0;
                outputPixels[outputOffset + 1] = 0;
                outputPixels[outputOffset + 2] = 0;
                outputPixels[outputOffset + 3] = 0;
              } else {
                outputPixels[outputOffset] = lowerPixels[lowerOffset];
                outputPixels[outputOffset + 1] = lowerPixels[lowerOffset + 1];
                outputPixels[outputOffset + 2] = lowerPixels[lowerOffset + 2];
                outputPixels[outputOffset + 3] = lowerAlphaByte;
              }
            } else if (lowerAlphaByte > 0) {
              // Both pixels are visible but neither completely hides the other.
              // Standard source-over composition determines their combined alpha
              // and how much of each colour comes from the upper pixel.
              const lowerAlpha = lowerAlphaByte / 255;
              const upperAlpha = upperAlphaByte / 255;
              const resultAlpha = upperAlpha + lowerAlpha - upperAlpha * lowerAlpha;
              const upperShare = upperAlpha / resultAlpha;

              for (let channel = 0; channel < 3; ++channel) {
                outputPixels[outputOffset + channel] = Math.trunc(
                  (1 - upperShare) * lowerPixels[lowerOffset + channel]
                    + upperShare * upperPixels[upperOffset + channel],
                );
              }
              outputPixels[outputOffset + 3] = Math.trunc(resultAlpha * 255);
            }
          }

          lowerOffset += 4;
          upperOffset += 4;
          outputOffset += 4;
        }
      }
      return;
    }

    // Step 5: all other combinations need the selected blend formula. Process one
    // pixel at a time. Each pixel contains red, green, blue, and alpha bytes.
    for (let y = 0; y < height; ++y) {
      let lowerOffset = y * lowerStride;
      let upperOffset = y * upperStride;
      let outputOffset = y * outputStride;

      for (let x = 0; x < width; ++x) {
        // Alpha is converted from its byte range of 0 to 255 into the decimal range
        // 0 to 1. The opacity setting changes only the image placed on top.
        const lowerAlpha = lowerPixels[lowerOffset + 3] / 255;
        const upperAlpha = upperOpacity * upperPixels[upperOffset + 3] / 255;

        // Source-over alpha combines the visible parts of the two pixels. Their
        // overlap is subtracted because it would otherwise be counted twice.
        const resultAlpha = upperAlpha + lowerAlpha - upperAlpha * lowerAlpha;

        // A visible result needs colour calculations. A fully transparent result
        // is handled by the final else branch and written as transparent black.
        if (resultAlpha > 0) {
          // Apply the blend mode to red, green, and blue separately. Blend modes
          // change colours only; alpha is handled after these three channels.
          for (let channel = 0; channel < 3; ++channel) {
            const lowerByte = lowerPixels[lowerOffset + channel];
            const upperByte = upperPixels[upperOffset + channel];
            let blendedByte = upperByte;

            if (blendMode === "Add") {
              // Add the colours and stop at the maximum byte value, 255.
              blendedByte = Math.min(255, lowerByte + upperByte);
            } else if (blendMode === "Multiply") {
              // A white foreground leaves Background unchanged; black darkens it.
              blendedByte = lowerByte * upperByte / 255;
            } else if (blendMode === "Screen") {
              // Multiply the inverted colours, then invert the result to lighten.
              blendedByte = 255 - (255 - lowerByte) * (255 - upperByte) / 255;
            } else if (blendMode === "Overlay") {
              // Dark background values use Multiply; light values use Screen.
              // This strengthens contrast while retaining Background's structure.
              blendedByte = lowerByte < 128
                ? 2 * lowerByte * upperByte / 255
                : 255 - 2 * (255 - lowerByte) * (255 - upperByte) / 255;
            } else if (blendMode === "Difference") {
              // Equal colours produce black; larger differences produce light.
              blendedByte = Math.abs(lowerByte - upperByte);
            }
            // Normal uses the foreground byte unchanged. Other modes may produce
            // fractions; keep a byte value before applying transparency below.
            blendedByte = Math.trunc(blendedByte);

            // The blend formula assumes the lower pixel is visible. Reduce its
            // effect when that pixel is transparent, then mix the lower and upper
            // contributions according to their combined alpha.
            const visibleUpperByte = Math.floor(
              (1 - lowerAlpha) * upperByte + lowerAlpha * blendedByte + 0.5,
            );
            const upperShare = upperAlpha / resultAlpha;
            const resultByte = Math.trunc(
              (1 - upperShare) * lowerByte + upperShare * visibleUpperByte,
            );
            outputPixels[outputOffset + channel] = Math.max(
              0,
              Math.min(255, resultByte),
            );
          }

          outputPixels[outputOffset + 3] = Math.trunc(resultAlpha * 255);
        } else {
          outputPixels[outputOffset] = 0;
          outputPixels[outputOffset + 1] = 0;
          outputPixels[outputOffset + 2] = 0;
          outputPixels[outputOffset + 3] = 0;
        }

        lowerOffset += 4;
        upperOffset += 4;
        outputOffset += 4;
      }
    }
  },
};
