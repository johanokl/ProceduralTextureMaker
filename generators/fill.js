const generator = {
  apiVersion: 1,
  name: "Fill",
  description: "Fills the entire texture with a solid colour.",
  type: "generator",
  inputs: [],

  // Each entry creates a control in the node settings panel.
  settings: [
    {
      id: "color",
      type: "color",
      name: "Colour",
      description: "Colour used to fill the texture.",
      default: { r: 255, g: 255, b: 255, a: 255 },
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    // Step 1: get the output byte array and the colour selected in the settings panel.
    // Fill does not need the texture dimensions or any input images, but they are
    // still part of the standard generate() arguments used by every generator.
    const pixels = output.data;
    const colour = settings.color;

    // Step 2: write the first pixel as four separate bytes. pixels.fill() cannot
    // be used here because it repeats one number, not a four-byte RGBA colour.
    pixels[0] = colour.r;
    pixels[1] = colour.g;
    pixels[2] = colour.b;
    pixels[3] = colour.a;

    // Step 3: copy the completed bytes into the empty part of the array. Each
    // iteration doubles the filled area: one pixel becomes two, then four, then
    // eight, and so on. pixels.set() performs each large copy outside JavaScript,
    // which keeps this fast even for large textures.
    for (let filled = 4; filled < pixels.length; filled *= 2) {
      // The final copy may be smaller when fewer than `filled` bytes remain.
      const copyLength = Math.min(filled, pixels.length - filled);
      pixels.set(pixels.subarray(0, copyLength), filled);
    }
  },
};
