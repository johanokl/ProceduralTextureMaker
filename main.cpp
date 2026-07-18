
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/applicationtheme.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char** argv) {
   QSurfaceFormat format;
   format.setRenderableType(QSurfaceFormat::OpenGL);
   format.setVersion(2, 1);
   format.setProfile(QSurfaceFormat::CompatibilityProfile);
   format.setDepthBufferSize(24);
   QSurfaceFormat::setDefaultFormat(format);

   QCoreApplication::setOrganizationName("Johan Lindqvist");
   QCoreApplication::setOrganizationDomain("github.com/johanokl");
   QCoreApplication::setApplicationName("ProceduralTextureMaker");

   TexGenApplication app(argc, argv);
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
