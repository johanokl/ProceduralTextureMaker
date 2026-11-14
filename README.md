# ProceduralTextureMaker

## Introduction

ProceduralTextureMaker generates texture images in arbitrary sizes based on graphs of nodes that
take other nodes' output as input and transform them into new images.

Nodes are added dynamically and connected to each other graphically.

Custom UI widgets are created for each node based on the texture generator's user configurable
settings, making it easy to try out new configurations and watch how changes affect the final
output.

The project files with texture graphs can be saved to and loaded from JSON files, and images can be
exported to PNG files.

A number of texture generators written in C++ and Javascript are included with the project.

For more general information about procedural textures, see
<https://en.wikipedia.org/wiki/Procedural_texture>

## Screenshot

![Screenshot 1](https://i.imgur.com/Ikn4oLt.png)

## Examples

Three example textures have been added to the repository:

- [Rose](examples/rose.ptmg)
- [Wall](examples/wall.ptmg)
- [Space](examples/space.ptmg)

## Technical Details

The application is written in C++ and uses the Qt framework.

It uses multiple threads on multiple CPU cores where supported, so that CPU intensive texture
calculations don't affect the UI performance.

It's easy to extend the application by adding new generators, especially ones written in Javascript
as those can be loaded dynamically from external files.

## How to build

The build requires:

- Qt 6 with the Qt Base and Qt Declarative modules
- CMake 3.21 or newer
- Ninja
- A C++17 compiler supported by the installed Qt version

Do not mix Qt libraries built for different compilers or architectures.

### Qt Creator

Qt Creator is the recommended IDE for building and developing the application. Open the repository
as a CMake project and configure it with a Qt 6 desktop kit.

The following platform instructions describe how to install the dependencies and build from a
terminal.

### Windows

Install Qt using the [Qt Online Installer](https://www.qt.io/download-open-source). The default "Qt
for desktop development" installation includes a matching MinGW toolchain. Make sure CMake and Ninja
are also selected. This avoids installing and configuring a compiler separately.

To build from PowerShell, add the selected Qt kit, MinGW, CMake, and Ninja `bin` directories to
`PATH`, then run:

```powershell
cmake --preset release
cmake --build --preset release
```

If CMake cannot find Qt, pass the directory containing the selected Qt kit:

```powershell
cmake --preset release -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\mingw_64"
cmake --build --preset release
```

To run the application:

```powershell
.\build\release\ProceduralTextureMaker.exe
```

### Linux

Install the compiler, CMake, and Qt 6 development packages from your distribution package manager.
On Debian and Ubuntu based distributions:

```sh
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev libgl1-mesa-dev
cmake --preset release
cmake --build --preset release
```

If Qt was installed with the Qt online installer instead of the system package manager, pass the Qt
installation prefix to CMake:

```sh
cmake --preset release -DCMAKE_PREFIX_PATH="$HOME/Qt/6.x.x/gcc_64"
cmake --build --preset release
```

To run the application:

```sh
./build/release/ProceduralTextureMaker
```

### macOS

Install the Xcode command-line tools, CMake, Ninja, and Qt 6. With Homebrew:

```sh
xcode-select --install
brew install cmake ninja qt
cmake --preset release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build --preset release
```

If Qt was installed with the Qt online installer, use the matching Qt installation prefix instead:

```sh
cmake --preset release -DCMAKE_PREFIX_PATH="$HOME/Qt/6.x.x/macos"
cmake --build --preset release
```

To run the application:

```sh
open build/release/ProceduralTextureMaker.app
```

## Project files

Projects use the `.ptmg` extension and contain a JSON array of nodes:

```json
[
  {
    "id": "background",
    "name": "Blue background",
    "position": { "x": 0, "y": 0 },
    "generator": "Fill",
    "settings": { "color": "#336699ff" },
    "slots": {}
  },
  {
    "id": "output",
    "name": "Inverted image",
    "position": { "x": 300, "y": 0 },
    "generator": "Invert",
    "settings": {
      "invertRed": true,
      "invertGreen": true,
      "invertBlue": true,
      "invertAlpha": false
    },
    "slots": { "Image": "background" }
  }
]
```

Each `id` is a nonempty, unique, case-sensitive string. Edit it in the node's **ID** field, which
trims surrounding spaces. New nodes receive a generated ID. Display names can be shared by several
nodes. Connections refer to IDs, and renaming an ID updates the references written when the project
is saved. The CLI's `--node` option uses the same ID.

Settings use JSON numbers, booleans, and strings. Colours are strings in `#RRGGBBAA` or `#RRGGBB`
notation, and choice settings use their option text. The keys in `slots` are the generator's input
names; unconnected inputs can be omitted. `position`, `settings`, and `slots` can be omitted when
their defaults are sufficient.

## Command-line export

`ProceduralTextureMaker --no-gui` loads a project and writes a PNG without opening an application
window. When the project has one final sink node, it is selected automatically:

```sh
ProceduralTextureMaker --no-gui --size 1024x1024 examples/rose.ptmg rose.png
```

Select an intermediate node by its persisted ID, or inspect available IDs first:

```sh
ProceduralTextureMaker --no-gui --list-nodes examples/rose.ptmg
ProceduralTextureMaker --no-gui --node 10 --size 512x512 examples/rose.ptmg exported.png
```

Exports have no time limit by default; use `--timeout SECONDS` to set one explicitly.

The exporter refuses to replace an existing image unless `--force` is supplied. Projects using
external JavaScript generators can load them explicitly with `--js-dir /path/to/generators`.

### CLI mode on Windows

Starting the executable in CLI mode on Windows returns to the prompt immediately.

In Command Prompt, use `start /wait` to stay in the foreground, see progress, and cancel with
Ctrl+C:

```cmd
start /wait "" ProceduralTextureMaker.exe ^
    --no-gui --node 10 --size 512x512 ^
    examples/rose.ptmg exported.png
```

In PowerShell, use `Start-Process`, `-Wait`, and `-NoNewWindow` to keep the export in the current
terminal, see progress, and cancel with Ctrl+C:

```powershell
Start-Process -FilePath .\ProceduralTextureMaker.exe `
    -NoNewWindow -Wait `
    -ArgumentList '--no-gui --node 10 --size 512x512 examples/rose.ptmg exported.png'
```

## JavaScript generators

JavaScript is the preferred way to add custom texture generators.

Print the bundled starter template or the built-in Mask source without locating installation files:

```sh
ProceduralTextureMaker --print-js-template > my-generator.js
ProceduralTextureMaker --print-js-generator Mask > my-mask.js
```

Configure a JavaScript directory in the application settings and select **Edit > Reload JavaScript
Generators** after editing. External scripts appear under `Custom Generators`, `Custom Filters`, or
`Custom Combiners`; bundled JavaScript generators appear alongside native built-ins.

See the [JavaScript generators](docs/javascript.md) document for a guide on how to implement and add
texture generators in JavaScript.

## Tests

Enable the Qt Test/CTest integration suite when configuring, then run the debug test preset:

```sh
cmake --preset debug -DBUILD_TESTING=ON
cmake --build --preset debug
ctest --preset debug
```

The suite covers the base graph model, JSON persistence, synchronous and background rendering,
settings isolation, all built-in generators, JavaScript generators, PNG export, and command-line
mode through separate application processes.

The offscreen UI smoke test is enabled by default and verifies application startup and project
loading. Environments that cannot run Qt offscreen tests can disable it with
`-DPROCEDURAL_TEXTURE_MAKER_UI_TESTS=OFF`.

### Static release builds

The Qt packages supplied by standard installers and package managers normally use shared libraries.
A release without distributable Qt DLLs or frameworks requires a separately installed
[static build of Qt](https://doc.qt.io/qt-6/deployment.html#static-linking).

Configure the application with the static Qt installation prefix:

```sh
cmake --preset static-release -DCMAKE_PREFIX_PATH="/path/to/static/Qt"
cmake --build --preset static-release
```

The `static-release` preset rejects shared Qt installations instead of producing a partially dynamic
build. On Windows, it also links the compiler runtime statically for MinGW and MSVC builds. The
result can still depend on libraries provided by the operating system.

## Code analysis and formatting

The repository uses clang-tidy and clang-format for C++, and ESLint with additional SonarJS and
Unicorn analysis for JavaScript. Install clang-tidy, clang-format, and Node.js 22 or later. Install
the JavaScript development dependencies and configure the debug preset once:

```sh
npm install
```

```sh
cmake --preset debug
```

Run clang-tidy with:

```sh
cmake --build --preset clang-tidy
```

Run JavaScript analysis directly or through CMake with:

```sh
npm run lint:js
cmake --build --preset eslint
```

To review all code with clang-tidy, ESLint, and clang-format:

```sh
cmake --build --preset review-all-code
```

To check formatting only:

```sh
cmake --build --preset clang-format-check
```

To format all C++ source and header files in place:

```sh
cmake --build --preset clang-format
```

Run `npm install` again after the JavaScript dependencies change. Reconfigure after changing CMake
files or installing a tool that was unavailable during the previous configuration.

## License

ProceduralTextureMaker is licensed under the [GNU General Public License v3.0](LICENSE).

## Author

[Johan Lindqvist](https://github.com/johanokl)

<johan.lindqvist@gmail.com>

## Acknowledgements

The [Rose example](examples/rose.ptmg) was created by [Emil Bardh](https://www.hardh.com/).

The application icon is from [Iconfinder](https://www.iconfinder.com/icons/28730/).
