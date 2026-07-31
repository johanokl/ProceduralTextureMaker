// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef BUILTINREGISTRY_H
#define BUILTINREGISTRY_H

class TextureProject;

/// Registers every built-in C++ texture generator with a project.
void registerBuiltInGenerators(TextureProject& project);

#endif  // BUILTINREGISTRY_H
