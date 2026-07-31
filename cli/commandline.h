// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

class QCoreApplication;

/// @brief Returns whether the process arguments request the non-GUI command-line mode.
bool useCommandLineMode(int argc, char* argv[]);

/// @brief Parses command-line export options and performs the requested operation.
int runCommandLine(QCoreApplication& application);

#endif  // COMMANDLINE_H
