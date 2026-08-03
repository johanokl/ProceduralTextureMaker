const generator = {
  apiVersion: 1,
  name: "Mask",
  description: "Replaces an image's alpha channel with that of the mask",
  type: "combiner",
  inputs: ["Image", "Mask"],
  settings: {},

  generate(size, settings, output, inputs) {
    const destination = output.data;
    const image = inputs.Image?.data;
    const mask = inputs.Mask?.data;

    // Keep unused API arguments visible in this small example.
    void settings;

    if (!image) {
      destination.fill(0);
      return;
    }

    destination.set(image);
    if (!mask) {
      return;
    }

    const pixelCount = size.width * size.height;
    for (let pixel = 0; pixel < pixelCount; ++pixel) {
      const offset = pixel * 4;
      destination[offset + 3] = mask[offset + 3]
    }
  }
};
