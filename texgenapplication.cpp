/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include "core/textureproject.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QFileOpenEvent>

/**
 * @brief TexGenApplication::TexGenApplication
 * @param argc Number of arguments passed (including the application name).
 * @param argv Array of arguments as C char strings.
 */
TexGenApplication::TexGenApplication(int argc, char* argv[]) : QApplication(argc, argv)
{
   addWindow();
#ifndef Q_OS_MAC
   // On Mac OS the platform handles this automatically,
   if (argc > 1) {
      mainwindows.first()->openFile(QString(argv[1]));
   }
#endif
}

/**
 * @brief TexGenApplication::quit
 * Tries to close all windows. Stops if one can't be closed.
 */
void TexGenApplication::quit()
{
   QMutableVectorIterator<MainWindow*> it(mainwindows);
   it.toBack();
   while (it.hasPrevious()) {
      if (it.previous()->close() == false) {
         return;
      }
   }
}

/**
 * @brief TexGenApplication::addWindow
 * Adds a new MainWindow.
 */
MainWindow* TexGenApplication::addWindow()
{
   auto* newWindow = new MainWindow(this);
   QObject::connect(newWindow, &MainWindow::windowTitleChanged,
                    this, &TexGenApplication::windowUpdated);
   QObject::connect(newWindow, &MainWindow::destroyed,
                    this, &TexGenApplication::removeWindow);
   mainwindows.push_back(newWindow);
   newWindow->show();
   emit windowsChanged();
   return newWindow;
}

/**
 * @brief TexGenApplication::removeWindow
 * @param windowObj The window to be removed.
 * Removes a MainWindows from the window list.
 */
void TexGenApplication::removeWindow(QObject* windowObj)
{
   int index = mainwindows.indexOf(dynamic_cast<MainWindow*>(windowObj));
   if (index == -1) {
      return;
   }
   mainwindows.remove(index);
   emit windowsChanged();
}

/**
 * @brief TexGenApplication::windowUpdated
 *
 * Slot invoked when a program has updated its title.
 * Will emit windowsChanged to all the windows, telling them to update their window lists.
 */
void TexGenApplication::windowUpdated(QString)
{
   emit windowsChanged();
}

/**
 * @brief TexGenApplication::event
 * Used to handle files being dropped on the application icon in OS X.
 */
bool TexGenApplication::event(QEvent* event) {
   if (event->type() == QEvent::FileOpen) {
      MainWindow* projectWindow = nullptr;
      if (!mainwindows.isEmpty()) {
         projectWindow = mainwindows.last();
      }
      if (mainwindows.length() != 1 ||
          (projectWindow && projectWindow->getTextureProject() &&
           projectWindow->getTextureProject()->isModified())) {
         // There are either multiple windows opened or the only one open
         // already contains data. Create a new fresh window for this project file.
         projectWindow = this->addWindow();
      }
      if (projectWindow != nullptr) {
         projectWindow->openFile(static_cast<QFileOpenEvent*>(event)->file());
      }
      return true;
   }
   return QApplication::event(event);
}
