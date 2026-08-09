const generator = {
  apiVersion: 1,
  name: "Sine plasma",
  description: "Generates a repeating two-dimensional sine-wave plasma.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour mixed into the generated plasma pattern.",
      default: { r: 255, g: 0, b: 0, a: 255 },
    },
    {
      id: "xoffset",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Shifts the plasma horizontally across the texture.",
      default: 10,
      min: -100,
      max: 100,
    },
    {
      id: "yoffset",
      type: "real",
      name: "Vertical offset (%)",
      description: "Shifts the plasma vertically across the texture.",
      default: 10,
      min: -100,
      max: 100,
    },
    {
      id: "xfrequency",
      type: "real",
      name: "Horizontal frequency",
      description: "Controls how often the pattern repeats along the horizontal axis.",
      default: 10,
      min: 0,
      max: 100,
      group: "frequencies",
    },
    {
      id: "yfrequency",
      type: "real",
      name: "Vertical frequency",
      description: "Controls how often the pattern repeats along the vertical axis.",
      default: 10,
      min: 0,
      max: 100,
      group: "frequencies",
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Begin with the optional background. Without one, begin with transparency.
    const pixels = output.data;
    const background = inputs.Background?.data;
    if (background) pixels.set(background);
    else pixels.fill(0);

    // Convert the controls into pixel offsets and sine-wave frequencies.
    const width = size.width;
    const height = size.height;
    const stride = output.stride;
    // Keep the real-valued offsets fractional. Sine accepts decimal positions, so
    // there is no reason to discard the fine control offered by the settings panel.
    const xOffset = settings.xoffset * width / 100;
    const yOffset = settings.yoffset * height / 100;
    const xFrequency = settings.xfrequency * 5 / width;
    const yFrequency = settings.yfrequency * 5 / height;
    const color = settings.color;

    // A plasma value is one horizontal sine plus one vertical sine. Caching the
    // horizontal part reduces two sine calls per pixel to one per row or column.
    const horizontal = new Float64Array(width);
    for (let x = 0; x < width; ++x) {
      horizontal[x] = 0.25 * Math.sin((x - xOffset) * xFrequency);
    }

    // Step 3: use the plasma value as the selected colour's opacity, then composite
    // it over the optional background. A colour with alpha zero now has no effect.
    for (let y = 0; y < height; ++y) {
      const vertical = 0.5 + 0.25 * Math.sin((y - yOffset) * yFrequency);
      let pixel = y * stride;
      for (let x = 0; x < width; ++x, pixel += 4) {
        const value = vertical + horizontal[x];
        const plasmaAlpha = value * color.a / 255;
        if (plasmaAlpha <= 0) continue;

        const backgroundAlpha = pixels[pixel + 3] / 255;
        if (plasmaAlpha >= 1 || backgroundAlpha === 0) {
          pixels[pixel] = color.r;
          pixels[pixel + 1] = color.g;
          pixels[pixel + 2] = color.b;
          pixels[pixel + 3] = Math.round(plasmaAlpha * 255);
          continue;
        }

        const remainingBackground = 1 - plasmaAlpha;
        const resultAlpha = plasmaAlpha + backgroundAlpha * remainingBackground;
        pixels[pixel] = Math.round(
          (color.r * plasmaAlpha
            + pixels[pixel] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixel + 1] = Math.round(
          (color.g * plasmaAlpha
            + pixels[pixel + 1] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixel + 2] = Math.round(
          (color.b * plasmaAlpha
            + pixels[pixel + 2] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixel + 3] = Math.round(resultAlpha * 255);
      }
    }
  },
};
