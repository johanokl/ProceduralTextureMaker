const generator = {
  apiVersion: 1,
  name: "Fire",
  description: "Generates a classic demoscene fire with an optional sine-wave bend.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "falloff",
      type: "real",
      name: "Cooling",
      description: "Controls how quickly the flames cool and disappear as they rise.",
      default: 3,
      min: 0,
      max: 8,
    },
    {
      id: "iterations",
      type: "integer",
      name: "Iterations",
      description: "Number of simulation steps used to develop the flame pattern.",
      default: 180,
      min: 1,
      max: 400,
    },
    {
      id: "randomize",
      type: "integer",
      name: "Random seed",
      description: "Selects the repeatable random pattern feeding the flames.",
      default: 5,
      min: 1,
      max: 100_000,
    },
    {
      id: "embercolor",
      type: "color",
      name: "Ember colour",
      description: "Colour used for the cooler outer parts of the flames.",
      default: { r: 150, g: 5, b: 0, a: 255 },
    },
    {
      id: "flamecolor",
      type: "color",
      name: "Flame colour",
      description: "Colour used for the main body of the flames.",
      default: { r: 255, g: 85, b: 0, a: 255 },
    },
    {
      id: "hotcolor",
      type: "color",
      name: "Hot colour",
      description: "Colour used for the hottest parts near the fire source.",
      default: { r: 255, g: 245, b: 190, a: 255 },
    },
    {
      id: "sinewave",
      type: "boolean",
      name: "Sine-wave bend",
      description: "Bends the flames from side to side while keeping their base anchored.",
      default: false,
    },
    {
      id: "waveamplitude",
      type: "real",
      name: "Wave amplitude (%)",
      description: "Maximum horizontal bend as a percentage of the texture width.",
      default: 6,
      min: 0,
      max: 30,
      enabler: "sinewave",
    },
    {
      id: "wavefrequency",
      type: "real",
      name: "Wave frequency",
      description: "Number of side-to-side waves over the height of the texture.",
      default: 2,
      min: 0,
      max: 10,
      enabler: "sinewave",
    },
    {
      id: "wavephase",
      type: "real",
      name: "Wave phase (°)",
      description: "Moves the sine-wave bend through its repeating cycle.",
      default: 0,
      min: 0,
      max: 360,
      enabler: "sinewave",
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional background into the output. The fire will be
    // composited over it. Without a background, begin with transparency.
    const outputPixels = output.data;
    const background = inputs.Background?.data;
    if (background) outputPixels.set(background);
    else outputPixels.fill(0);

    const width = size.width;
    const height = size.height;
    const outputStride = output.stride;

    // Step 2: create a smaller heat buffer. Simulating at no more than 160 pixels
    // per axis keeps large textures responsive; the result is smoothly enlarged later.
    const simulationWidth = Math.min(width, 160);
    const simulationHeight = Math.min(height, 160);
    const simulationSize = simulationWidth * simulationHeight;
    let heat = new Float32Array(simulationSize);
    let nextHeat = new Float32Array(simulationSize);

    // This is a small deterministic random-number generator. Using the same seed
    // produces the same fire every time, which is important for procedural textures.
    let randomState = settings.randomize >>> 0;
    if (randomState === 0) randomState = 1;

    // Scale cooling with the simulation height so short and tall textures retain
    // roughly the same flame proportions instead of producing different effects.
    const cooling = settings.falloff * 100 / simulationHeight;
    const iterations = settings.iterations;
    const bottomRow = simulationHeight - 1;
    const rowAboveBottom = Math.max(0, simulationHeight - 2);

    // Step 3: develop the fire over a number of simulation steps. Fresh heat is
    // added at the bottom, then neighbouring values are averaged into the row above.
    for (let iteration = 0; iteration < iterations; ++iteration) {
      for (let y = 0; y < simulationHeight - 1; ++y) {
        const firstRowBelow = y + 1;
        const secondRowBelow = Math.min(simulationHeight - 1, y + 2);

        for (let x = 0; x < simulationWidth; ++x) {
          // Horizontal wrapping lets flames leaving one side re-enter at the other,
          // avoiding a visible hard border in the simulation.
          const leftX = x === 0 ? simulationWidth - 1 : x - 1;
          const rightX = x + 1 === simulationWidth ? 0 : x + 1;
          const heatFromBelow = (
            heat[firstRowBelow * simulationWidth + leftX]
              + heat[firstRowBelow * simulationWidth + x]
              + heat[firstRowBelow * simulationWidth + rightX]
              + heat[secondRowBelow * simulationWidth + x]
          ) / 4;

          // Cooling removes a little energy as the heat rises. Larger values make
          // short flames; smaller values allow tall flames to reach the upper edge.
          nextHeat[y * simulationWidth + x] = Math.max(0, heatFromBelow - cooling);
        }
      }

      // Feed the bottom of the fire with bright random heat. Occasional cooler
      // pockets break the source into separate, naturally flickering flame tongues.
      for (let x = 0; x < simulationWidth; ++x) {
        randomState = (Math.imul(randomState, 1_664_525) + 1_013_904_223) >>> 0;
        const brightness = randomState / 4_294_967_296;
        randomState = (Math.imul(randomState, 1_664_525) + 1_013_904_223) >>> 0;
        const coolerPocket = randomState / 4_294_967_296 < 0.12;
        const sourceHeat = coolerPocket
          ? 45 + brightness * 80
          : 190 + brightness * 65;

        nextHeat[bottomRow * simulationWidth + x] = sourceHeat;
        nextHeat[rowAboveBottom * simulationWidth + x] = Math.max(
          nextHeat[rowAboveBottom * simulationWidth + x],
          sourceHeat * 0.72,
        );
      }

      // The new buffer becomes the input for the next step. Reusing two arrays is
      // quicker and creates less temporary memory than allocating one each time.
      const previousHeat = heat;
      heat = nextHeat;
      nextHeat = previousHeat;
    }

    // Step 4: smoothly enlarge the heat buffer to the requested texture size.
    // The optional sine wave shifts each sampled row. Its strength fades to zero
    // at the bottom, so the source of the flames remains fixed in place.
    const wavePhase = settings.wavephase * Math.PI / 180;
    const waveTurns = settings.wavefrequency * 2 * Math.PI;
    const waveAmplitude = settings.waveamplitude * simulationWidth / 100;

    for (let y = 0; y < height; ++y) {
      const verticalPosition = height > 1 ? y / (height - 1) : 0;
      const simulationY = verticalPosition * (simulationHeight - 1);
      const firstSimulationY = Math.floor(simulationY);
      const secondSimulationY = Math.min(simulationHeight - 1, firstSimulationY + 1);
      const verticalFraction = simulationY - firstSimulationY;

      let waveShift = 0;
      if (settings.sinewave) {
        const distanceFromBase = 1 - verticalPosition;
        waveShift = Math.sin(verticalPosition * waveTurns + wavePhase)
          * waveAmplitude * distanceFromBase;
      }

      let outputOffset = y * outputStride;
      for (let x = 0; x < width; ++x) {
        const horizontalPosition = width > 1 ? x / (width - 1) : 0;
        let simulationX = horizontalPosition * (simulationWidth - 1) - waveShift;

        // Wrap horizontally after applying the wave so no empty strip appears at
        // either side of the texture.
        simulationX %= simulationWidth;
        if (simulationX < 0) simulationX += simulationWidth;

        const firstSimulationX = Math.floor(simulationX);
        const secondSimulationX = (firstSimulationX + 1) % simulationWidth;
        const horizontalFraction = simulationX - firstSimulationX;

        // Bilinear interpolation mixes the four nearest simulation values. This
        // removes the blocky pixels that a direct enlargement would produce.
        const topLeftHeat = heat[
          firstSimulationY * simulationWidth + firstSimulationX
        ];
        const topRightHeat = heat[
          firstSimulationY * simulationWidth + secondSimulationX
        ];
        const bottomLeftHeat = heat[
          secondSimulationY * simulationWidth + firstSimulationX
        ];
        const bottomRightHeat = heat[
          secondSimulationY * simulationWidth + secondSimulationX
        ];
        const topHeat = topLeftHeat
          + (topRightHeat - topLeftHeat) * horizontalFraction;
        const bottomHeat = bottomLeftHeat
          + (bottomRightHeat - bottomLeftHeat) * horizontalFraction;
        const sampledHeat = topHeat + (bottomHeat - topHeat) * verticalFraction;
        const heatLevel = Math.max(0, Math.min(1, sampledHeat / 255));

        if (heatLevel > 0) {
          // Step 5: turn heat into colour. Raising the heat to a fractional power
          // reveals more detail in cooler regions without washing out the hot core.
          const palettePosition = Math.pow(heatLevel, 0.75);
          let red;
          let green;
          let blue;
          let paletteAlpha;

          if (palettePosition < 0.35) {
            // The coolest region fades from black into the ember colour.
            const amount = palettePosition / 0.35;
            red = settings.embercolor.r * amount;
            green = settings.embercolor.g * amount;
            blue = settings.embercolor.b * amount;
            paletteAlpha = settings.embercolor.a;
          } else if (palettePosition < 0.7) {
            // Medium heat moves from dark embers into the main flame colour.
            const amount = (palettePosition - 0.35) / 0.35;
            red = settings.embercolor.r
              + (settings.flamecolor.r - settings.embercolor.r) * amount;
            green = settings.embercolor.g
              + (settings.flamecolor.g - settings.embercolor.g) * amount;
            blue = settings.embercolor.b
              + (settings.flamecolor.b - settings.embercolor.b) * amount;
            paletteAlpha = settings.embercolor.a
              + (settings.flamecolor.a - settings.embercolor.a) * amount;
          } else {
            // The hottest region brightens from the flame colour to the hot colour.
            const amount = (palettePosition - 0.7) / 0.3;
            red = settings.flamecolor.r
              + (settings.hotcolor.r - settings.flamecolor.r) * amount;
            green = settings.flamecolor.g
              + (settings.hotcolor.g - settings.flamecolor.g) * amount;
            blue = settings.flamecolor.b
              + (settings.hotcolor.b - settings.flamecolor.b) * amount;
            paletteAlpha = settings.flamecolor.a
              + (settings.hotcolor.a - settings.flamecolor.a) * amount;
          }

          // Low heat becomes transparent, giving the flames soft edges when they
          // are placed over the optional background image.
          const fireAlpha = Math.min(1, heatLevel * 1.35) * paletteAlpha / 255;
          const backgroundAlpha = outputPixels[outputOffset + 3] / 255;

          if (fireAlpha >= 1 || backgroundAlpha === 0) {
            outputPixels[outputOffset] = Math.round(red);
            outputPixels[outputOffset + 1] = Math.round(green);
            outputPixels[outputOffset + 2] = Math.round(blue);
            outputPixels[outputOffset + 3] = Math.round(fireAlpha * 255);
          } else {
            // Source-over composition preserves the visible contribution from the
            // fire and the background in ordinary, non-premultiplied RGBA.
            const remainingBackground = 1 - fireAlpha;
            const resultAlpha = fireAlpha + backgroundAlpha * remainingBackground;
            outputPixels[outputOffset] = Math.round(
              (red * fireAlpha
                + outputPixels[outputOffset] * backgroundAlpha * remainingBackground)
                / resultAlpha,
            );
            outputPixels[outputOffset + 1] = Math.round(
              (green * fireAlpha
                + outputPixels[outputOffset + 1] * backgroundAlpha * remainingBackground)
                / resultAlpha,
            );
            outputPixels[outputOffset + 2] = Math.round(
              (blue * fireAlpha
                + outputPixels[outputOffset + 2] * backgroundAlpha * remainingBackground)
                / resultAlpha,
            );
            outputPixels[outputOffset + 3] = Math.round(resultAlpha * 255);
          }
        }

        outputOffset += 4;
      }
    }
  },
};
