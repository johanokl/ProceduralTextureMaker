var name = "SolidJS";
var separateColorChannels = false;

function getInputSlots() {
  return [];
}

/// @brief Returns the fixture generator's empty setting schema.
function getSettings() {
  return {};
}

/// @brief Fills the requested image with a fixed RGBA color.
/// @param data JSON-encoded render dimensions and settings.
/// @return The populated destination pixel array.
function generate(data) {
  var args = JSON.parse(data);
  for (var i = 0; i < args.imagewidth * args.imageheight; ++i) {
    dest[i] = 0x123456ff;
  }
  return dest;
}
