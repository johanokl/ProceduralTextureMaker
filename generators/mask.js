/*
 * This file is intended to be easy to customise.
 *
 * Image supplies the colours and alpha to keep. Mask selects the visible parts:
 * opaque mask pixels keep Image, transparent pixels remove it, and partial
 * transparency creates smooth edges. Cutout keeps the opposite parts.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes. The byte
 * position of pixel (x, y) is y * output.stride + x * 4.
 */
const generator = {
  apiVersion: 1,
  name: "Mask",
  description: "Keeps parts of Image using the Mask's alpha channel.",
  type: "combiner",
  inputs: ["Image", "Mask"],
  settings: [],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: read the two optional inputs. Image provides the colours and alpha.
    // Mask controls how much of that alpha is kept.
    const destination = output.data;
    const image = inputs.Image?.data;
    const mask = inputs.Mask?.data;

    // Without an Image there are no colour channels to keep, so return a fully
    // transparent result even if a Mask is connected.
    if (!image) {
      destination.fill(0);
      return;
    }

    // Step 2: begin with an unchanged copy of the Image. If no Mask is connected,
    // this copy is also the final result.
    destination.set(image);
    if (!mask) return;

    // Step 3: multiply image alpha by the fraction kept by the mask, then round
    // to the nearest byte. This preserves existing transparency and leaves RGB
    // unchanged, including the colours of fully transparent pixels.
    const pixelCount = size.width * size.height;
    for (let pixel = 0; pixel < pixelCount; ++pixel) {
      const alphaOffset = pixel * 4 + 3;
      destination[alphaOffset] = Math.round(image[alphaOffset] * mask[alphaOffset] / 255);
    }
  },
};
