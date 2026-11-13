/*
 * This file is intended to be easy to customise.
 *
 * Image supplies the colours. Alpha supplies the replacement alpha channel.
 * Existing image transparency is discarded rather than multiplied, unlike Mask.
 * Leave Alpha disconnected to make Image fully opaque.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes.
 * Each pixel uses four bytes, with alpha at offset 3.
 */
const generator = {
  apiVersion: 1,
  name: "Replace alpha",
  description: "Replaces Image's alpha using Alpha, or makes Image opaque when Alpha is missing.",
  type: "combiner",
  inputs: ["Image", "Alpha"],
  settings: [],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    const destination = output.data;
    const image = inputs.Image?.data;
    const alpha = inputs.Alpha?.data;

    // Without Image there are no colours to keep, so return transparent black.
    if (!image) {
      destination.fill(0);
      return;
    }
    destination.set(image);

    // Copy only the replacement alpha bytes. Without Alpha, write opaque 255.
    // The RGB channels stay unchanged, even when a pixel becomes transparent.
    for (let alphaOffset = 3; alphaOffset < destination.length; alphaOffset += 4) {
      destination[alphaOffset] = alpha ? alpha[alphaOffset] : 255;
    }
  },
};
