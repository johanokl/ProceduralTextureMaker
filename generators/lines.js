const generator = {
  apiVersion: 1,
  name: "Lines",
  description: "Draws evenly spaced parallel lines over an optional input image.",
  type: "generator",
  inputs: ["Canvas"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Line colour",
      description: "Colour used to draw the lines.",
      default: { r: 255, g: 100, b: 50, a: 255 },
    },
    {
      id: "lineheight",
      type: "integer",
      name: "Line thickness (%)",
      description: "Thickness of each line relative to the shorter texture dimension.",
      default: 10,
      min: 0,
      max: 100,
    },
    {
      id: "spacing",
      type: "integer",
      name: "Gap size (%)",
      description: "Gap between adjacent lines relative to the shorter texture dimension.",
      default: 10,
      min: 0,
      max: 100,
    },
    {
      id: "offset",
      type: "integer",
      name: "Pattern phase (%)",
      description: "Shifts the pattern through one complete gap-and-line period.",
      default: 0,
      min: 0,
      max: 100,
    },
    {
      id: "angle",
      type: "real",
      name: "Angle (°)",
      description: "Rotation angle of the lines.",
      default: 0,
      min: 0,
      max: 180,
    },
    {
      id: "antialiasing",
      type: "boolean",
      name: "Antialiasing",
      description: "Smooths the edges of the lines.",
      default: true,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional canvas into the output. The lines will be painted
    // over this image. Without a canvas, start with a transparent image.
    const outputPixels = output.data;
    const canvas = inputs.Canvas?.data;
    if (canvas) outputPixels.set(canvas);
    else outputPixels.fill(0);

    const width = size.width;
    const height = size.height;
    const outputStride = output.stride;

    // Step 2: convert percentages from the settings panel into pixel measurements.
    // Using the shorter dimension prevents a wide or tall texture from stretching
    // the pattern. A 10% line therefore has the same proportions in either shape.
    const referenceSize = Math.min(width, height);
    const lineThickness = settings.lineheight * referenceSize / 100;
    const gapSize = settings.spacing * referenceSize / 100;
    const patternLength = lineThickness + gapSize;

    // A zero-width line has nothing to draw. A zero-length pattern would also make
    // the repeating remainder calculation below impossible.
    if (lineThickness <= 0 || patternLength <= 0 || settings.color.a === 0) return;

    // Phase is a percentage of the complete repeating period. Its full 0–100%
    // range can therefore reach every possible alignment, regardless of the
    // selected line thickness and gap size.
    const patternOffset = settings.offset * patternLength / 100;

    // Step 3: prepare the line angle. JavaScript's sine and cosine functions use
    // radians, while degrees are easier to select in the settings panel.
    const angleInRadians = settings.angle * Math.PI / 180;
    const cosine = Math.cos(angleInRadians);
    const sine = Math.sin(angleInRadians);
    const antialiasing = settings.antialiasing;
    const colour = settings.color;

    // Step 4: visit every output pixel. Projecting (x, y) onto the direction across
    // the lines turns a two-dimensional texture into one repeating measurement.
    for (let y = 0; y < height; ++y) {
      for (let x = 0; x < width; ++x) {
        const distanceAcrossPattern = x * cosine + y * sine - patternOffset;

        // JavaScript keeps the sign of a negative remainder. Adding one complete
        // pattern and taking the remainder again always produces 0..patternLength.
        const positionInPattern = (
          (distanceAcrossPattern % patternLength) + patternLength
        ) % patternLength;
        let coverage;

        if (gapSize <= 0) {
          // No gap means that the line colour covers the complete texture.
          coverage = 1;
        } else if (antialiasing) {
          // A period begins with a gap and ends with a line. Measure the nearest
          // edge, then make the distance negative while the pixel is in the gap.
          // Adding half a pixel turns that signed distance into partial coverage.
          let signedDistance;
          if (positionInPattern >= gapSize) {
            signedDistance = Math.min(
              positionInPattern - gapSize,
              patternLength - positionInPattern,
            );
          } else {
            signedDistance = -Math.min(
              gapSize - positionInPattern,
              positionInPattern,
            );
          }
          coverage = Math.max(0, Math.min(1, signedDistance + 0.5));
        } else {
          // Without antialiasing, a pixel is either completely in a gap or line.
          coverage = positionInPattern >= gapSize ? 1 : 0;
        }

        if (coverage <= 0) continue;

        const outputOffset = y * outputStride + x * 4;

        // Step 5: combine the line colour with the existing canvas pixel. Coverage
        // and the colour's alpha both contribute to the line's effective opacity.
        const lineAlpha = colour.a * coverage / 255;
        const canvasAlpha = outputPixels[outputOffset + 3] / 255;

        // An opaque line replaces the canvas. A transparent canvas has no existing
        // colour to preserve, so it can also be assigned directly.
        if (lineAlpha >= 1 || canvasAlpha === 0) {
          outputPixels[outputOffset] = colour.r;
          outputPixels[outputOffset + 1] = colour.g;
          outputPixels[outputOffset + 2] = colour.b;
          outputPixels[outputOffset + 3] = Math.round(lineAlpha * 255);
        } else {
          // For two translucent colours, retain the visible contribution from both.
          // The output is stored as ordinary, non-premultiplied RGBA.
          const remainingCanvas = 1 - lineAlpha;
          const resultAlpha = lineAlpha + canvasAlpha * remainingCanvas;
          outputPixels[outputOffset] = Math.round(
            (colour.r * lineAlpha
              + outputPixels[outputOffset] * canvasAlpha * remainingCanvas) / resultAlpha,
          );
          outputPixels[outputOffset + 1] = Math.round(
            (colour.g * lineAlpha
              + outputPixels[outputOffset + 1] * canvasAlpha * remainingCanvas) / resultAlpha,
          );
          outputPixels[outputOffset + 2] = Math.round(
            (colour.b * lineAlpha
              + outputPixels[outputOffset + 2] * canvasAlpha * remainingCanvas) / resultAlpha,
          );
          outputPixels[outputOffset + 3] = Math.round(resultAlpha * 255);
        }
      }
    }
  },
};
