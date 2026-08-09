const generator = {
  apiVersion: 1,
  name: "Pixelate",
  description: "Reduces an image to a grid of rectangular colour blocks.",
  type: "filter",
  inputs: ["Image"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "width",
      type: "real",
      name: "Block width (%)",
      description: "Width of each block; 0% leaves the input unchanged.",
      default: 5,
      min: 0,
      max: 50,
      group: "size",
    },
    {
      id: "height",
      type: "real",
      name: "Block height (%)",
      description: "Height of each block; 0% leaves the input unchanged.",
      default: 5,
      min: 0,
      max: 50,
      group: "size",
    },
    {
      id: "offsetx",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the block grid horizontally across the texture.",
      default: 0,
      min: -50,
      max: 50,
    },
    {
      id: "offsety",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the block grid vertically across the texture.",
      default: 0,
      min: -50,
      max: 50,
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
    const textureWidth = size.width;
    const textureHeight = size.height;

    // Step 2: a zero size disables the effect. This makes the zero offered by the
    // settings panel useful and avoids silently turning it into a one-pixel block.
    if (settings.width <= 0 || settings.height <= 0) {
      outputPixels.set(sourcePixels);
      return;
    }

    // Convert positive percentages into pixel measurements. Very small positive
    // values still need a one-pixel block so that both loops always advance.
    const blockWidth = Math.max(1, Math.trunc(settings.width * textureWidth / 100));
    const blockHeight = Math.max(1, Math.trunc(settings.height * textureHeight / 100));
    const horizontalOffset = Math.trunc(settings.offsetx * textureWidth / 100);
    const verticalOffset = Math.trunc(settings.offsety * textureHeight / 100);

    // Step 3: find the block boundary at or immediately before the visible origin.
    // Keeping it congruent with the selected offset makes an offset of zero begin
    // exactly at (0, 0), even when a block does not divide the texture evenly.
    const firstBlockLeft = horizontalOffset
      - Math.ceil(horizontalOffset / blockWidth) * blockWidth;
    const firstBlockTop = verticalOffset
      - Math.ceil(verticalOffset / blockHeight) * blockHeight;

    for (
      let blockTop = firstBlockTop;
      blockTop < textureHeight;
      blockTop += blockHeight
    ) {
      // Skip rows of blocks that end before reaching the visible texture.
      if (blockTop + blockHeight <= 0) continue;

      for (
        let blockLeft = firstBlockLeft;
        blockLeft < textureWidth;
        blockLeft += blockWidth
      ) {
        // Skip blocks that end before reaching the visible texture.
        if (blockLeft + blockWidth <= 0) continue;

        // Step 4: add all colours in this block. Sampling wraps around the source
        // edges, so an offset block can use pixels from both sides of the image.
        let premultipliedRedTotal = 0;
        let premultipliedGreenTotal = 0;
        let premultipliedBlueTotal = 0;
        let alphaTotal = 0;
        let sampledPixels = 0;

        for (let sampleY = blockTop; sampleY < blockTop + blockHeight; ++sampleY) {
          // JavaScript keeps a negative sign after %, so add the texture size when
          // necessary to obtain a valid source row from 0 to textureHeight - 1.
          let sourceY = sampleY % textureHeight;
          if (sourceY < 0) sourceY += textureHeight;

          for (let sampleX = blockLeft; sampleX < blockLeft + blockWidth; ++sampleX) {
            let sourceX = sampleX % textureWidth;
            if (sourceX < 0) sourceX += textureWidth;

            const sourceOffset = sourceY * sourceStride + sourceX * 4;
            const alpha = sourcePixels[sourceOffset + 3];
            premultipliedRedTotal += sourcePixels[sourceOffset] * alpha;
            premultipliedGreenTotal += sourcePixels[sourceOffset + 1] * alpha;
            premultipliedBlueTotal += sourcePixels[sourceOffset + 2] * alpha;
            alphaTotal += alpha;
            ++sampledPixels;
          }
        }

        // RGB is weighted by alpha, so invisible colours cannot tint a translucent
        // block. Dividing by total alpha converts it back to ordinary straight RGBA.
        const averageRed = alphaTotal > 0
          ? Math.round(premultipliedRedTotal / alphaTotal)
          : 0;
        const averageGreen = alphaTotal > 0
          ? Math.round(premultipliedGreenTotal / alphaTotal)
          : 0;
        const averageBlue = alphaTotal > 0
          ? Math.round(premultipliedBlueTotal / alphaTotal)
          : 0;
        const averageAlpha = Math.floor(alphaTotal / sampledPixels);

        // Step 5: clip the block to the visible texture, then give every visible
        // pixel the average colour calculated above.
        const visibleLeft = Math.max(0, blockLeft);
        const visibleRight = Math.min(textureWidth, blockLeft + blockWidth);
        const visibleTop = Math.max(0, blockTop);
        const visibleBottom = Math.min(textureHeight, blockTop + blockHeight);

        for (let y = visibleTop; y < visibleBottom; ++y) {
          let outputOffset = y * outputStride + visibleLeft * 4;

          for (let x = visibleLeft; x < visibleRight; ++x) {
            outputPixels[outputOffset] = averageRed;
            outputPixels[outputOffset + 1] = averageGreen;
            outputPixels[outputOffset + 2] = averageBlue;
            outputPixels[outputOffset + 3] = averageAlpha;
            outputOffset += 4;
          }
        }
      }
    }
  },
};
