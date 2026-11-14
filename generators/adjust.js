/*
 * Multiply scales selected channels by a percentage: 100 leaves them unchanged,
 * 50 halves them, and 200 doubles them. Add raises byte values; Subtract lowers them.
 * Results are rounded and limited to the valid range, 0 to 255.
 *
 * Red, green, and blue are checked by default. Check alpha to adjust transparency.
 * The output.data array stores pixels as red, green, blue, and alpha bytes.
 * Each pixel uses four bytes.
 */
const generator = {
  apiVersion: 1,
  name: "Adjust",
  description: "Adjusts selected colour and alpha channels by adding, subtracting, or multiplying.",
  type: "filter",
  inputs: ["Image"],

  // Boolean settings appear as checkboxes in the node settings panel.
  settings: [
    {
      id: "channelRed",
      type: "boolean",
      name: "Adjust red",
      description: "Selects whether the red channel is adjusted.",
      default: true,
    },
    {
      id: "channelGreen",
      type: "boolean",
      name: "Adjust green",
      description: "Selects whether the green channel is adjusted.",
      default: true,
    },
    {
      id: "channelBlue",
      type: "boolean",
      name: "Adjust blue",
      description: "Selects whether the blue channel is adjusted.",
      default: true,
    },
    {
      id: "channelAlpha",
      type: "boolean",
      name: "Adjust alpha",
      description: "Selects whether the alpha channel is adjusted.",
      default: false,
    },
    {
      id: "mode",
      type: "choice",
      name: "Adjustment mode",
      description: "Selects whether to multiply, add, or subtract the adjustment amount.",
      values: ["Multiply", "Add", "Subtract"],
      default: "Multiply",
    },
    {
      id: "level",
      type: "real",
      name: "Adjustment amount",
      description: "A percentage in Multiply mode or a channel-value amount in Add and Subtract modes.",
      default: 100,
      min: 0,
      max: 255,
    },
  ],

  // This function runs whenever the application renders the node.
  generate(size, settings, output, inputs) {
    const destination = output.data;
    const image = inputs.Image?.data;
    if (!image) {
      destination.fill(0);
      return;
    }
    destination.set(image);

    // Choose the operation once. Multiplication uses a percentage, while
    // addition and subtraction use the amount directly. Keep the amount in the
    // same 0-to-255 range as the slider, including hand-edited project values.
    const multiply = settings.mode === "Multiply";
    const amount = Math.max(0, Math.min(255, settings.level));
    const factor = amount / 100;
    const adjustment = settings.mode === "Subtract" ? -amount : amount;
    const adjust = [
      settings.channelRed,
      settings.channelGreen,
      settings.channelBlue,
      settings.channelAlpha,
    ];

    // Unchecked channels keep their copied values. Visit each checked channel's
    // byte in every pixel, then round and clamp the adjusted value.
    for (let channel = 0; channel < 4; ++channel) {
      if (!adjust[channel]) continue;
      for (let offset = channel; offset < destination.length; offset += 4) {
        const value = multiply ? image[offset] * factor : image[offset] + adjustment;
        destination[offset] = Math.max(0, Math.min(255, Math.round(value)));
      }
    }
  },
};
