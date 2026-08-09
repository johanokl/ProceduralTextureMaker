const generator = {
  apiVersion: 1,
  name: "Shadow",
  description: "Adds a coloured, blurred, and offset shadow behind an image.",
  type: "filter",
  inputs: ["Foreground"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Shadow colour",
      description: "Colour and maximum opacity of the generated shadow.",
      default: { r: 100, g: 100, b: 100, a: 255 },
    },
    {
      id: "xscale",
      type: "real",
      name: "Horizontal scale (%)",
      description: "Scales the shadow horizontally around the texture centre.",
      default: 100,
      min: 0,
      max: 500,
    },
    {
      id: "yscale",
      type: "real",
      name: "Vertical scale (%)",
      description: "Scales the shadow vertically around the texture centre.",
      default: 100,
      min: 0,
      max: 500,
    },
    {
      id: "offsetleft",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the shadow horizontally relative to the texture width.",
      default: -10,
      min: -100,
      max: 100,
    },
    {
      id: "offsettop",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the shadow vertically relative to the texture height.",
      default: 10,
      min: -100,
      max: 100,
    },
    {
      id: "level",
      type: "real",
      name: "Blur radius (%)",
      description: "Softens the shadow edge relative to the shorter texture dimension.",
      default: 3,
      min: 0,
      max: 20,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: begin with a transparent image. A shadow needs a foreground shape,
    // so there is nothing else to do when that input is disconnected.
    const outputPixels = output.data;
    outputPixels.fill(0);

    const foregroundImage = inputs.Foreground;
    if (!foregroundImage) return;

    const foregroundPixels = foregroundImage.data;
    const foregroundStride = foregroundImage.stride;
    const outputStride = output.stride;
    const width = size.width;
    const height = size.height;
    const pixelCount = width * height;

    // Step 2: convert the blur percentage into pixels before making the mask. The
    // mask receives this much padding on every side, allowing an off-screen part
    // of the transformed shadow to blur back into the visible texture.
    const radius = Math.max(
      0,
      Math.round(settings.level * Math.min(width, height) / 100),
    );
    const paddedWidth = width + radius * 2;
    const paddedHeight = height + radius * 2;
    const paddedPixelCount = paddedWidth * paddedHeight;

    // Make the scaled and offset shadow shape from the foreground's alpha channel.
    // Working backwards from each padded destination pixel avoids enlargement holes.
    const scaledAlpha = new Uint8Array(paddedPixelCount);
    const horizontalScale = settings.xscale / 100;
    const verticalScale = settings.yscale / 100;

    if (horizontalScale > 0 && verticalScale > 0) {
      const shadowCentreX = width / 2 + settings.offsetleft * width / 100;
      const shadowCentreY = height / 2 + settings.offsettop * height / 100;

      for (let paddedY = 0; paddedY < paddedHeight; ++paddedY) {
        const y = paddedY - radius;
        // Adding 0.5 uses the centre of a pixel instead of its top-left corner.
        const sourceY = Math.floor(
          (y + 0.5 - shadowCentreY) / verticalScale + height / 2,
        );
        if (sourceY < 0 || sourceY >= height) continue;

        for (let paddedX = 0; paddedX < paddedWidth; ++paddedX) {
          const x = paddedX - radius;
          const sourceX = Math.floor(
            (x + 0.5 - shadowCentreX) / horizontalScale + width / 2,
          );
          if (sourceX < 0 || sourceX >= width) continue;

          scaledAlpha[paddedY * paddedWidth + paddedX] = foregroundPixels[
            sourceY * foregroundStride + sourceX * 4 + 3
          ];
        }
      }
    }

    // Step 3: soften the padded shadow with a box blur. Its radius is a percentage
    // of the shorter texture dimension, keeping the effect proportional at every size.
    let paddedShadowAlpha = scaledAlpha;

    if (radius > 0) {
      const horizontalBlur = new Uint8Array(paddedPixelCount);
      const blurredAlpha = new Uint8Array(paddedPixelCount);
      const sampleCount = radius * 2 + 1;

      // First average a horizontal window across every row. The running total is
      // updated by removing the sample that leaves and adding the one that enters.
      for (let y = 0; y < paddedHeight; ++y) {
        const rowStart = y * paddedWidth;
        let total = 0;

        for (let sampleX = -radius; sampleX <= radius; ++sampleX) {
          if (sampleX >= 0 && sampleX < paddedWidth) {
            total += scaledAlpha[rowStart + sampleX];
          }
        }

        for (let x = 0; x < paddedWidth; ++x) {
          horizontalBlur[rowStart + x] = Math.round(total / sampleCount);

          const leavingX = x - radius;
          const enteringX = x + radius + 1;
          if (leavingX >= 0) total -= scaledAlpha[rowStart + leavingX];
          if (enteringX < paddedWidth) total += scaledAlpha[rowStart + enteringX];
        }
      }

      // Then average a vertical window down every column. The two simple passes
      // together spread the alpha in every direction around the original shape.
      for (let x = 0; x < paddedWidth; ++x) {
        let total = 0;

        for (let sampleY = -radius; sampleY <= radius; ++sampleY) {
          if (sampleY >= 0 && sampleY < paddedHeight) {
            total += horizontalBlur[sampleY * paddedWidth + x];
          }
        }

        for (let y = 0; y < paddedHeight; ++y) {
          blurredAlpha[y * paddedWidth + x] = Math.round(total / sampleCount);

          const leavingY = y - radius;
          const enteringY = y + radius + 1;
          if (leavingY >= 0) total -= horizontalBlur[leavingY * paddedWidth + x];
          if (enteringY < paddedHeight) {
            total += horizontalBlur[enteringY * paddedWidth + x];
          }
        }
      }

      paddedShadowAlpha = blurredAlpha;
    }

    // Crop the padded mask back to the requested texture. Padding has already done
    // its job by preserving blur contributions that originated beyond an edge.
    const shadowAlpha = new Uint8Array(pixelCount);
    for (let y = 0; y < height; ++y) {
      const paddedRow = (y + radius) * paddedWidth + radius;
      shadowAlpha.set(
        paddedShadowAlpha.subarray(paddedRow, paddedRow + width),
        y * width,
      );
    }

    // Step 4: place the original foreground over the coloured shadow. Straight-alpha
    // composition keeps the visible contribution of both translucent layers.
    const shadowColour = settings.color;
    const shadowColourOpacity = shadowColour.a / 255;

    for (let y = 0; y < height; ++y) {
      let foregroundOffset = y * foregroundStride;
      let outputOffset = y * outputStride;
      let alphaOffset = y * width;

      for (let x = 0; x < width; ++x) {
        const foregroundOpacity = foregroundPixels[foregroundOffset + 3] / 255;
        const shadowOpacity = shadowAlpha[alphaOffset] / 255 * shadowColourOpacity;
        const visibleShadow = shadowOpacity * (1 - foregroundOpacity);
        const resultOpacity = foregroundOpacity + visibleShadow;

        if (resultOpacity > 0) {
          outputPixels[outputOffset] = Math.round(
            (foregroundPixels[foregroundOffset] * foregroundOpacity
              + shadowColour.r * visibleShadow) / resultOpacity,
          );
          outputPixels[outputOffset + 1] = Math.round(
            (foregroundPixels[foregroundOffset + 1] * foregroundOpacity
              + shadowColour.g * visibleShadow) / resultOpacity,
          );
          outputPixels[outputOffset + 2] = Math.round(
            (foregroundPixels[foregroundOffset + 2] * foregroundOpacity
              + shadowColour.b * visibleShadow) / resultOpacity,
          );
          outputPixels[outputOffset + 3] = Math.round(resultOpacity * 255);
        }

        foregroundOffset += 4;
        outputOffset += 4;
        ++alphaOffset;
      }
    }
  },
};
