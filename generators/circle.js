
const generator = {
  apiVersion: 1,
  name: "Circle",
  description: "Draws a rotatable circle, ellipse, or ring on an optional canvas.",
  type: "generator",
  inputs: ["Canvas"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour used to draw the ellipse or ring.",
      default: { r: 200, g: 100, b: 0, a: 255 },
    },
    {
      id: "innerradius",
      type: "real",
      name: "Inner radius (%)",
      description: "Inner radius as a percentage of half the texture height.",
      default: 0,
      min: 0,
      max: 200,
    },
    {
      id: "outerradius",
      type: "real",
      name: "Outer radius (%)",
      description: "Outer radius as a percentage of half the texture height.",
      default: 100,
      min: 0,
      max: 200,
    },
    {
      id: "horizontalscale",
      type: "real",
      name: "Horizontal scale (%)",
      description: "Ellipse width as a percentage of its height.",
      default: 100,
      min: 1,
      max: 400,
    },
    {
      id: "rotation",
      type: "real",
      name: "Rotation (°)",
      description: "Rotates the ellipse around its centre.",
      default: 0,
      min: -360,
      max: 360,
    },
    {
      id: "offsetleft",
      type: "real",
      name: "Horizontal offset (%)",
      description: "Moves the centre horizontally as a percentage of the texture width.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "offsettop",
      type: "real",
      name: "Vertical offset (%)",
      description: "Moves the centre vertically as a percentage of the texture height.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "antialiasing",
      type: "boolean",
      name: "Antialiasing",
      description: "Smooths the edges of the ellipse and its cut-out.",
      default: true,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: copy the optional canvas into the output. The ellipse will be
    // painted over this image. Without a canvas, start with a transparent image.
    const pixels = output.data;
    const canvas = inputs.Canvas?.data;
    if (canvas) pixels.set(canvas);
    else pixels.fill(0);

    // Step 2: convert percentages from the settings panel into pixel measurements.
    const width = size.width;
    const height = size.height;
    const stride = output.stride;
    const centerX = width / 2 + settings.offsetleft * width / 100;
    const centerY = height / 2 + settings.offsettop * height / 100;

    // The radius settings use half the texture height as 100%. An inner radius
    // greater than zero removes the middle and turns the ellipse into a ring.
    const innerRadius = settings.innerradius * height / 200;
    const outerRadius = settings.outerradius * height / 200;
    const innerRadiusSquared = innerRadius * innerRadius;
    const outerRadiusSquared = outerRadius * outerRadius;

    // A horizontal scale of 1 is a circle. Smaller or larger values make an ellipse.
    const horizontalScale = settings.horizontalscale / 100;
    const inverseHorizontalScale = 1 / horizontalScale;

    // JavaScript's trigonometric functions use radians rather than degrees.
    const radians = settings.rotation * Math.PI / 180;
    const cosine = Math.cos(radians);
    const sine = Math.sin(radians);
    const antialiasing = settings.antialiasing;
    if (outerRadius <= innerRadius || outerRadius <= 0) return;

    const colour = settings.color;

    // Step 3: find the smallest axis-aligned box containing the rotated ellipse.
    // Restricting the loops to this box avoids checking unrelated image pixels.
    // The extra pixel gives antialiasing room to draw its partially covered edge.
    const horizontalRadius = outerRadius * horizontalScale;
    const padding = antialiasing ? 1 : 0;
    const extentX = Math.sqrt(
      horizontalRadius * horizontalRadius * cosine * cosine
        + outerRadius * outerRadius * sine * sine,
    ) + padding;
    const extentY = Math.sqrt(
      horizontalRadius * horizontalRadius * sine * sine
        + outerRadius * outerRadius * cosine * cosine,
    ) + padding;
    const left = Math.max(0, Math.floor(centerX - extentX));
    const right = Math.min(width - 1, Math.ceil(centerX + extentX));
    const top = Math.max(0, Math.floor(centerY - extentY));
    const bottom = Math.min(height - 1, Math.ceil(centerY + extentY));

    // Step 4: examine each pixel in the box. Pixel positions are transformed in
    // the opposite direction of the rotation. In that local coordinate system,
    // the ellipse is horizontal and its coverage is straightforward to calculate.
    for (let y = top; y <= bottom; ++y) {
      const yFromCenter = y + 0.5 - centerY;
      let localX = cosine * (left + 0.5 - centerX) + sine * yFromCenter;
      let localY = -sine * (left + 0.5 - centerX) + cosine * yFromCenter;
      let pixelOffset = y * stride + left * 4;

      for (let x = left; x <= right; ++x) {
        // Undo the horizontal stretch. In this adjusted coordinate system the
        // ellipse is a circle, so its radius is simple to calculate.
        const circleX = localX * inverseHorizontalScale;
        const distanceSquared = circleX * circleX + localY * localY;
        let coverage;

        if (antialiasing) {
          // The exact centre belongs to a solid ellipse, but not to a ring with
          // an inner cut-out. It is handled separately to avoid division by zero.
          if (distanceSquared === 0) {
            coverage = innerRadius > 0 ? 0 : 1;
          } else {
            const distanceFromCentre = Math.sqrt(distanceSquared);

            // Stretching an ellipse means that one image pixel changes the circle
            // radius by different amounts near its sides and top. This converts
            // radius units back to image pixels so the smooth edge stays even.
            const radiusChangePerPixel = Math.hypot(
              circleX * inverseHorizontalScale,
              localY,
            ) / distanceFromCentre;

            // Coverage is the fraction of the pixel covered by the ellipse. It is
            // 0 outside, 1 inside, and between 0 and 1 along a smoothed edge.
            const outerCoverage = Math.max(
              0,
              Math.min(
                1,
                (outerRadius - distanceFromCentre) / radiusChangePerPixel + 0.5,
              ),
            );
            const cutoutCoverage = innerRadius > 0
              ? Math.max(
                0,
                Math.min(
                  1,
                  (innerRadius - distanceFromCentre) / radiusChangePerPixel + 0.5,
                ),
              )
              : 0;

            // A ring is the outer ellipse minus its inner cut-out. Subtraction
            // also makes an extremely thin ring fade correctly where both smooth
            // edges overlap, instead of multiplying two partial coverages.
            coverage = Math.max(0, outerCoverage - cutoutCoverage);
          }
        } else {
          // Without antialiasing, a pixel is either completely inside or outside.
          coverage = distanceSquared <= outerRadiusSquared
              && distanceSquared >= innerRadiusSquared
            ? 1
            : 0;
        }

        // Combine the ellipse colour with the existing canvas pixel. Alpha is
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
