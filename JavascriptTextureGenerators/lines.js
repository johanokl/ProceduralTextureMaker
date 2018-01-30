var name = "LinesJS";
var numSlots = 1;
// Set separateColorChannels to true to have one array index per color
// If set to false all four channels are interleaved in one 32 bit value.
var separateColorChannels = false;

function generate(data, sourceImg) {
  var obj = JSON.parse(data);
  var color = obj.color.a +
    (obj.color.b << 8) +
    (obj.color.g << 16) +
    (obj.color.r << 24);
  obj.imagewidth = parseInt(obj.imagewidth);
  obj.imageheight = parseInt(obj.imageheight);
  obj.distance = Math.round(obj.distance * obj.imageheight / 100);
  obj.height = Math.round(obj.height * obj.imageheight / 100);
  obj.offset = Math.round(obj.offset * obj.imageheight / 100);

  if (obj.imageheight > 0) {
    var x, y;
    for (y = 0; y < obj.imageheight; y++) {
      var linestart = y * obj.imagewidth;
      if (((y + obj.offset) % (obj.height + obj.distance)) > obj.distance) {
        for (x = 0; x < obj.imagewidth; x++) {
          dest[linestart + x] = color;
        }
      } else if (sourceImg) {
        for (x = 0; x < obj.imagewidth; x++) {
          dest[linestart + x] = sourceImg[linestart + x];
        }
      } else {
        for (x = 0; x < obj.imagewidth; x++) {
          dest[linestart + x] = 0;
        }
      }
    }
  }
  return dest;
}

function getSettings() {
  return {
    color: {
      name: "Line color",
      defaultvalue: {
        r: 255,
        g: 100,
        b: 50,
        a: 255
      },
      type: "color",
      order: 1
    },
    height: {
      name: "Line height",
      defaultvalue: 10,
      min: 0,
      max: 100,
      type: "integer",
      order: 2
    },
    distance: {
      name: "Distance",
      defaultvalue: 10,
      min: 0,
      max: 100,
      type: "integer",
      order: 3
    },
    offset: {
      name: "Offset",
      defaultvalue: 0,
      min: 0,
      max: 100,
      type: "integer",
      order: 4
    }
  };
}
