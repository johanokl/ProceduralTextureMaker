/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef TEXGENAPPLICATION_H
#define TEXGENAPPLICATION_H

#include <QApplication>
#include <QVector>

class QMenu;
class MainWindow;

/**
 * @brief The TexGenApplication class
 * Manages the instances of class MainWindow.
 *
 * All instances of MainWindow has this as their parent object, and all windows' menu
 * bars contain links to all the other instances.
 *
 * Inherits QApplication and adds some convenient application specifc functions.
 * Will be destroyed (and the application process exited) when the
 * last visible MainWindow is closed.

 */
class TexGenApplication : public QApplication
{
   Q_OBJECT

public:
   TexGenApplication(int &argc, char* argv[]);
   virtual ~TexGenApplication() {}

signals:
   void windowsChanged();

public slots:
   MainWindow* addWindow();
   void removeWindow(QObject*);
   void quit();
   void windowUpdated(QString);

protected:
   bool event(QEvent* event);

public:
   QVector<MainWindow*> mainwindows;

};


#endif // TEXGENAPPLICATION_H
