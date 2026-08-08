const generator = {
  apiVersion: 1,
  name: "SolidJS",
  description: "Fills the image with a fixed RGBA color for integration tests.",
  type: "generator",
  inputs: [],
  settings: [],

  generate(size, settings, output, inputs) {
    void settings;
    void inputs;
    for (let index = 0; index < size.width * size.height; ++index) {
      const offset = index * 4;
      output.data[offset] = 0x12;
      output.data[offset + 1] = 0x34;
      output.data[offset + 2] = 0x56;
      output.data[offset + 3] = 0xff;
    }
  }
};
