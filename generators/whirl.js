const generator = {
  apiVersion: 1,
  name: "Whirl",
  description: "Twists the input image around a configurable centre.",
  type: "filter",
  inputs: ["Image"],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "radius",
      type: "real",
      name: "Radius (%)",
      description: "Radius of the affected area as a percentage of the texture width.",
      default: 50,
      min: 0,
      max: 200,
    },
    {
      id: "strength",
      type: "real",
      name: "Strength",
      description: "Controls the amount of twisting; negative values reverse its direction.",
      default: 40,
      min: -500,
      max: 500,
    },
    {
      id: "offsetleft",
      type: "real",
      name: "Horizontal centre offset (%)",
      description: "Moves the centre of the whirl horizontally.",
      default: 0,
      min: -100,
      max: 100,
    },
    {
      id: "offsettop",
      type: "real",
      name: "Vertical centre offset (%)",
      description: "Moves the centre of the whirl vertically.",
      default: 0,
      min: -100,
      max: 100,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: begin with a transparent output. A filter has nothing to transform
    // when its Image input is disconnected, so the transparent image is returned.
    const outputPixels = output.data;
    outputPixels.fill(0);

    const sourceImage = inputs.Image;
    if (!sourceImage) return;

    // Copy the complete input first. The whirl only replaces pixels inside its
    // circular area, so everything outside that circle should remain unchanged.
    const sourcePixels = sourceImage.data;
    outputPixels.set(sourcePixels);

    const width = size.width;
    const height = size.height;
    const sourceStride = sourceImage.stride;
    const outputStride = output.stride;

    // Step 2: convert the settings from percentages into pixel measurements.
    // Radius follows the texture width, while each centre offset follows the size
    // of its own axis. This matches the behaviour of the native Whirl generator.
    const radius = settings.radius * width / 100;
    const horizontalCentreOffset = settings.offsetleft * width / 100;
    const verticalCentreOffset = settings.offsettop * height / 100;

    // The native generator divides the displayed strength by 80 before using it.
    // A positive value twists one way and a negative value twists the other way.
    const strength = settings.strength / 80;
    if (radius <= 0 || strength === 0) return;

    // Pixel indices represent their centres in this calculation. `(size - 1) / 2`
    // therefore finds the exact middle for both odd and even texture dimensions.
    const whirlCentreX = (width - 1) / 2 + horizontalCentreOffset;
    const whirlCentreY = (height - 1) / 2 + verticalCentreOffset;

    // Step 3: find the square containing the whirl circle and clip it to the image.
    // Restricting the loops to this square avoids examining unrelated pixels.
    const left = Math.max(0, Math.ceil(whirlCentreX - radius));
    const right = Math.min(width - 1, Math.floor(whirlCentreX + radius));
    const top = Math.max(0, Math.ceil(whirlCentreY - radius));
    const bottom = Math.min(height - 1, Math.floor(whirlCentreY + radius));
    const radiusSquared = radius * radius;

    // This factor converts the distance from the circle's edge into a rotation
    // angle. JavaScript's sine and cosine functions expect angles in radians.
    const angleFactor = 2 * Math.PI * strength / radiusSquared;

    // Step 4: examine each pixel in the square. Pixels outside the circular radius
    // stay unchanged because the source image was copied into the output above.
    for (let y = top; y <= bottom; ++y) {
      const verticalDistance = y - whirlCentreY;
      let outputOffset = y * outputStride + left * 4;

      for (let x = left; x <= right; ++x) {
        const horizontalDistance = x - whirlCentreX;

        // The Pythagorean theorem gives the squared distance from this pixel to the
        // whirl centre. Comparing squared values avoids an unnecessary square root
        // for pixels outside the circle.
        const distanceSquared = horizontalDistance * horizontalDistance
          + verticalDistance * verticalDistance;

        if (distanceSquared <= radiusSquared) {
          const distanceFromCentre = Math.sqrt(distanceSquared);
          const distanceFromEdge = radius - distanceFromCentre;

          // Squaring the distance from the edge makes the twist strongest near the
          // centre and lets it fade smoothly to no rotation at the circle's edge.
          const rotation = distanceFromEdge * distanceFromEdge * angleFactor;
          const cosine = Math.cos(rotation);
          const sine = Math.sin(rotation);

          // Step 5: rotate this pixel's position around the whirl centre to find
          // the source pixel whose colour belongs here in the output image.
          let sourceX = Math.round(
            horizontalDistance * cosine
              - verticalDistance * sine
              + whirlCentreX,
          );
          let sourceY = Math.round(
            verticalDistance * cosine
              + horizontalDistance * sine
              + whirlCentreY,
          );

          // A whirl near an edge can point anywhere outside the texture. Applying
          // the remainder twice wraps every positive or negative coordinate into
          // the valid range, including a coordinate exactly equal to a dimension.
          sourceX = ((sourceX % width) + width) % width;
          sourceY = ((sourceY % height) + height) % height;

          // A stride is the number of bytes from the start of one row to the next.
          // Multiplying x by four reaches that pixel's red, green, blue, and alpha.
          const sourceOffset = sourceY * sourceStride + sourceX * 4;
          outputPixels[outputOffset] = sourcePixels[sourceOffset];
          outputPixels[outputOffset + 1] = sourcePixels[sourceOffset + 1];
          outputPixels[outputOffset + 2] = sourcePixels[sourceOffset + 2];
          outputPixels[outputOffset + 3] = sourcePixels[sourceOffset + 3];
        }

        outputOffset += 4;
      }
    }
  },
};
