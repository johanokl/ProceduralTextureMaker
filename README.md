# ProceduralTextureMaker

## Introduction

ProceduralTextureMaker generates texture images in arbitrary sizes based on graphs of nodes that
take other nodes' output as input and transform them into new images.

Nodes are added dynamically and connected to each other graphically.

Custom UI widgets are created for each node based on the texture generator's user configurable
settings, making it easy to try out new configurations and watch how changes affect the final
output.

The project files with texture graphs can be saved to and loaded from XML files, and images can be
exported to PNG files.

A number of texture generators written in C++ and Javascript are included with the project.

For more general information about procedural textures, see
<https://en.wikipedia.org/wiki/Procedural_texture>

## Screenshot

![Screenshot 1](https://i.imgur.com/Ikn4oLt.png)

## Examples

Three example textures have been added to the repository:

- [Rose](examples/rose.txl)
- [Wall](examples/wall.txl)
- [Space](examples/space.txl)

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

## Command-line export

`ProceduralTextureMaker --no-gui` loads a project and writes a PNG without opening an application
window. When the project has one final sink node, it is selected automatically:

```sh
ProceduralTextureMaker --no-gui --size 1024x1024 examples/rose.txl rose.png
```

Select an intermediate node by its persisted ID, or inspect available IDs first:

```sh
ProceduralTextureMaker --no-gui --list-nodes examples/rose.txl
ProceduralTextureMaker --no-gui --node 10 --size 512x512 examples/rose.txl intermediate.png
```

The exporter refuses to replace an existing image unless `--force` is supplied. Projects using
external JavaScript generators can load them explicitly with `--js-dir /path/to/generators`.
`--help`, `--help-all`, and `--version` also use the non-window startup path without requiring
`--no-gui`.

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

The suite covers the base graph model, XML compatibility, synchronous and background rendering,
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

The repository includes clang-tidy and clang-format configuration. Install clang-tidy and
clang-format, then configure the debug preset once:

```sh
cmake --preset debug
```

Run clang-tidy with:

```sh
cmake --build --preset clang-tidy
```

To review all code with clang-tidy and clang-format:

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

Reconfigure after changing CMake files or installing a tool that was unavailable during the previous
configuration.

## License

ProceduralTextureMaker is licensed under the [GNU General Public License v3.0](LICENSE).

## Author

[Johan Lindqvist](https://github.com/johanokl)

<johan.lindqvist@gmail.com>

## Acknowledgements

The [Rose example](examples/rose.txl) was created by [Emil Bardh](https://www.hardh.com/).

The application icon is from [Iconfinder](https://www.iconfinder.com/icons/28730/).
