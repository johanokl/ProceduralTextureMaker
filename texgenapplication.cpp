
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QFileOpenEvent>

/// @brief TexGenApplication constructor.
/// @param argc Number of arguments passed (including the application name).
/// @param argv Array of arguments as C char strings.
TexGenApplication::TexGenApplication(int& argc, char* argv[]) : QApplication(argc, argv) {}

/// @brief Tries to close all windows. Stops if any window refuses to close (e.g. if the user
/// cancels a save dialog).
void TexGenApplication::quitApplication() {
   for (int i = mainwindows.size() - 1; i >= 0; --i) {
      if (mainwindows.at(i)->close() == false) {
         return;
      }
   }
}

/// @brief Adds a new MainWindow.
/// @return A pointer to the newly created MainWindow.
MainWindow* TexGenApplication::addWindow() {
   auto* newWindow = new MainWindow(this);
   QObject::connect(newWindow, &MainWindow::windowTitleChanged, this,
                    &TexGenApplication::windowUpdated);
   QObject::connect(newWindow, &MainWindow::destroyed, this,
                    [this, newWindow]() { removeWindow(newWindow); });
   mainwindows.push_back(newWindow);
   newWindow->show();
   emit windowsChanged();
   return newWindow;
}

/// @brief Removes a MainWindows from the window list.
/// @param window The window to be removed.
void TexGenApplication::removeWindow(MainWindow* window) {
   int index = mainwindows.indexOf(window);
   if (index == -1) {
      return;
   }
   mainwindows.remove(index);
   emit windowsChanged();
}

/// @brief Slot invoked when a program has updated its title.
/// @details Emits windowsChanged to all the windows, telling them to update their window lists.
void TexGenApplication::windowUpdated(QString) { emit windowsChanged(); }

/// @brief Used to handle files being dropped on the application icon in OS X.
/// @param event The file open event.
/// @return @c true if the event was handled, @c false if not.
bool TexGenApplication::event(QEvent* event) {
   if (event->type() == QEvent::FileOpen) {
      MainWindow* projectWindow = nullptr;
      if (!mainwindows.isEmpty()) {
         projectWindow = mainwindows.last();
      }
      if (mainwindows.length() != 1 || (projectWindow && projectWindow->getTextureProject() &&
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
