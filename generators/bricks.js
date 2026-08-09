const generator = {
  apiVersion: 1,
  name: "Bricks",
  description: "Draws a staggered brickwork pattern over an optional input image.",
  type: "generator",
  inputs: ["Background"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour used to draw the mortar lines.",
      default: { r: 200, g: 200, b: 200, a: 255 },
    },
    {
      id: "linewidth",
      type: "real",
      name: "Line width (%)",
      description: "Width of the mortar lines relative to the shorter texture dimension; 0% disables them.",
      default: 3.33,
      min: 0,
      max: 100,
    },
    {
      id: "brickwidth",
      type: "real",
      name: "Brick width (%)",
      description: "Width of each brick as a percentage of the texture width.",
      default: 40,
      min: 1,
      max: 100,
      group: "size",
    },
    {
      id: "brickheight",
      type: "real",
      name: "Brick height (%)",
      description: "Height of each brick as a percentage of the texture height.",
      default: 15,
      min: 1,
      max: 100,
      group: "size",
    },
    {
      id: "offsetx",
      type: "integer",
      name: "Horizontal offset (%)",
      description: "Moves the brickwork horizontally as a percentage of the texture width.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "offsety",
      type: "integer",
      name: "Vertical offset (%)",
      description: "Moves the brickwork vertically as a percentage of the texture height.",
      default: 0,
      min: -100,
      max: 100,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: begin with the optional background. Without one, begin with
    // transparency. Mortar pixels will be painted over this image once each.
    const pixels = output.data;
    const background = inputs.Background?.data;
    if (background) pixels.set(background);
    else pixels.fill(0);

    const width = size.width;
    const height = size.height;
    const stride = output.stride;
    const colour = settings.color;

    // Step 2: convert the percentage controls into pixels. A positive mortar
    // width smaller than one pixel is rounded up so that it remains visible.
    if (settings.linewidth <= 0 || colour.a === 0) return;
    const lineWidth = Math.max(
      1,
      Math.round(settings.linewidth * Math.min(width, height) / 100),
    );
    const brickWidth = Math.max(1, Math.round(settings.brickwidth * width / 100));
    const brickHeight = Math.max(1, Math.round(settings.brickheight * height / 100));
    const offsetX = settings.offsetx * width / 100;
    const offsetY = settings.offsety * height / 100;
    const columnStep = brickWidth + lineWidth;
    const rowStep = brickHeight + lineWidth;

    // Alpha values become fractions from 0 to 1 while the mortar is composited.
    const mortarAlpha = colour.a / 255;
    const remainingBackground = 1 - mortarAlpha;

    // Step 3: examine every output pixel and decide whether it belongs to mortar.
    // Horizontal lines separate brick rows. Vertical joints repeat within a row
    // and move by half a brick on alternating rows to make a running bond.
    for (let y = 0; y < height; ++y) {
      const relativeY = y - offsetY;
      const brickRow = Math.floor(relativeY / rowStep);
      const positionInRow = ((relativeY % rowStep) + rowStep) % rowStep;
      const horizontalMortar = positionInRow < lineWidth;
      const rowShift = brickRow % 2 === 0 ? 0 : columnStep / 2;
      let pixelOffset = y * stride;

      for (let x = 0; x < width; ++x) {
        const relativeX = x - offsetX - rowShift;
        const positionInColumn = ((relativeX % columnStep) + columnStep) % columnStep;
        const verticalMortar = positionInColumn < lineWidth;

        if (horizontalMortar || verticalMortar) {
          // Step 4: paint this mortar pixel exactly once. Intersections therefore
          // have the same opacity as every other part of a translucent line.
          const backgroundAlpha = pixels[pixelOffset + 3] / 255;
          if (mortarAlpha >= 1 || backgroundAlpha === 0) {
            pixels[pixelOffset] = colour.r;
            pixels[pixelOffset + 1] = colour.g;
            pixels[pixelOffset + 2] = colour.b;
            pixels[pixelOffset + 3] = colour.a;
          } else {
            const resultAlpha = mortarAlpha + backgroundAlpha * remainingBackground;
            pixels[pixelOffset] = Math.round(
              (colour.r * mortarAlpha
                + pixels[pixelOffset] * backgroundAlpha * remainingBackground) / resultAlpha,
            );
            pixels[pixelOffset + 1] = Math.round(
              (colour.g * mortarAlpha
                + pixels[pixelOffset + 1] * backgroundAlpha * remainingBackground) / resultAlpha,
            );
            pixels[pixelOffset + 2] = Math.round(
              (colour.b * mortarAlpha
                + pixels[pixelOffset + 2] * backgroundAlpha * remainingBackground) / resultAlpha,
            );
            pixels[pixelOffset + 3] = Math.round(resultAlpha * 255);
          }
        }

        pixelOffset += 4;
      }
    }
  },
};
