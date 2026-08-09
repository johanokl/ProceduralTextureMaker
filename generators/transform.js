const generator = {
  apiVersion: 1,
  name: "Transform",
  description: "Scales, rotates, offsets, and tiles the input image.",
  type: "filter",
  inputs: ["Image"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "xscale",
      type: "real",
      name: "Horizontal scale (%)",
      description: "Scales the input image horizontally around the texture centre.",
      default: 100,
      min: 0,
      max: 500,
      group: "scale",
    },
    {
      id: "yscale",
      type: "real",
      name: "Vertical scale (%)",
      description: "Scales the input image vertically around the texture centre.",
      default: 100,
      min: 0,
      max: 500,
      group: "scale",
    },
    {
      id: "rotation",
      type: "real",
      name: "Rotation (°)",
      description: "Rotates the input image around the texture centre.",
      default: 0,
      min: -360,
      max: 360,
    },
    {
      id: "offsetleft",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the input image horizontally across the texture.",
      default: 0,
      min: -200,
      max: 200,
    },
    {
      id: "offsettop",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the input image vertically across the texture.",
      default: 0,
      min: -200,
      max: 200,
    },
    {
      id: "backgroundcolor",
      type: "color",
      name: "Background colour",
      description: "Colour used where the transformed image leaves empty space.",
      default: { r: 0, g: 0, b: 0, a: 0 },
    },
    {
      id: "firstXtiles",
      type: "integer",
      name: "First-pass horizontal tiles",
      description: "Number of source-sized tiles across the area transformed as one image.",
      default: 1,
      min: 1,
      max: 5,
      group: "first tiles",
    },
    {
      id: "firstYtiles",
      type: "integer",
      name: "First-pass vertical tiles",
      description: "Number of source-sized tiles down the area transformed as one image.",
      default: 1,
      min: 1,
      max: 5,
      group: "first tiles",
    },
    {
      id: "secondXtiles",
      type: "integer",
      name: "Second-pass horizontal tiles",
      description: "Number of source copies fitted horizontally within each tile.",
      default: 1,
      min: 1,
      max: 50,
      group: "second tiles",
    },
    {
      id: "secondYtiles",
      type: "integer",
      name: "Second-pass vertical tiles",
      description: "Number of source copies fitted vertically within each tile.",
      default: 1,
      min: 1,
      max: 50,
      group: "second tiles",
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: fill the output with the selected background colour. Parts of the
    // transformed image that do not cover the output will leave this colour visible.
    const outputPixels = output.data;
    const width = size.width;
    const height = size.height;
    const backgroundColour = settings.backgroundcolor;

    // Write one RGBA pixel first. outputPixels.fill() cannot be used because it
    // repeats one number rather than a four-byte colour.
    outputPixels[0] = backgroundColour.r;
    outputPixels[1] = backgroundColour.g;
    outputPixels[2] = backgroundColour.b;
    outputPixels[3] = backgroundColour.a;

    // Repeatedly copy the completed bytes over the empty part. The filled area
    // doubles each time, which is much faster than writing every pixel in JavaScript.
    for (let filled = 4; filled < outputPixels.length; filled *= 2) {
      const copyLength = Math.min(filled, outputPixels.length - filled);
      outputPixels.set(outputPixels.subarray(0, copyLength), filled);
    }

    // Step 2: read the source image and convert percentages into scale factors.
    // A scale of 100% becomes 1, 50% becomes 0.5, and 200% becomes 2.
    const sourceImage = inputs.Image;
    const horizontalScale = settings.xscale / 100;
    const verticalScale = settings.yscale / 100;

    // There is nothing to transform without an image or with an invisible scale.
    if (!sourceImage || horizontalScale <= 0 || verticalScale <= 0) return;

    const sourcePixels = sourceImage.data;
    // A stride is the number of bytes from the start of one row to the next.
    const sourceStride = sourceImage.stride;
    const outputStride = output.stride;

    // Step 3: describe the virtual tiled source that will be transformed.
    // The first-pass settings enlarge this area to contain several source-sized tiles.
    const tiledSourceWidth = settings.firstXtiles * width;
    const tiledSourceHeight = settings.firstYtiles * height;

    // The second-pass settings repeat the source within every source-sized tile.
    // For example, a value of 2 fits two complete copies across one tile.
    const horizontalRepeats = settings.secondXtiles;
    const verticalRepeats = settings.secondYtiles;

    // The destination centre stays fixed while the image rotates. Keeping its
    // half-pixel part is important for odd-sized textures and tiled source areas.
    const transformedCenterX = width / 2 + settings.offsetleft * width / 100;
    const transformedCenterY = height / 2 + settings.offsettop * height / 100;

    // JavaScript's trigonometric functions use radians rather than degrees.
    const radians = settings.rotation * Math.PI / 180;
    const cosine = Math.cos(radians);
    const sine = Math.sin(radians);

    // Step 4: calculate the output rectangle that can contain the transformed image.
    // Rotating a rectangle makes its horizontal and vertical reach depend on both
    // original dimensions. These extents give its furthest distance from the centre.
    const halfScaledWidth = tiledSourceWidth * horizontalScale / 2;
    const halfScaledHeight = tiledSourceHeight * verticalScale / 2;
    const extentX = Math.abs(cosine) * halfScaledWidth + Math.abs(sine) * halfScaledHeight;
    const extentY = Math.abs(sine) * halfScaledWidth + Math.abs(cosine) * halfScaledHeight;
    const left = Math.max(0, Math.floor(transformedCenterX - extentX));
    const right = Math.min(width - 1, Math.ceil(transformedCenterX + extentX));
    const top = Math.max(0, Math.floor(transformedCenterY - extentY));
    const bottom = Math.min(height - 1, Math.ceil(transformedCenterY + extentY));

    // Step 5: visit every output pixel in that rectangle. Instead of pushing source
    // pixels forwards, work backwards: undo the transformation to discover which
    // source pixel belongs at each output position. This avoids gaps in the result.
    const virtualXChangePerPixel = cosine / horizontalScale;
    const virtualYChangePerPixel = -sine / verticalScale;

    for (let y = top; y <= bottom; ++y) {
      const xFromCenter = left + 0.5 - transformedCenterX;
      const yFromCenter = y + 0.5 - transformedCenterY;

      // Undo rotation and scaling, then move the origin from the centre to the
      // top-left corner of the virtual tiled source.
      let virtualSourceX = (cosine * xFromCenter + sine * yFromCenter)
          / horizontalScale
        + tiledSourceWidth / 2;
      let virtualSourceY = (-sine * xFromCenter + cosine * yFromCenter)
          / verticalScale
        + tiledSourceHeight / 2;
      let outputOffset = y * outputStride + left * 4;

      for (let x = left; x <= right; ++x) {
        const insideTiledSource = virtualSourceX >= 0
          && virtualSourceX < tiledSourceWidth
          && virtualSourceY >= 0
          && virtualSourceY < tiledSourceHeight;

        if (insideTiledSource) {
          // Multiplication creates the second-pass repetitions. The remainder
          // operator (%) then wraps the coordinate into the original source image.
          const repeatedSourceX = Math.floor(virtualSourceX * horizontalRepeats);
          const repeatedSourceY = Math.floor(virtualSourceY * verticalRepeats);
          const sourceX = repeatedSourceX % width;
          const sourceY = repeatedSourceY % height;
          const sourceOffset = sourceY * sourceStride + sourceX * 4;
          const sourceAlpha = sourcePixels[sourceOffset + 3] / 255;

          // Step 6: copy opaque pixels directly. They completely hide the background.
          if (sourceAlpha >= 1) {
            outputPixels[outputOffset] = sourcePixels[sourceOffset];
            outputPixels[outputOffset + 1] = sourcePixels[sourceOffset + 1];
            outputPixels[outputOffset + 2] = sourcePixels[sourceOffset + 2];
            outputPixels[outputOffset + 3] = sourcePixels[sourceOffset + 3];
          } else if (sourceAlpha > 0) {
            // A transparent background has no existing colour to preserve.
            const backgroundAlpha = outputPixels[outputOffset + 3] / 255;
            if (backgroundAlpha === 0) {
              outputPixels[outputOffset] = sourcePixels[sourceOffset];
              outputPixels[outputOffset + 1] = sourcePixels[sourceOffset + 1];
              outputPixels[outputOffset + 2] = sourcePixels[sourceOffset + 2];
              outputPixels[outputOffset + 3] = sourcePixels[sourceOffset + 3];
            } else {
              // For two translucent colours, retain the visible contribution from
              // both. The result is stored as ordinary, non-premultiplied RGBA.
              const remainingBackground = 1 - sourceAlpha;
              const resultAlpha = sourceAlpha + backgroundAlpha * remainingBackground;
              outputPixels[outputOffset] = Math.round(
                (sourcePixels[sourceOffset] * sourceAlpha
                  + outputPixels[outputOffset] * backgroundAlpha * remainingBackground)
                  / resultAlpha,
              );
              outputPixels[outputOffset + 1] = Math.round(
                (sourcePixels[sourceOffset + 1] * sourceAlpha
                  + outputPixels[outputOffset + 1] * backgroundAlpha * remainingBackground)
                  / resultAlpha,
              );
              outputPixels[outputOffset + 2] = Math.round(
                (sourcePixels[sourceOffset + 2] * sourceAlpha
                  + outputPixels[outputOffset + 2] * backgroundAlpha * remainingBackground)
                  / resultAlpha,
              );
              outputPixels[outputOffset + 3] = Math.round(resultAlpha * 255);
            }
          }
        }

        // Move to the next output pixel. Four bytes advance to its RGBA values.
        virtualSourceX += virtualXChangePerPixel;
        virtualSourceY += virtualYChangePerPixel;
        outputOffset += 4;
      }
    }
  },
};
