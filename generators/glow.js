const generator = {
  apiVersion: 1,
  name: "Glow",
  description: "Adds a coloured glow around the visible parts of an image.",
  type: "filter",
  inputs: ["Image"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Glow colour",
      description: "Colour used for the generated glow.",
      default: { r: 255, g: 255, b: 0, a: 255 },
    },
    {
      id: "size",
      type: "real",
      name: "Glow size (%)",
      description: "Controls how far the glow expands beyond the source shape.",
      default: 4,
      min: 0,
      max: 20,
    },
    {
      id: "firstblurlevel",
      type: "real",
      name: "Outer blur radius (%)",
      description: "Softens the outline relative to the shorter texture dimension.",
      default: 3,
      min: 0,
      max: 25,
    },
    {
      id: "mode",
      type: "choice",
      name: "Expansion mode",
      description: "Selects whether the outline is expanded by copies or by scaling.",
      values: ["Multiply", "Enlarge"],
      default: "Multiply",
    },
    {
      id: "ontop",
      type: "boolean",
      name: "Glow on top",
      description: "Allows the glow to extend over the opaque area of the source image.",
      default: false,
    },
    {
      id: "cutoutx",
      type: "real",
      name: "Cut-out width (%)",
      description: "Width of the inner area removed when the glow is drawn on top.",
      default: 95,
      min: 0,
      max: 100,
      group: "cutout",
      enabler: "ontop",
    },
    {
      id: "cutouty",
      type: "real",
      name: "Cut-out height (%)",
      description: "Height of the inner area removed when the glow is drawn on top.",
      default: 95,
      min: 0,
      max: 100,
      group: "cutout",
      enabler: "ontop",
    },
    {
      id: "secondblurlevel",
      type: "real",
      name: "Inner blur radius (%)",
      description: "Softens the cut-out relative to the shorter texture dimension.",
      default: 3,
      min: 0,
      max: 25,
      enabler: "ontop",
    },
    {
      id: "includesource",
      type: "boolean",
      name: "Include source image",
      description: "Composites the original input image with the generated glow.",
      default: true,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: begin with a transparent output. A filter has nothing to draw when
    // its Image input is disconnected, so the transparent image is returned.
    const outputPixels = output.data;
    outputPixels.fill(0);

    const sourceImage = inputs.Image;
    if (!sourceImage) return;

    const sourcePixels = sourceImage.data;
    const sourceStride = sourceImage.stride;
    const outputStride = output.stride;
    const width = size.width;
    const height = size.height;
    const pixelCount = width * height;

    // The glow is built from an alpha mask. Each entry represents how visible the
    // corresponding source pixel is: 0 is transparent and 255 is fully opaque.
    let glowAlpha = new Uint8Array(pixelCount);

    // Step 2: expand the source shape. Multiply mode places eight shifted copies
    // around it. Four copies move horizontally or vertically, while four move
    // diagonally. The diagonal distance is shortened to keep the outline rounded.
    if (settings.mode === "Multiply") {
      const horizontalDistance = Math.trunc(settings.size * width / 100);
      const verticalDistance = Math.trunc(settings.size * height / 100);
      const diagonalHorizontalDistance = Math.trunc(settings.size * 0.705 * width / 100);
      const diagonalVerticalDistance = Math.trunc(settings.size * 0.705 * height / 100);
      const horizontalOffsets = [
        -horizontalDistance,
        horizontalDistance,
        0,
        0,
        -diagonalHorizontalDistance,
        diagonalHorizontalDistance,
        -diagonalHorizontalDistance,
        diagonalHorizontalDistance,
      ];
      const verticalOffsets = [
        0,
        0,
        -verticalDistance,
        verticalDistance,
        -diagonalVerticalDistance,
        -diagonalVerticalDistance,
        diagonalVerticalDistance,
        diagonalVerticalDistance,
      ];

      for (let y = 0; y < height; ++y) {
        for (let x = 0; x < width; ++x) {
          let combinedAlpha = 0;

          for (let copy = 0; copy < 8; ++copy) {
            // Work backwards from the output pixel to the shifted copy's source.
            const sourceX = x - horizontalOffsets[copy];
            const sourceY = y - verticalOffsets[copy];
            if (sourceX < 0 || sourceX >= width || sourceY < 0 || sourceY >= height) {
              continue;
            }

            const sourceOffset = sourceY * sourceStride + sourceX * 4;
            combinedAlpha += sourcePixels[sourceOffset + 3];
          }

          // The native Merge generator adds the copies and limits the result to
          // the largest value that fits in one byte.
          glowAlpha[y * width + x] = Math.min(255, combinedAlpha);
        }
      }
    } else {
      // Enlarge mode makes one scaled copy instead. Glow size describes how far
      // each edge expands, so both edges add twice that percentage to the scale.
      const scale = 1 + settings.size * 2 / 100;
      const centreX = width / 2;
      const centreY = height / 2;

      for (let y = 0; y < height; ++y) {
        const sourceY = Math.floor((y + 0.5 - centreY) / scale + centreY);
        if (sourceY < 0 || sourceY >= height) continue;

        for (let x = 0; x < width; ++x) {
          const sourceX = Math.floor((x + 0.5 - centreX) / scale + centreX);
          if (sourceX < 0 || sourceX >= width) continue;

          const sourceOffset = sourceY * sourceStride + sourceX * 4;
          glowAlpha[y * width + x] = sourcePixels[sourceOffset + 3];
        }
      }
    }

    // Step 3: strengthen the expanded mask. Faint source pixels must still create
    // a visible glow, so their alpha is multiplied by five and limited to 255.
    for (let pixel = 0; pixel < pixelCount; ++pixel) {
      glowAlpha[pixel] = Math.min(255, glowAlpha[pixel] * 5);
    }

    // Step 4: soften the outside of the mask. The radius follows the shorter
    // dimension continuously, so changing the texture size cannot make it jump.
    const blurReference = Math.min(width, height);
    const outerRadius = Math.min(
      254,
      Math.max(0, Math.round(settings.firstblurlevel * blurReference / 100)),
    );

    if (outerRadius > 0) {
      const horizontalBlur = new Uint8Array(pixelCount);
      const blurredAlpha = new Uint8Array(pixelCount);
      const blurDivisor = (outerRadius + 1) * (outerRadius + 1);

      // First blur every row. Pixels nearest the centre have the greatest weight,
      // and progressively more distant pixels have progressively smaller weights.
      for (let y = 0; y < height; ++y) {
        const rowStart = y * width;
        let weightedTotal = 0;
        let leavingTotal = 0;
        let enteringTotal = 0;

        // Prepare the weighted total for the first pixel in this row. Coordinates
        // outside the texture use the nearest edge pixel, just like Stack Blur.
        for (let distance = -outerRadius; distance <= outerRadius; ++distance) {
          const sampleX = Math.max(0, Math.min(width - 1, distance));
          const alpha = glowAlpha[rowStart + sampleX];
          weightedTotal += (outerRadius + 1 - Math.abs(distance)) * alpha;
          if (distance <= 0) leavingTotal += alpha;
        }
        for (let distance = 1; distance <= outerRadius + 1; ++distance) {
          const sampleX = Math.min(width - 1, distance);
          enteringTotal += glowAlpha[rowStart + sampleX];
        }

        // Moving one pixel changes every weight by exactly one. Updating these
        // totals avoids recalculating the complete blur window at every position.
        for (let x = 0; x < width; ++x) {
          horizontalBlur[rowStart + x] = Math.floor(weightedTotal / blurDivisor);
          weightedTotal += enteringTotal - leavingTotal;

          const leavingX = Math.max(0, x - outerRadius);
          const centreX = Math.min(width - 1, x + 1);
          const enteringX = Math.min(width - 1, x + outerRadius + 2);
          leavingTotal += glowAlpha[rowStart + centreX] - glowAlpha[rowStart + leavingX];
          enteringTotal += glowAlpha[rowStart + enteringX] - glowAlpha[rowStart + centreX];
        }
      }

      // Then repeat the same process down every column. Combining horizontal and
      // vertical passes spreads the glow in every direction around the source.
      for (let x = 0; x < width; ++x) {
        let weightedTotal = 0;
        let leavingTotal = 0;
        let enteringTotal = 0;

        for (let distance = -outerRadius; distance <= outerRadius; ++distance) {
          const sampleY = Math.max(0, Math.min(height - 1, distance));
          const alpha = horizontalBlur[sampleY * width + x];
          weightedTotal += (outerRadius + 1 - Math.abs(distance)) * alpha;
          if (distance <= 0) leavingTotal += alpha;
        }
        for (let distance = 1; distance <= outerRadius + 1; ++distance) {
          const sampleY = Math.min(height - 1, distance);
          enteringTotal += horizontalBlur[sampleY * width + x];
        }

        for (let y = 0; y < height; ++y) {
          blurredAlpha[y * width + x] = Math.floor(weightedTotal / blurDivisor);
          weightedTotal += enteringTotal - leavingTotal;

          const leavingY = Math.max(0, y - outerRadius);
          const centreY = Math.min(height - 1, y + 1);
          const enteringY = Math.min(height - 1, y + outerRadius + 2);
          leavingTotal += horizontalBlur[centreY * width + x]
            - horizontalBlur[leavingY * width + x];
          enteringTotal += horizontalBlur[enteringY * width + x]
            - horizontalBlur[centreY * width + x];
        }
      }

      glowAlpha = blurredAlpha;
    }

    // Step 5: remove the centre of the glow. Normally the original source is the
    // cut-out. "Glow on top" uses a smaller copy, leaving some glow over its edge.
    if (settings.ontop) {
      const horizontalScale = settings.cutoutx / 100;
      const verticalScale = settings.cutouty / 100;
      const centreX = width / 2;
      const centreY = height / 2;

      if (horizontalScale > 0 && verticalScale > 0) {
        for (let y = 0; y < height; ++y) {
          const sourceY = Math.floor((y + 0.5 - centreY) / verticalScale + centreY);
          if (sourceY < 0 || sourceY >= height) continue;

          for (let x = 0; x < width; ++x) {
            const sourceX = Math.floor((x + 0.5 - centreX) / horizontalScale + centreX);
            if (sourceX < 0 || sourceX >= width) continue;

            const sourceOffset = sourceY * sourceStride + sourceX * 4;
            const sourceAlpha = sourcePixels[sourceOffset + 3] / 255;
            const alphaOffset = y * width + x;
            glowAlpha[alphaOffset] = Math.round(
              glowAlpha[alphaOffset] * (1 - sourceAlpha),
            );
          }
        }
      }
    } else if (!settings.includesource) {
      // A glow-only result needs the original source shape removed from its mask.
      // When the source is included, placing it over the glow in Step 8 performs
      // this attenuation naturally and avoids applying it twice.
      for (let y = 0; y < height; ++y) {
        let sourceOffset = y * sourceStride + 3;
        let alphaOffset = y * width;

        for (let x = 0; x < width; ++x) {
          const sourceAlpha = sourcePixels[sourceOffset] / 255;
          glowAlpha[alphaOffset] = Math.round(glowAlpha[alphaOffset] * (1 - sourceAlpha));
          sourceOffset += 4;
          ++alphaOffset;
        }
      }
    }

    // Step 6: when the glow is allowed on top, soften the edge of its smaller
    // cut-out. This is the same triangular blur used for the outside of the glow.
    const innerRadius = Math.min(
      254,
      Math.max(0, Math.round(settings.secondblurlevel * blurReference / 100)),
    );

    if (settings.ontop && innerRadius > 0) {
      const horizontalBlur = new Uint8Array(pixelCount);
      const blurredAlpha = new Uint8Array(pixelCount);
      const blurDivisor = (innerRadius + 1) * (innerRadius + 1);

      for (let y = 0; y < height; ++y) {
        const rowStart = y * width;
        let weightedTotal = 0;
        let leavingTotal = 0;
        let enteringTotal = 0;

        for (let distance = -innerRadius; distance <= innerRadius; ++distance) {
          const sampleX = Math.max(0, Math.min(width - 1, distance));
          const alpha = glowAlpha[rowStart + sampleX];
          weightedTotal += (innerRadius + 1 - Math.abs(distance)) * alpha;
          if (distance <= 0) leavingTotal += alpha;
        }
        for (let distance = 1; distance <= innerRadius + 1; ++distance) {
          const sampleX = Math.min(width - 1, distance);
          enteringTotal += glowAlpha[rowStart + sampleX];
        }

        for (let x = 0; x < width; ++x) {
          horizontalBlur[rowStart + x] = Math.floor(weightedTotal / blurDivisor);
          weightedTotal += enteringTotal - leavingTotal;

          const leavingX = Math.max(0, x - innerRadius);
          const centreX = Math.min(width - 1, x + 1);
          const enteringX = Math.min(width - 1, x + innerRadius + 2);
          leavingTotal += glowAlpha[rowStart + centreX] - glowAlpha[rowStart + leavingX];
          enteringTotal += glowAlpha[rowStart + enteringX] - glowAlpha[rowStart + centreX];
        }
      }

      for (let x = 0; x < width; ++x) {
        let weightedTotal = 0;
        let leavingTotal = 0;
        let enteringTotal = 0;

        for (let distance = -innerRadius; distance <= innerRadius; ++distance) {
          const sampleY = Math.max(0, Math.min(height - 1, distance));
          const alpha = horizontalBlur[sampleY * width + x];
          weightedTotal += (innerRadius + 1 - Math.abs(distance)) * alpha;
          if (distance <= 0) leavingTotal += alpha;
        }
        for (let distance = 1; distance <= innerRadius + 1; ++distance) {
          const sampleY = Math.min(height - 1, distance);
          enteringTotal += horizontalBlur[sampleY * width + x];
        }

        for (let y = 0; y < height; ++y) {
          blurredAlpha[y * width + x] = Math.floor(weightedTotal / blurDivisor);
          weightedTotal += enteringTotal - leavingTotal;

          const leavingY = Math.max(0, y - innerRadius);
          const centreY = Math.min(height - 1, y + 1);
          const enteringY = Math.min(height - 1, y + innerRadius + 2);
          leavingTotal += horizontalBlur[centreY * width + x]
            - horizontalBlur[leavingY * width + x];
          enteringTotal += horizontalBlur[enteringY * width + x]
            - horizontalBlur[centreY * width + x];
        }
      }

      glowAlpha = blurredAlpha;
    }

    // Step 7: apply both the selected glow colour and its alpha. Mask alpha controls
    // the shape, while colour alpha controls the maximum opacity of that shape.
    const glowColour = settings.color;
    const glowColourOpacity = glowColour.a / 255;

    if (!settings.includesource) {
      for (let y = 0; y < height; ++y) {
        let outputOffset = y * outputStride;
        let alphaOffset = y * width;

        for (let x = 0; x < width; ++x) {
          outputPixels[outputOffset] = glowColour.r;
          outputPixels[outputOffset + 1] = glowColour.g;
          outputPixels[outputOffset + 2] = glowColour.b;
          outputPixels[outputOffset + 3] = Math.round(
            glowAlpha[alphaOffset] * glowColourOpacity,
          );
          outputOffset += 4;
          ++alphaOffset;
        }
      }
      return;
    }

    // Step 8: combine the glow and original image. Normally the source is placed
    // over the glow. "Glow on top" deliberately reverses that layer order.
    for (let y = 0; y < height; ++y) {
      let sourceOffset = y * sourceStride;
      let outputOffset = y * outputStride;
      let alphaOffset = y * width;

      for (let x = 0; x < width; ++x) {
        const sourceAlpha = sourcePixels[sourceOffset + 3] / 255;
        const addedAlpha = glowAlpha[alphaOffset] / 255 * glowColourOpacity;
        const visibleSource = settings.ontop
          ? sourceAlpha * (1 - addedAlpha)
          : sourceAlpha;
        const visibleGlow = settings.ontop
          ? addedAlpha
          : addedAlpha * (1 - sourceAlpha);
        const resultAlpha = visibleSource + visibleGlow;

        if (resultAlpha > 0) {
          outputPixels[outputOffset] = Math.floor(
            (sourcePixels[sourceOffset] * visibleSource + glowColour.r * visibleGlow)
              / resultAlpha,
          );
          outputPixels[outputOffset + 1] = Math.floor(
            (sourcePixels[sourceOffset + 1] * visibleSource + glowColour.g * visibleGlow)
              / resultAlpha,
          );
          outputPixels[outputOffset + 2] = Math.floor(
            (sourcePixels[sourceOffset + 2] * visibleSource + glowColour.b * visibleGlow)
              / resultAlpha,
          );
          outputPixels[outputOffset + 3] = Math.floor(resultAlpha * 255);
        }

        sourceOffset += 4;
        outputOffset += 4;
        ++alphaOffset;
      }
    }
  },
};
