/*
 * This file is intended to be easy to customise.
 *
 * The metadata describes the generator and the controls shown in the application.
 * This generator has no settings: it combines the alpha channel of one image with
 * the colour channels of another.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes. The byte
 * position of pixel (x, y) is y * output.stride + x * 4.
 */
const generator = {
  apiVersion: 1,
  name: "Mask",
  description: "Replaces the image's alpha channel with the mask image's alpha channel.",
  type: "combiner",
  inputs: ["Image", "Mask"],
  settings: [],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: read the two optional inputs. The Image provides red, green, and
    // blue. The Mask provides only the replacement alpha channel.
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

    // Step 3: replace every fourth byte. Offsets 0, 1, and 2 are the colour
    // channels; offset 3 is alpha. The RGB bytes copied above remain unchanged.
    const pixelCount = size.width * size.height;
    for (let pixel = 0; pixel < pixelCount; ++pixel) {
      const offset = pixel * 4;
      destination[offset + 3] = mask[offset + 3];
    }
  },
};
