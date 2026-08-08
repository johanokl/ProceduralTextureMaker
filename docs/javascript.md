# JavaScript generators

JavaScript is the easiest way to add new generators. Scripts can be edited and reloaded without
rebuilding ProceduralTextureMaker.

## Templates

Print the template from any installed executable:

```sh
ProceduralTextureMaker --print-js-template > my-generator.js
```

The built-in Mask combiner is also available as editable source:

```sh
ProceduralTextureMaker --print-js-generator Mask > my-mask.js
```

Rename a copied generator before loading it; built-in names are reserved. In the GUI, select a
JavaScript generator directory in Settings, enable JavaScript generators, and use **File > Reload
JavaScript files** after saving changes. Valid files remain available when another file is invalid,
and a broken edit does not replace the previous working revision.

The CLI accepts custom definitions through `--js-dir /path/to/generators`.

## Interface

Every new script defines one `generator` descriptor:

```js
const generator = {
  apiVersion: 1,
  name: "Blue Fill",
  description: "Fills the image with a solid blue colour.",
  type: "generator",
  inputs: [],

  settings: [
    {
      id: "opacity",
      type: "integer",
      name: "Opacity",
      description: "Controls the opacity of the generated colour.",
      default: 255,
      min: 0,
      max: 255,
    },
  ],

  generate(size, settings, output, inputs) {
    void size;
    void inputs;
    for (let offset = 0; offset < output.data.length; offset += 4) {
      output.data[offset] = 0; // red
      output.data[offset + 1] = 0; // green
      output.data[offset + 2] = 255; // blue
      output.data[offset + 3] = settings.opacity;
    }
  },
};
```

`type` is required and is one of `"generator"`, `"filter"`, or `"combiner"`. `inputs` contains
ordered, unique slot names. Type is never inferred from the number of inputs.

`generate(size, settings, output, inputs)` receives four separate, frozen objects. It writes the
complete image into `output.data` and normally returns nothing. Image dimensions are in `size`.
Missing input connections have no property in `inputs`.

## Settings

Supported setting types are:

| Type        | JavaScript default | Notes                                   |
| ----------- | ------------------ | --------------------------------------- |
| `integer`   | integer            | Optional `min` and `max`                |
| `real`      | finite number      | Optional `min` and `max`                |
| `boolean`   | boolean            |                                         |
| `string`    | string             | Single-line editor                      |
| `multiline` | string             | Multiline editor                        |
| `color`     | `{r, g, b, a}`     | Channels are integers from 0 to 255     |
| `choice`    | string             | Requires a unique string `values` array |

Each setting requires a unique, non-empty string `id`. Settings appear in the interface in array
order; there is no separate `order` property. All types also support `name`, `description`,
`group`, and `enabler` metadata. Use `settings: []` when a generator has no settings.

## Image layout and ownership

Every image view has this shape:

```js
{
  data: Uint8Array,
  width: 512,
  height: 512,
  stride: 2048,
  format: "rgba8"
}
```

The byte order is red, green, blue, alpha and matches `QImage::Format_RGBA8888` on every supported
platform. Use `TexGen.offset(x, y, image.stride)` for a byte offset. `TexGen.clamp8`, `TexGen.copy`,
and `TexGen.clear` are also available.

The `output.data` array is writable. Input buffers are isolated copies, so changing an input typed
array cannot corrupt a shared graph result; scripts should nevertheless treat inputs as read-only.
Output begins as transparent black.
