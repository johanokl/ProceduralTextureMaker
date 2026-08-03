const generator = {
  apiVersion: 1,
  name: "LinesJS",
  description: "Draws horizontal lines over an optional source image.",
  type: "filter",
  inputs: ["Input"],

  settings: {
    color: {
      type: "color",
      name: "Line color",
      default: { r: 255, g: 100, b: 50, a: 255 },
      order: 1
    },
    height: {
      type: "integer",
      name: "Line height",
      default: 10,
      min: 0,
      max: 100,
      order: 2
    },
    distance: {
      type: "integer",
      name: "Distance",
      default: 10,
      min: 0,
      max: 100,
      order: 3
    },
    offset: {
      type: "integer",
      name: "Offset",
      default: 0,
      min: 0,
      max: 100,
      order: 4
    }
  },

  generate(size, settings, output, inputs) {
    const destination = output.data;
    const source = inputs.Input?.data;
    const lineHeight = Math.round((settings.height * size.height) / 100);
    const distance = Math.round((settings.distance * size.height) / 100);
    const offset = Math.round((settings.offset * size.height) / 100);
    const period = lineHeight + distance;

    if (source) {
      destination.set(source);
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
