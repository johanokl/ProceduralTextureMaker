const generator = {
  apiVersion: 1,
  name: "Perlin noise",
  description: "Generates smooth multi-octave gradient noise over an optional background image.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Noise colour",
      description: "Colour and maximum opacity of the generated noise.",
      default: { r: 255, g: 255, b: 255, a: 255 },
    },
    {
      id: "numoctaves",
      type: "integer",
      name: "Octaves",
      description: "Number of progressively finer noise layers combined into the result.",
      default: 5,
      min: 1,
      max: 12,
    },
    {
      id: "persistence",
      type: "real",
      name: "Persistence",
      description: "Strength of each finer octave relative to the previous one.",
      default: 0.5,
      min: 0,
      max: 1,
    },
    {
      id: "lacunarity",
      type: "real",
      name: "Lacunarity",
      description: "Frequency multiplier between successive octaves.",
      default: 2,
      min: 1,
      max: 4,
    },
    {
      id: "zoom",
      type: "real",
      name: "Feature size (%)",
      description: "Approximate size of the largest features relative to the shorter texture dimension.",
      default: 20,
      min: 2,
      max: 200,
    },
    {
      id: "randomizer",
      type: "integer",
      name: "Random seed",
      description: "Selects a repeatable arrangement of gradient directions.",
      default: 500,
      min: 0,
      max: 1_000_000,
    },
    {
      id: "offsetx",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the noise horizontally relative to the texture width.",
      default: 0,
      min: -100,
      max: 100,
      group: "offsets",
    },
    {
      id: "offsety",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the noise vertically relative to the texture height.",
      default: 0,
      min: -100,
      max: 100,
      group: "offsets",
    },
    {
      id: "contrast",
      type: "real",
      name: "Contrast (%)",
      description: "Expands or compresses differences around the middle noise value.",
      default: 100,
      min: 0,
      max: 300,
    },
    {
      id: "brightness",
      type: "real",
      name: "Brightness (%)",
      description: "Moves the complete noise range towards black or white.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "minimum",
      type: "real",
      name: "Minimum strength (%)",
      description: "Lowest amount of noise colour applied to the background.",
      default: 0,
      min: 0,
      max: 100,
    },
    {
      id: "maximum",
      type: "real",
      name: "Maximum strength (%)",
      description: "Highest amount of noise colour applied to the background.",
      default: 100,
      min: 0,
      max: 100,
    },
    {
      id: "invert",
      type: "boolean",
      name: "Invert",
      description: "Reverses the dark and light parts of the noise.",
      default: false,
    },
    {
      id: "seamless",
      type: "boolean",
      name: "Seamless edges",
      description: "Makes the generated noise match at opposite texture edges.",
      default: false,
    },
  ],

  // This function runs whenever the application renders the node.
   
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional background. The generated colour will be painted
    // over it. Without a background, begin with a transparent texture.
    const outputPixels = output.data;
    const backgroundPixels = inputs.Background?.data;
    if (backgroundPixels) outputPixels.set(backgroundPixels);
    else outputPixels.fill(0);

    const width = size.width;
    const height = size.height;
    const outputStride = output.stride;
    const referenceSize = Math.min(width, height);
    const offsetX = settings.offsetx * width / 100;
    const offsetY = settings.offsety * height / 100;
    const seed = Math.trunc(settings.randomizer);

    // Feature size describes the wavelength of the first octave. Frequencies above
    // half a cycle per pixel would alias, so tiny textures stop at that useful limit.
    const featureSize = Math.max(0.01, settings.zoom);
    const requestedFrequency = 100 / featureSize;
    const maximumFrequency = Math.max(0.5, referenceSize / 2);
    const firstFrequency = Math.min(requestedFrequency, maximumFrequency);
    const octaveCount = Math.max(1, Math.min(12, Math.trunc(settings.numoctaves)));
    const persistence = Math.max(0, Math.min(1, settings.persistence));
    const lacunarity = Math.max(1, Math.min(4, settings.lacunarity));

    // The strength range remains valid even if a project file supplies the two
    // controls in the opposite order.
    const minimumStrength = Math.min(settings.minimum, settings.maximum) / 100;
    const maximumStrength = Math.max(settings.minimum, settings.maximum) / 100;
    const strengthRange = maximumStrength - minimumStrength;
    const colour = settings.color;
    const colourOpacity = colour.a / 255;

    // Step 2: visit every output pixel. Coordinates are divided by the shorter
    // dimension, so noise features stay round instead of stretching on wide or tall textures.
    for (let y = 0; y < height; ++y) {
      const verticalBlend = height > 1 ? y / (height - 1) : 0;
      let outputOffset = y * outputStride;

      for (let x = 0; x < width; ++x) {
        const horizontalBlend = width > 1 ? x / (width - 1) : 0;
        let frequency = firstFrequency;
        let amplitude = 1;
        let amplitudeTotal = 0;
        let noiseTotal = 0;

        // Step 3: combine broad and fine octaves. Persistence reduces each new
        // layer's strength, while lacunarity increases its frequency.
        for (let octave = 0; octave < octaveCount; ++octave) {
          if (octave > 0 && (frequency > maximumFrequency || amplitude === 0)) break;

          const sampleX = (x + offsetX) * frequency / referenceSize;
          const sampleY = (y + offsetY) * frequency / referenceSize;
          const horizontalSpan = (width - 1) * frequency / referenceSize;
          const verticalSpan = (height - 1) * frequency / referenceSize;
          const horizontalSamples = settings.seamless && width > 1 ? 2 : 1;
          const verticalSamples = settings.seamless && height > 1 ? 2 : 1;
          let octaveNoise = 0;

          // Seamless mode blends four copies of the same noise field. At the right
          // and bottom edges the shifted copies reach the exact samples used at the
          // left and top, so opposite edges match without changing the hash grid.
          for (let verticalSample = 0; verticalSample < verticalSamples; ++verticalSample) {
            const positionY = sampleY - verticalSample * verticalSpan;
            let verticalWeight = 1;
            if (verticalSamples > 1) {
              verticalWeight = verticalSample === 0 ? 1 - verticalBlend : verticalBlend;
            }

            for (
              let horizontalSample = 0;
              horizontalSample < horizontalSamples;
              ++horizontalSample
            ) {
              const positionX = sampleX - horizontalSample * horizontalSpan;
              let horizontalWeight = 1;
              if (horizontalSamples > 1) {
                horizontalWeight = horizontalSample === 0
                  ? 1 - horizontalBlend
                  : horizontalBlend;
              }

              // Step 4: calculate true two-dimensional gradient noise. Each corner
              // of the surrounding grid cell receives a repeatable direction from
              // its integer coordinates and the seed. The direction is dotted with
              // the distance to this sample point.
              const latticeX = Math.floor(positionX);
              const latticeY = Math.floor(positionY);
              const fractionX = positionX - latticeX;
              const fractionY = positionY - latticeY;
              const smoothX = fractionX * fractionX * fractionX
                * (fractionX * (fractionX * 6 - 15) + 10);
              const smoothY = fractionY * fractionY * fractionY
                * (fractionY * (fractionY * 6 - 15) + 10);
              let topRow = 0;
              let bottomRow = 0;

              for (let cornerY = 0; cornerY <= 1; ++cornerY) {
                let leftDot = 0;
                let rightDot = 0;

                for (let cornerX = 0; cornerX <= 1; ++cornerX) {
                  const gradientX = latticeX + cornerX;
                  const gradientY = latticeY + cornerY;

                  // Integer multiplication and bit mixing form a compact hash.
                  // Converting the hash to an angle gives this corner a unit vector.
                  let hash = Math.imul(gradientX, 374_761_393)
                    + Math.imul(gradientY, 668_265_263)
                    + Math.imul(seed, 1_442_695_041);
                  hash = Math.imul(hash ^ (hash >>> 13), 1_274_126_177);
                  hash ^= hash >>> 16;
                  const angle = (hash >>> 0) / 4_294_967_296 * 2 * Math.PI;
                  const distanceX = fractionX - cornerX;
                  const distanceY = fractionY - cornerY;
                  const dot = Math.cos(angle) * distanceX + Math.sin(angle) * distanceY;

                  if (cornerX === 0) leftDot = dot;
                  else rightDot = dot;
                }

                const row = leftDot + (rightDot - leftDot) * smoothX;
                if (cornerY === 0) topRow = row;
                else bottomRow = row;
              }

              const gradientNoise = topRow + (bottomRow - topRow) * smoothY;
              octaveNoise += gradientNoise * horizontalWeight * verticalWeight;
            }
          }

          noiseTotal += octaveNoise * amplitude;
          amplitudeTotal += amplitude;
          frequency *= lacunarity;
          amplitude *= persistence;
        }

        // Step 5: normalise the octave sum before applying artistic controls. This
        // prevents persistence or octave count from making the result clip brighter.
        let value = 0.5 + noiseTotal / amplitudeTotal * Math.SQRT1_2;
        value = (value - 0.5) * settings.contrast / 100 + 0.5;
        value += settings.brightness / 100;
        value = Math.max(0, Math.min(1, value));
        if (settings.invert) value = 1 - value;

        const strength = minimumStrength + value * strengthRange;
        const noiseOpacity = strength * colourOpacity;

        // Step 6: paint the selected noise colour over the background using normal
        // straight-alpha composition. Transparent colours now leave it unchanged.
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
