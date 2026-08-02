// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.

#ifndef EDITMANAGER_H
#define EDITMANAGER_H

#include "generators/texturegenerator.h"
#include <QMap>
#include <QPointF>
#include <QString>
#include <QUndoStack>
#include <QVariant>

class TextureProject;

/// @brief Applies user-initiated project edits and owns their undo history.
class EditManager {
public:
   /// @brief Creates an edit manager for a project.
   explicit EditManager(TextureProject& project);

   /// @brief Returns the Qt undo stack used by menus and tests.
   QUndoStack& stack() { return undoStack; }

   /// @brief Returns the Qt undo stack used by menus and tests.
   const QUndoStack& stack() const { return undoStack; }

   /// @brief Clears all commands and establishes the current graph as the saved state.
   void reset();

   /// @brief Marks the current command index as saved.
   void setClean();

   /// @brief Returns whether the command index is at the saved state.
   bool isClean() const { return undoStack.isClean(); }

   /// @brief Records a completed node drag as one command.
   void moveNode(int nodeId, const QPointF& oldPosition, const QPointF& newPosition);

   /// @brief Changes one generator setting, optionally merging consecutive changes.
   void changeSetting(int nodeId, const QString& settingId, const QVariant& newValue,
                      bool merge = true);

   /// @brief Changes a node's display name.
   void renameNode(int nodeId, const QString& newName);

   /// @brief Connects or disconnects a receiver slot.
   bool setConnection(int receiverId, const QString& slot, int sourceId);

   /// @brief Connects a source to the first available receiver slot.
   bool setConnectionToFirstAvailable(int receiverId, int sourceId);

   /// @brief Rotates a node's source-slot values as one command.
   void swapSources(int receiverId);

   /// @brief Adds a node and returns its allocated ID.
   int addNode(const TextureGeneratorPtr& generator, const QPointF& position);

   /// @brief Removes a node while retaining enough state to restore it.
   bool removeNode(int nodeId);

   /// @brief Removes every node as one undoable command.
   bool clearProject();

   /// @brief Pastes clipboard XML as one undoable operation.
   int pasteNodes(const QString& xml);

private:
   /// @brief Synchronizes the legacy project modified flag with the stack clean state.
   void syncModifiedState();

   TextureProject& project;
   QUndoStack undoStack;
};

#endif  // EDITMANAGER_H
