
const generator = {
  apiVersion: 1,
  name: "Square",
  description: "Draws a rotatable rectangle with an optional rectangular cut-out.",
  type: "generator",
  inputs: ["Canvas"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour used to draw the rectangle.",
      default: { r: 200, g: 100, b: 0, a: 255 },
    },
    {
      id: "width",
      type: "real",
      name: "Width (%)",
      description: "Width of the rectangle as a percentage of the texture width.",
      default: 80,
      min: 0,
      max: 200,
      group: "size",
    },
    {
      id: "height",
      type: "real",
      name: "Height (%)",
      description: "Height of the rectangle as a percentage of the texture height.",
      default: 80,
      min: 0,
      max: 200,
      group: "size",
    },
    {
      id: "rotation",
      type: "real",
      name: "Rotation (°)",
      description: "Rotates the rectangle around its centre.",
      default: 50,
      min: 0,
      max: 360,
    },
    {
      id: "offsetleft",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the rectangle horizontally from the texture centre.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "offsettop",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the rectangle vertically from the texture centre.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "cutoutwidth",
      type: "real",
      name: "Cut-out width (%)",
      description: "Width of the inner area left unpainted, relative to the rectangle width.",
      default: 0,
      min: 0,
      max: 100,
      group: "cutout",
    },
    {
      id: "cutoutheight",
      type: "real",
      name: "Cut-out height (%)",
      description: "Height of the inner area left unpainted, relative to the rectangle height.",
      default: 0,
      min: 0,
      max: 100,
      group: "cutout",
    },
    {
      id: "antialiasing",
      type: "boolean",
      name: "Antialiasing",
      description: "Smooths the edges of the rectangle and its cut-out.",
      default: true,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional canvas into the output. The rectangle will be
    // painted over this image. Without a canvas, start with a transparent image.
    const pixels = output.data;
    const canvas = inputs.Canvas?.data;
    if (canvas) pixels.set(canvas);
    else pixels.fill(0);

    // Step 2: convert percentages from the settings panel into pixel measurements.
    const width = size.width;
    const height = size.height;
    const stride = output.stride;
    const rectangleWidth = settings.width * width / 100;
    const rectangleHeight = settings.height * height / 100;
    if (rectangleWidth <= 0 || rectangleHeight <= 0 || settings.color.a === 0) return;

    // Distances are measured from the centre, so half sizes are more convenient.
    const halfWidth = rectangleWidth / 2;
    const halfHeight = rectangleHeight / 2;
    const cutoutHalfWidth = halfWidth * settings.cutoutwidth / 100;
    const cutoutHalfHeight = halfHeight * settings.cutoutheight / 100;
    const hasCutout = cutoutHalfWidth > 0 && cutoutHalfHeight > 0;
    if (cutoutHalfWidth >= halfWidth && cutoutHalfHeight >= halfHeight) return;

    // JavaScript's trigonometric functions use radians rather than degrees.
    const radians = settings.rotation * Math.PI / 180;
    const cosine = Math.cos(radians);
    const sine = Math.sin(radians);
    const antialiasing = settings.antialiasing;
    const centerX = width / 2 + settings.offsetleft * width / 100;
    const centerY = height / 2 + settings.offsettop * height / 100;

    // Step 3: find the smallest axis-aligned box containing the rotated rectangle.
    // Restricting the loops to this box avoids checking unrelated image pixels.
    // The extra pixel gives antialiasing room to draw its partially covered edge.
    const padding = antialiasing ? 1 : 0;
    const extentX = Math.abs(cosine) * halfWidth + Math.abs(sine) * halfHeight + padding;
    const extentY = Math.abs(sine) * halfWidth + Math.abs(cosine) * halfHeight + padding;
    const left = Math.max(0, Math.floor(centerX - extentX));
    const right = Math.min(width - 1, Math.ceil(centerX + extentX));
    const top = Math.max(0, Math.floor(centerY - extentY));
    const bottom = Math.min(height - 1, Math.ceil(centerY + extentY));
    const colour = settings.color;

    // Step 4: examine each pixel in the box. Pixel positions are transformed in
    // the opposite direction of the rotation. In that local coordinate system,
    // the rectangle is upright and checking whether a pixel is inside is simple.
    for (let y = top; y <= bottom; ++y) {
      const yFromCenter = y + 0.5 - centerY;
      let localX = cosine * (left + 0.5 - centerX) + sine * yFromCenter;
      let localY = -sine * (left + 0.5 - centerX) + cosine * yFromCenter;
      let pixelOffset = y * stride + left * 4;

      for (let x = left; x <= right; ++x) {
        // Measure how far this pixel is from the nearest outer edge. The distance
        // is positive inside the rectangle and negative outside it.
        const distanceFromOuterEdge = Math.min(
          halfWidth - Math.abs(localX),
          halfHeight - Math.abs(localY),
        );

        // Coverage is the fraction of the pixel covered by the rectangle. Hard
        // edges use only 0 or 1. Antialiased edges can use values between them.
        let coverage;
        if (antialiasing) {
          coverage = Math.max(0, Math.min(1, distanceFromOuterEdge + 0.5));
        } else {
          coverage = distanceFromOuterEdge >= 0 ? 1 : 0;
        }

        // Remove the part covered by the optional inner rectangle. A frame is the
        // outer rectangle minus this cut-out, so subtract their pixel coverages.
        if (coverage > 0 && hasCutout) {
          const distanceFromCutoutEdge = Math.min(
            cutoutHalfWidth - Math.abs(localX),
            cutoutHalfHeight - Math.abs(localY),
          );
          let cutoutCoverage;
          if (antialiasing) {
            cutoutCoverage = Math.max(0, Math.min(1, distanceFromCutoutEdge + 0.5));
          } else {
            cutoutCoverage = distanceFromCutoutEdge >= 0 ? 1 : 0;
          }
          coverage = Math.max(0, coverage - cutoutCoverage);
        }

        // Combine the rectangle colour with the existing canvas pixel. Alpha is
        // converted from its byte range (0–255) to a fraction (0–1) for the maths.
        if (coverage > 0) {
          const sourceAlpha = colour.a * coverage / 255;
          const canvasAlpha = pixels[pixelOffset + 3] / 255;

          // An opaque source replaces the canvas. A transparent canvas has no
          // existing colour to preserve, so it can also be assigned directly.
          if (sourceAlpha >= 1 || canvasAlpha === 0) {
            pixels[pixelOffset] = colour.r;
            pixels[pixelOffset + 1] = colour.g;
            pixels[pixelOffset + 2] = colour.b;
            pixels[pixelOffset + 3] = Math.round(sourceAlpha * 255);
          } else {
            // For two translucent colours, retain the visible contribution from
            // both. The result is stored as ordinary, non-premultiplied RGBA.
            const remainingCanvas = 1 - sourceAlpha;
            const resultAlpha = sourceAlpha + canvasAlpha * remainingCanvas;
            pixels[pixelOffset] = Math.round(
              (colour.r * sourceAlpha
                + pixels[pixelOffset] * canvasAlpha * remainingCanvas) / resultAlpha,
            );
            pixels[pixelOffset + 1] = Math.round(
              (colour.g * sourceAlpha
                + pixels[pixelOffset + 1] * canvasAlpha * remainingCanvas) / resultAlpha,
            );
            pixels[pixelOffset + 2] = Math.round(
              (colour.b * sourceAlpha
                + pixels[pixelOffset + 2] * canvasAlpha * remainingCanvas) / resultAlpha,
            );
            pixels[pixelOffset + 3] = Math.round(resultAlpha * 255);
          }
        }

        // Moving one image pixel to the right changes the rotated local position
        // by (cosine, -sine). Updating it is cheaper and clearer than rotating again.
        localX += cosine;
        localY -= sine;
        pixelOffset += 4; // Each pixel contains four bytes: red, green, blue, and alpha.
      }
    }
  },
};
