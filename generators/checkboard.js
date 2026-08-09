const generator = {
  apiVersion: 1,
  name: "Checkboard",
  description: "Draws a chequerboard pattern over an optional background.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour used for alternating squares.",
      default: { r: 0, g: 0, b: 0, a: 255 },
    },
    {
      id: "brickwidth",
      type: "real",
      name: "Square width (%)",
      description: "Width of each square as a percentage of the texture width.",
      default: 10,
      min: 1,
      max: 100,
    },
    {
      id: "brickheight",
      type: "real",
      name: "Square height (%)",
      description: "Height of each square as a percentage of the texture height.",
      default: 10,
      min: 1,
      max: 100,
    },
    {
      id: "offsetx",
      type: "integer",
      name: "Horizontal offset (%)",
      description: "Moves the pattern horizontally as a percentage of the texture width.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "offsety",
      type: "integer",
      name: "Vertical offset (%)",
      description: "Moves the pattern vertically as a percentage of the texture height.",
      default: 0,
      min: -100,
      max: 100,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional background into the output. The coloured squares
    // will be painted over this image. Without a background, start transparent.
    const pixels = output.data;
    const background = inputs.Background?.data;
    if (background) pixels.set(background);
    else pixels.fill(0);

    // Step 2: convert percentages from the settings panel into pixel measurements.
    // A square width of 10% uses one tenth of the texture width, for example.
    const width = size.width;
    const height = size.height;
    const stride = output.stride;
    const squareWidth = Math.max(1, Math.trunc(settings.brickwidth * width / 100));
    const squareHeight = Math.max(1, Math.trunc(settings.brickheight * height / 100));
    const offsetX = Math.trunc(settings.offsetx * width / 100);
    const offsetY = Math.trunc(settings.offsety * height / 100);
    const colour = settings.color;
    if (colour.a === 0) return;

    // Alpha values are stored as bytes from 0 to 255. Dividing by 255 converts
    // them to fractions from 0 to 1, which are easier to use when blending colours.
    const colourAlpha = colour.a / 255;
    const remainingBackground = 1 - colourAlpha;

    // Step 3: use the offsets as the top-left origin of the repeating pattern.
    // Coordinates before this origin become negative, but Math.floor() still places
    // them in the correct square, so the pattern repeats in every direction.
    const patternOriginX = offsetX;
    const patternOriginY = offsetY;

    // Step 4: examine every output pixel. Dividing its distance from the pattern
    // origin by the square size tells us which row and column of squares it belongs to.
    for (let y = 0; y < height; ++y) {
      const squareRow = Math.floor((y - patternOriginY) / squareHeight);

      for (let x = 0; x < width; ++x) {
        const squareColumn = Math.floor((x - patternOriginX) / squareWidth);

        // Even row-and-column sums are coloured; odd sums are left unchanged.
        // Moving into the next row or column changes the sum and alternates the square.
        const isColouredSquare = (squareRow + squareColumn) % 2 === 0;
        if (!isColouredSquare) continue;

        // Each pixel contains four consecutive bytes: red, green, blue, and alpha.
        const pixelOffset = y * stride + x * 4;

        // Step 5: an opaque colour completely replaces the background pixel.
        if (colour.a === 255) {
          pixels[pixelOffset] = colour.r;
          pixels[pixelOffset + 1] = colour.g;
          pixels[pixelOffset + 2] = colour.b;
          pixels[pixelOffset + 3] = 255;
          continue;
        }

        // A transparent background has no visible colour to preserve, so the
        // translucent square can also be assigned directly.
        const backgroundAlpha = pixels[pixelOffset + 3] / 255;
        if (backgroundAlpha === 0) {
          pixels[pixelOffset] = colour.r;
          pixels[pixelOffset + 1] = colour.g;
          pixels[pixelOffset + 2] = colour.b;
          pixels[pixelOffset + 3] = colour.a;
          continue;
        }

        // For two translucent colours, retain the visible contribution from both.
        // The result is stored as ordinary, non-premultiplied RGBA.
        const resultAlpha = colourAlpha + backgroundAlpha * remainingBackground;
        pixels[pixelOffset] = Math.round(
          (colour.r * colourAlpha
            + pixels[pixelOffset] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixelOffset + 1] = Math.round(
          (colour.g * colourAlpha
            + pixels[pixelOffset + 1] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixelOffset + 2] = Math.round(
          (colour.b * colourAlpha
            + pixels[pixelOffset + 2] * backgroundAlpha * remainingBackground) / resultAlpha,
        );
        pixels[pixelOffset + 3] = Math.round(resultAlpha * 255);
      }
    }
  },
};
