/*
 * This file is intended to be easy to customise.
 *
 * Each output channel can use a channel from First or Second, or a fixed value.
 * None writes 0; Fill writes 255. Channels read from a missing input also use 0.
 * Fixed values work even when neither input is connected.
 *
 * The output.data array stores pixels as red, green, blue, and alpha bytes.
 * Each pixel uses four bytes.
 */
const channelSources = [
  "None",
  "Fill",
  "First's red",
  "First's green",
  "First's blue",
  "First's alpha",
  "Second's red",
  "Second's green",
  "Second's blue",
  "Second's alpha",
];

const generator = {
  apiVersion: 1,
  name: "Set channels",
  description: "Builds an image by assigning each output channel from an input or fixed value.",
  type: "combiner",
  inputs: ["First", "Second"],

  // Each setting selects the source of one output channel.
  settings: [
    {
      id: "channelRed",
      type: "choice",
      name: "Red channel source",
      description: "Selects the value copied into the output red channel.",
      values: channelSources,
      default: "First's red",
    },
    {
      id: "channelGreen",
      type: "choice",
      name: "Green channel source",
      description: "Selects the value copied into the output green channel.",
      values: channelSources,
      default: "First's green",
    },
    {
      id: "channelBlue",
      type: "choice",
      name: "Blue channel source",
      description: "Selects the value copied into the output blue channel.",
      values: channelSources,
      default: "First's blue",
    },
    {
      id: "channelAlpha",
      type: "choice",
      name: "Alpha channel source",
      description: "Selects the value copied into the output alpha channel.",
      values: channelSources,
      default: "First's alpha",
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    const destination = output.data;
    const first = inputs.First?.data;
    const second = inputs.Second?.data;
    const selections = [
      settings.channelRed,
      settings.channelGreen,
      settings.channelBlue,
      settings.channelAlpha,
    ];

    // Select a source once per output channel, then visit that byte in every
    // pixel. This avoids looking up the same setting for every pixel.
    for (let channel = 0; channel < 4; ++channel) {
      const choice = channelSources.indexOf(selections[channel]);

      // The first two choices are constants: None is zero and Fill is 255.
      if (choice < 2) {
        const value = choice === 1 ? 255 : 0;
        for (let offset = channel; offset < destination.length; offset += 4) {
          destination[offset] = value;
        }
        continue;
      }

      // The next four choices are First's RGBA channels, followed by Second's.
      // Subtract the two constant choices and wrap every four channels to find
      // the source byte: red is 0, green is 1, blue is 2, and alpha is 3.
      const source = choice < 6 ? first : second;
      const sourceChannel = (choice - 2) % 4;
      for (let pixelOffset = 0; pixelOffset < destination.length; pixelOffset += 4) {
        destination[pixelOffset + channel] = source ? source[pixelOffset + sourceChannel] : 0;
      }
    }
  },
};
