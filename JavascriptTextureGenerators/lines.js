const generator = {
  apiVersion: 1,
  name: "LinesJS",
  description: "Draws horizontal lines on an optional canvas.",
  type: "filter",
  inputs: ["Canvas"],

  settings: [
    {
      id: "color",
      type: "color",
      name: "Line colour",
      description: "Colour used to draw the lines.",
      default: { r: 255, g: 100, b: 50, a: 255 },
    },
    {
      id: "height",
      type: "integer",
      name: "Line thickness (%)",
      description: "Thickness of each line as a percentage of the texture height.",
      default: 10,
      min: 0,
      max: 100,
    },
    {
      id: "distance",
      type: "integer",
      name: "Gap size (%)",
      description: "Gap between adjacent lines as a percentage of the texture height.",
      default: 10,
      min: 0,
      max: 100,
    },
    {
      id: "offset",
      type: "integer",
      name: "Pattern offset (%)",
      description: "Shifts the repeating line pattern vertically.",
      default: 0,
      min: 0,
      max: 100,
    }
  ],

  generate(size, settings, output, inputs) {
    const destination = output.data;
    const canvas = inputs.Canvas?.data;
    const lineHeight = Math.round((settings.height * size.height) / 100);
    const distance = Math.round((settings.distance * size.height) / 100);
    const offset = Math.round((settings.offset * size.height) / 100);
    const period = lineHeight + distance;

    if (canvas) {
      destination.set(canvas);
    }

    for (let y = 0; y < size.height; ++y) {
      const drawLine = period > 0 && ((y + offset) % period) > distance;
      for (let x = 0; x < size.width; ++x) {
        const pixel = TexGen.offset(x, y, output.stride);
        if (drawLine) {
          destination[pixel] = settings.color.r;
          destination[pixel + 1] = settings.color.g;
          destination[pixel + 2] = settings.color.b;
          destination[pixel + 3] = settings.color.a;
        } else if (!source) {
          destination[pixel] = 0;
          destination[pixel + 1] = 0;
          destination[pixel + 2] = 0;
          destination[pixel + 3] = 0;
        }
      }
    }
  }
};
