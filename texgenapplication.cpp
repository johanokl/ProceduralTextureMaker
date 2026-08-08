
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "gui/mainwindow.h"
#include "texgenapplication.h"
#include <QFileOpenEvent>

TexGenApplication::TexGenApplication(int& argc, char* argv[]) : QApplication(argc, argv) {}

void TexGenApplication::quitApplication() {
   for (int i = mainwindows.size() - 1; i >= 0; --i) {
      if (mainwindows.at(i)->close() == false) {
         return;
      }
   }
}

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

MainWindow* TexGenApplication::openProject(MainWindow* currentWindow, const QString& fileName) {
   if (fileName.isNull()) {
      return nullptr;
   }
   MainWindow* projectWindow = currentWindow;
   if (projectWindow == nullptr || projectWindow->getTextureProject() == nullptr ||
       projectWindow->getTextureProject()->isModified() ||
       projectWindow->getTextureProject()->getNumNodes() != 0) {
      projectWindow = addWindow();
   }
   projectWindow->openFile(fileName);
   return projectWindow;
}

void TexGenApplication::removeWindow(MainWindow* window) {
   int index = mainwindows.indexOf(window);
   if (index == -1) {
      return;
   }
   mainwindows.remove(index);
   emit windowsChanged();
}

void TexGenApplication::windowUpdated(QString) { emit windowsChanged(); }

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
