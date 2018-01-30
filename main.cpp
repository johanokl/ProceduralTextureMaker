/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QApplication>
#include "texgenapplication.h"

int main(int argc, char**argv)
{
   QCoreApplication::setOrganizationName("Johan Lindqvist");
   QCoreApplication::setOrganizationDomain("github.com/johanokl");
   QCoreApplication::setApplicationName("ProceduralTextureMaker");

   TexGenApplication app(argc, argv);
   return app.exec();
}
