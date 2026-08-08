
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef TEXGENAPPLICATION_H
#define TEXGENAPPLICATION_H

#include <QApplication>
#include <QVector>
class QMenu;
class MainWindow;
class QString;

/// @brief Manages application windows and application-level file-open events.
/// @details Owns each MainWindow and keeps their window menus synchronized. The
/// application exits when its last visible MainWindow is closed.
class TexGenApplication : public QApplication {
   Q_OBJECT

public:
   /// @brief Constructs the application from command-line arguments.
   /// @param argc The number of arguments, including the application name.
   /// @param argv The arguments as C strings.
   TexGenApplication(int& argc, char* argv[]);

   /// @brief Destroys the application and its child windows.
   virtual ~TexGenApplication() {}

signals:
   /// @brief Emitted when the set of windows or a window title changes.
   void windowsChanged();

public slots:
   /// @brief Creates, tracks, and displays an empty main window.
   /// @return The newly created window.
   MainWindow* addWindow();

   /// @brief Loads a project, reusing only an empty window without unsaved changes.
   /// @param currentWindow The window requesting the load, or null if none exists.
   /// @param fileName The project file to load.
   /// @return The window used for the project, or null if no file name was supplied.
   MainWindow* openProject(MainWindow* currentWindow, const QString& fileName);

   /// @brief Closes every window, stopping if a window refuses to close.
   void quitApplication();

   /// @brief Notifies all windows that their window menus may need updating.
   /// @param title The updated window title; the value itself is not used.
   void windowUpdated(QString title);

protected:
   /// @brief Handles application events, including requests to open project files.
   /// @param event The event delivered to the application.
   /// @return @c true if the event was handled; otherwise the base-class result.
   bool event(QEvent* event);

private:
   /// @brief Removes a window from the tracked window list.
   /// @param window The window to remove.
   void removeWindow(MainWindow* window);

public:
   /// @brief Main windows currently tracked by the application.
   QVector<MainWindow*> mainwindows;
};

#endif  // TEXGENAPPLICATION_H
