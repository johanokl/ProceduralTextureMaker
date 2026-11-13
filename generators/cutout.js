/*
 * This file is intended to be easy to customise.
 *
 * Image supplies the colours and alpha to keep. Mask removes parts of Image:
 * transparent mask pixels leave it unchanged, opaque pixels remove it, and
 * partial transparency creates smooth edges. Swap the connections to reverse
 * which image is cut out.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes.
 * Each pixel uses four bytes, with alpha at offset 3.
 */
const generator = {
  apiVersion: 1,
  name: "Cutout",
  description: "Removes parts of Image using the Mask's alpha channel.",
  type: "combiner",
  inputs: ["Image", "Mask"],
  settings: [],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    const image = inputs.Image?.data;
    const mask = inputs.Mask?.data;
    const destination = output.data;

    // Step 1: start with Image. Without it, return transparent black even if a
    // Mask is connected. Without a Mask, leave Image unchanged.
    if (!image) {
      destination.fill(0);
      return;
    }
    destination.set(image);
    if (!mask) return;

    // Step 2: keep the fraction of image alpha not removed by the mask.
    // Round to the nearest byte value and leave all copied colour channels
    // unchanged, including those of pixels that become fully transparent.
    const pixelCount = size.width * size.height;
    for (let pixel = 0; pixel < pixelCount; ++pixel) {
      const alphaOffset = pixel * 4 + 3;
      const imageAlpha = image[alphaOffset];
      const maskAlpha = mask[alphaOffset];
      destination[alphaOffset] = Math.round(imageAlpha * (255 - maskAlpha) / 255);
    }
  },
};
