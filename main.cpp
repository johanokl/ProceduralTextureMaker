
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "cli/commandline.h"
#include "gui/applicationtheme.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QApplication>
#include <QSurfaceFormat>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <cstdio>
#include <io.h>
#endif

/// @brief Starts the GUI or runs command-line operations.
int main(int argc, char** argv) {
   const bool commandLineMode = useCommandLineMode(argc, argv);
#ifdef Q_OS_WIN
   if (commandLineMode) {
      // Keep redirected output, but connect missing streams to the calling terminal.
      const bool missingOutput =
          _fileno(stdout) < 0 || GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_UNKNOWN;
      const bool missingError =
          _fileno(stderr) < 0 || GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_UNKNOWN;
      if (AttachConsole(ATTACH_PARENT_PROCESS)) {
         if (missingOutput) {
            std::freopen("CONOUT$", "w", stdout);
         }
         if (missingError) {
            std::freopen("CONOUT$", "w", stderr);
         }
      }
   }
#endif
   if (!commandLineMode) {
      QSurfaceFormat format;
      format.setRenderableType(QSurfaceFormat::OpenGL);
      format.setVersion(2, 1);
      format.setProfile(QSurfaceFormat::CompatibilityProfile);
      format.setDepthBufferSize(24);
      QSurfaceFormat::setDefaultFormat(format);
   }

   QCoreApplication::setOrganizationName("Johan Lindqvist");
   QCoreApplication::setOrganizationDomain("github.com/johanokl");
   QCoreApplication::setApplicationName("ProceduralTextureMaker");
   QCoreApplication::setApplicationVersion("0.1.0");

   TexGenApplication app(argc, argv);
   if (commandLineMode) {
      return runCommandLine(app);
   }

   ApplicationTheme::apply(app);
   MainWindow* window = app.addWindow();
#ifndef Q_OS_MAC
   // On macOS the platform sends file-open events to the application.
   if (argc > 1) {
      window->openFile(QString(argv[1]));
   }
#endif
   return app.exec();
}
