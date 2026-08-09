const generator = {
  apiVersion: 1,
  name: "Noise",
  description: "Generates coloured random noise over an optional background image.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Noise colour",
      description: "Colour used for the generated noise.",
      default: { r: 0, g: 0, b: 0, a: 255 },
    },
    {
      id: "alphamin",
      type: "integer",
      name: "Minimum alpha",
      description: "Lowest opacity that can be assigned to a noise sample.",
      default: 0,
      min: 0,
      max: 255,
    },
    {
      id: "alphamax",
      type: "integer",
      name: "Maximum alpha",
      description: "Highest opacity that can be assigned to a noise sample.",
      default: 255,
      min: 0,
      max: 255,
    },
    {
      id: "width",
      type: "integer",
      name: "Noise width (%)",
      description: "Number of horizontal noise samples relative to the texture width.",
      default: 100,
      min: 1,
      max: 100,
      group: "size",
    },
    {
      id: "height",
      type: "integer",
      name: "Noise height (%)",
      description: "Number of vertical noise samples relative to the texture height.",
      default: 100,
      min: 1,
      max: 100,
      group: "size",
    },
    {
      id: "scatter",
      type: "boolean",
      name: "Scatter points",
      description: "Leaves some samples transparent instead of filling the complete texture.",
      default: true,
    },
    {
      id: "density",
      type: "real",
      name: "Point density (%)",
      description: "Percentage of noise samples filled when scattering is enabled.",
      default: 10,
      min: 0,
      max: 100,
      enabler: "scatter",
    },
    {
      id: "randomizer",
      type: "integer",
      name: "Random seed",
      description: "Selects the repeatable random noise pattern.",
      default: 500,
      min: 0,
      max: 1000,
    },
    {
      id: "smoothscale",
      type: "boolean",
      name: "Smooth scaling",
      description: "Blends neighbouring samples when the noise grid is enlarged.",
      default: false,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional background. Without one, begin with transparent
    // pixels. The noise will be painted over this image near the end of the function.
    const outputPixels = output.data;
    const backgroundPixels = inputs.Background?.data;
    if (backgroundPixels) outputPixels.set(backgroundPixels);
    else outputPixels.fill(0);

    const width = size.width;
    const height = size.height;
    const outputStride = output.stride;

    // Step 2: make a small grid of noise samples. Both dimensions are percentages
    // of the requested texture, so the pattern has the same detail at every size.
    const noiseWidth = Math.max(1, Math.round(width * settings.width / 100));
    const noiseHeight = Math.max(1, Math.round(height * settings.height / 100));
    const noiseAlpha = new Uint8Array(noiseWidth * noiseHeight);

    // Treat the two alpha controls as a range even if the user selects them in the
    // opposite order. This is friendlier than producing an invalid random range.
    const minimumAlpha = Math.min(settings.alphamin, settings.alphamax);
    const maximumAlpha = Math.max(settings.alphamin, settings.alphamax);
    const alphaRange = maximumAlpha - minimumAlpha + 1;
    const density = settings.density / 100;

    // Step 3: fill the grid using a small repeatable random-number calculation.
    // Keeping its state in this script makes Noise independent of every generator.
    let randomState = settings.randomizer >>> 0;
    for (let sample = 0; sample < noiseAlpha.length; ++sample) {
      // This linear congruential calculation advances to the next random value.
      // Dividing the unsigned 32-bit result gives a decimal from 0 up to, but not
      // including, 1. The same seed always creates the same sequence.
      randomState = (Math.imul(1_664_525, randomState) + 1_013_904_223) >>> 0;
      const placement = randomState / 4_294_967_296;

      if (settings.scatter && placement >= density) continue;

      randomState = (Math.imul(1_664_525, randomState) + 1_013_904_223) >>> 0;
      const opacity = randomState / 4_294_967_296;
      noiseAlpha[sample] = minimumAlpha + Math.floor(opacity * alphaRange);
    }

    // Step 4: visit each output pixel and find the corresponding sample. Nearest
    // scaling picks one grid value. Smooth scaling blends the four values around
    // the exact sample position instead, removing block-shaped transitions.
    const colour = settings.color;
    const colourAlpha = colour.a / 255;

    for (let y = 0; y < height; ++y) {
      let outputOffset = y * outputStride;

      for (let x = 0; x < width; ++x) {
        let sampledAlpha;

        if (!settings.smoothscale || (noiseWidth === width && noiseHeight === height)) {
          const sampleX = Math.min(noiseWidth - 1, Math.floor(x * noiseWidth / width));
          const sampleY = Math.min(noiseHeight - 1, Math.floor(y * noiseHeight / height));
          sampledAlpha = noiseAlpha[sampleY * noiseWidth + sampleX];
        } else {
          // Pixel centres are mapped to the noise grid. Coordinates outside its
          // first or last centre use the closest edge sample.
          const exactX = (x + 0.5) * noiseWidth / width - 0.5;
          const exactY = (y + 0.5) * noiseHeight / height - 0.5;
          const left = Math.max(0, Math.floor(exactX));
          const right = Math.min(noiseWidth - 1, left + 1);
          const top = Math.max(0, Math.floor(exactY));
          const bottom = Math.min(noiseHeight - 1, top + 1);
          const horizontalPart = Math.max(0, exactX - left);
          const verticalPart = Math.max(0, exactY - top);

          const topAlpha = noiseAlpha[top * noiseWidth + left]
            + (noiseAlpha[top * noiseWidth + right] - noiseAlpha[top * noiseWidth + left])
              * horizontalPart;
          const bottomAlpha = noiseAlpha[bottom * noiseWidth + left]
            + (noiseAlpha[bottom * noiseWidth + right]
              - noiseAlpha[bottom * noiseWidth + left]) * horizontalPart;
          sampledAlpha = topAlpha + (bottomAlpha - topAlpha) * verticalPart;
        }

        // Step 5: paint this noise sample over the background. Both the random
        // opacity and the alpha of the chosen colour affect the result.
        const noiseOpacity = sampledAlpha / 255 * colourAlpha;
        if (noiseOpacity > 0) {
          const backgroundOpacity = outputPixels[outputOffset + 3] / 255;
          const remainingBackground = 1 - noiseOpacity;
          const resultOpacity = noiseOpacity + backgroundOpacity * remainingBackground;

          outputPixels[outputOffset] = Math.round(
            (colour.r * noiseOpacity
              + outputPixels[outputOffset] * backgroundOpacity * remainingBackground)
              / resultOpacity,
          );
          outputPixels[outputOffset + 1] = Math.round(
            (colour.g * noiseOpacity
              + outputPixels[outputOffset + 1] * backgroundOpacity * remainingBackground)
              / resultOpacity,
          );
          outputPixels[outputOffset + 2] = Math.round(
            (colour.b * noiseOpacity
              + outputPixels[outputOffset + 2] * backgroundOpacity * remainingBackground)
              / resultOpacity,
          );
          outputPixels[outputOffset + 3] = Math.round(resultOpacity * 255);
        }

        outputOffset += 4;
      }
    }
  },
};
