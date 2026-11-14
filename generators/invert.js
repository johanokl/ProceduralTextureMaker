/*
 * Each checkbox inverts one channel: 0 becomes 255, 255 becomes 0, and values
 * in between are reversed. Red, green, and blue are checked by default.
 * Alpha controls transparency and is left unchanged unless checked.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes.
 * Each pixel uses four bytes.
 */
const generator = {
  apiVersion: 1,
  name: "Invert",
  description: "Selectively inverts the red, green, blue, and alpha channels.",
  type: "filter",
  inputs: ["Image"],

  // Boolean settings appear as checkboxes in the node settings panel.
  settings: [
    {
      id: "channelRed",
      type: "boolean",
      name: "Invert red",
      description: "Selects whether the red channel is inverted.",
      default: true,
    },
    {
      id: "channelGreen",
      type: "boolean",
      name: "Invert green",
      description: "Selects whether the green channel is inverted.",
      default: true,
    },
    {
      id: "channelBlue",
      type: "boolean",
      name: "Invert blue",
      description: "Selects whether the blue channel is inverted.",
      default: true,
    },
    {
      id: "channelAlpha",
      type: "boolean",
      name: "Invert alpha",
      description: "Selects whether the alpha channel is inverted.",
      default: false,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    const destination = output.data;
    const image = inputs.Image?.data;

    // Without Image, return transparent black instead of inverting empty pixels.
    if (!image) {
      destination.fill(0);
      return;
    }
    destination.set(image);

    // Copy unchecked channels unchanged. For each checked channel, visit its
    // byte in every pixel and subtract it from the maximum byte value, 255.
    const invert = [
      settings.channelRed,
      settings.channelGreen,
      settings.channelBlue,
      settings.channelAlpha,
    ];
    for (let channel = 0; channel < 4; ++channel) {
      if (!invert[channel]) continue;
      for (let offset = channel; offset < destination.length; offset += 4) {
        destination[offset] = 255 - image[offset];
      }
    }
  },
};
