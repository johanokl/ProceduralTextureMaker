// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.

#include "editmanager.h"
#include "base/texturenode.h"
#include "base/textureproject.h"
#include <QList>
#include <QSet>
#include <QUndoCommand>
#include <utility>

namespace {

struct ConnectionSnapshot {
   int sourceId{0};
   int receiverId{0};
   int slot{0};
};

struct NodeSnapshot {
   int id{0};
   TextureGeneratorPtr generator;
   QString name;
   QPointF position;
   TextureNodeSettings settings;
   QMap<int, int> sources;
   QList<ConnectionSnapshot> outgoing;
};

NodeSnapshot captureNode(TextureProject& project, const int nodeId) {
   NodeSnapshot snapshot;
   const TextureNodePtr node = project.getNode(nodeId);
   if (node.isNull()) {
      return snapshot;
   }
   snapshot.id = nodeId;
   snapshot.generator = node->getGenerator();
   snapshot.name = node->getName();
   snapshot.position = node->getPos();
   snapshot.settings = node->getSettings();
   snapshot.sources = node->getSources();
   for (const int receiverId : project.getNodeIds()) {
      if (receiverId == nodeId) {
         continue;
      }
      const TextureNodePtr receiver = project.getNode(receiverId);
      if (receiver.isNull()) {
         continue;
      }
      const QMap<int, int> sources = receiver->getSources();
      for (auto source = sources.cbegin(); source != sources.cend(); ++source) {
         if (source.value() == nodeId) {
            snapshot.outgoing.append({nodeId, receiverId, source.key()});
         }
      }
   }
   return snapshot;
}

void restoreNodeState(TextureProject& project, const NodeSnapshot& snapshot) {
   if (snapshot.id == 0 || !project.getNode(snapshot.id).isNull()) {
      return;
   }
   const TextureNodePtr node = project.newNode(snapshot.id, snapshot.generator);
   node->setName(snapshot.name);
   node->setPos(snapshot.position);
   node->setSettings(snapshot.settings);
}

void restoreConnections(TextureProject& project, const NodeSnapshot& snapshot) {
   const TextureNodePtr node = project.getNode(snapshot.id);
   if (node.isNull()) {
      return;
   }
   for (auto source = snapshot.sources.cbegin(); source != snapshot.sources.cend(); ++source) {
      if (source.value() != 0) {
         node->setSourceSlot(source.key(), source.value());
      }
   }
   for (const ConnectionSnapshot& connection : snapshot.outgoing) {
      const TextureNodePtr receiver = project.getNode(connection.receiverId);
      if (!receiver.isNull()) {
         receiver->setSourceSlot(connection.slot, connection.sourceId);
      }
   }
}

void restoreNode(TextureProject& project, const NodeSnapshot& snapshot) {
   restoreNodeState(project, snapshot);
   restoreConnections(project, snapshot);
}

void restoreNodes(TextureProject& project, const QList<NodeSnapshot>& snapshots) {
   for (const NodeSnapshot& snapshot : snapshots) {
      restoreNodeState(project, snapshot);
   }
   for (const NodeSnapshot& snapshot : snapshots) {
      restoreConnections(project, snapshot);
   }
}

class MoveNodeCommand final : public QUndoCommand {
public:
   MoveNodeCommand(TextureProject& project, const int nodeId, QPointF oldPosition,
                   QPointF newPosition)
       : project(project), nodeId(nodeId), oldPosition(oldPosition), newPosition(newPosition) {
      setText(QStringLiteral("Move node"));
   }
   void undo() override { apply(oldPosition); }
   void redo() override { apply(newPosition); }

private:
   void apply(const QPointF& position) {
      const TextureNodePtr node = project.getNode(nodeId);
      if (!node.isNull()) {
         node->setPos(position);
      }
   }
   TextureProject& project;
   int nodeId;
   QPointF oldPosition;
   QPointF newPosition;
};

class ChangeSettingCommand final : public QUndoCommand {
public:
   ChangeSettingCommand(TextureProject& project, const int nodeId, QString settingId,
                        QVariant oldValue, QVariant newValue, const bool merge)
       : project(project),
         nodeId(nodeId),
         settingId(std::move(settingId)),
         oldValue(std::move(oldValue)),
         newValue(std::move(newValue)),
         merge(merge) {
      setText(QStringLiteral("Change %1").arg(this->settingId));
   }
   int id() const override { return merge ? 0x50544d01 : -1; }
   bool mergeWith(const QUndoCommand* other) override {
      const auto* command = dynamic_cast<const ChangeSettingCommand*>(other);
      if (!command || !merge || !command->merge || &project != &command->project ||
          nodeId != command->nodeId || settingId != command->settingId) {
         return false;
      }
      newValue = command->newValue;
      return true;
   }
   void undo() override { apply(oldValue); }
   void redo() override { apply(newValue); }

private:
   void apply(const QVariant& value) {
      const TextureNodePtr node = project.getNode(nodeId);
      if (node.isNull()) {
         return;
      }
      TextureNodeSettings settings = node->getSettings();
      settings[settingId] = value;
      node->setSettings(settings);
   }
   TextureProject& project;
   int nodeId;
   QString settingId;
   QVariant oldValue;
   QVariant newValue;
   bool merge;
};

class RenameNodeCommand final : public QUndoCommand {
public:
   RenameNodeCommand(TextureProject& project, const int nodeId, QString oldName, QString newName)
       : project(project),
         nodeId(nodeId),
         oldName(std::move(oldName)),
         newName(std::move(newName)) {
      setText(QStringLiteral("Rename node"));
   }
   void undo() override { apply(oldName); }
   void redo() override { apply(newName); }

private:
   void apply(const QString& name) {
      const TextureNodePtr node = project.getNode(nodeId);
      if (!node.isNull()) {
         node->setName(name);
      }
   }
   TextureProject& project;
   int nodeId;
   QString oldName;
   QString newName;
};

class SetConnectionCommand final : public QUndoCommand {
public:
   SetConnectionCommand(TextureProject& project, const int receiverId, const int slot,
                        const int oldSourceId, const int newSourceId)
       : project(project),
         receiverId(receiverId),
         slot(slot),
         oldSourceId(oldSourceId),
         newSourceId(newSourceId) {
      setText(newSourceId == 0 ? QStringLiteral("Remove connection")
                               : QStringLiteral("Add connection"));
   }
   void undo() override { apply(oldSourceId); }
   void redo() override { apply(newSourceId); }

private:
   void apply(const int sourceId) {
      const TextureNodePtr receiver = project.getNode(receiverId);
      if (!receiver.isNull()) {
         receiver->setSourceSlot(slot, sourceId);
      }
   }
   TextureProject& project;
   int receiverId;
   int slot;
   int oldSourceId;
   int newSourceId;
};

class SetSourcesCommand final : public QUndoCommand {
public:
   SetSourcesCommand(TextureProject& project, const int receiverId, QMap<int, int> oldSources,
                     QMap<int, int> newSources)
       : project(project),
         receiverId(receiverId),
         oldSources(std::move(oldSources)),
         newSources(std::move(newSources)) {
      setText(QStringLiteral("Swap connections"));
   }
   void undo() override { apply(oldSources); }
   void redo() override { apply(newSources); }

private:
   void apply(const QMap<int, int>& sources) {
      const TextureNodePtr receiver = project.getNode(receiverId);
      if (receiver.isNull()) {
         return;
      }
      for (int slot = 0; slot < receiver->getNumSourceSlots(); ++slot) {
         receiver->setSourceSlot(slot, 0);
      }
      for (auto source = sources.cbegin(); source != sources.cend(); ++source) {
         if (source.value() != 0) {
            receiver->setSourceSlot(source.key(), source.value());
         }
      }
   }
   TextureProject& project;
   int receiverId;
   QMap<int, int> oldSources;
   QMap<int, int> newSources;
};

class AddNodeCommand final : public QUndoCommand {
public:
   AddNodeCommand(TextureProject& project, TextureGeneratorPtr generator, QPointF position)
       : project(project), generator(std::move(generator)), position(position) {
      setText(QStringLiteral("Add node"));
   }
   int nodeId() const { return snapshot.id; }
   void undo() override { project.removeNode(snapshot.id); }
   void redo() override {
      if (snapshot.id != 0) {
         restoreNode(project, snapshot);
         return;
      }
      const TextureNodePtr node = project.newNode(0, generator);
      node->setPos(position);
      snapshot = captureNode(project, node->getId());
      setText(QStringLiteral("Add %1").arg(node->getName()));
   }

private:
   TextureProject& project;
   TextureGeneratorPtr generator;
   QPointF position;
   NodeSnapshot snapshot;
};

class RemoveNodeCommand final : public QUndoCommand {
public:
   RemoveNodeCommand(TextureProject& project, const int nodeId)
       : project(project), snapshot(captureNode(project, nodeId)) {
      setText(QStringLiteral("Remove %1").arg(snapshot.name));
   }
   void undo() override { restoreNode(project, snapshot); }
   void redo() override { project.removeNode(snapshot.id); }

private:
   TextureProject& project;
   NodeSnapshot snapshot;
};

class AddNodesCommand final : public QUndoCommand {
public:
   AddNodesCommand(TextureProject& project, QList<NodeSnapshot> snapshots)
       : project(project), snapshots(std::move(snapshots)) {
      setText(this->snapshots.size() == 1 ? QStringLiteral("Paste node")
                                          : QStringLiteral("Paste nodes"));
   }
   void undo() override {
      for (auto snapshot = snapshots.crbegin(); snapshot != snapshots.crend(); ++snapshot) {
         project.removeNode(snapshot->id);
      }
   }
   void redo() override { restoreNodes(project, snapshots); }

private:
   TextureProject& project;
   QList<NodeSnapshot> snapshots;
};

class ClearProjectCommand final : public QUndoCommand {
public:
   ClearProjectCommand(TextureProject& project, QList<NodeSnapshot> snapshots)
       : project(project), snapshots(std::move(snapshots)) {
      setText(QStringLiteral("Clear scene"));
   }
   void undo() override { restoreNodes(project, snapshots); }
   void redo() override { project.clear(); }

private:
   TextureProject& project;
   QList<NodeSnapshot> snapshots;
};

}  // namespace

EditManager::EditManager(TextureProject& project) : project(project) {
   QObject::connect(&undoStack, &QUndoStack::cleanChanged, [this](bool) { syncModifiedState(); });
   QObject::connect(&undoStack, &QUndoStack::indexChanged, [this](int) { syncModifiedState(); });
   syncModifiedState();
}

void EditManager::reset() {
   undoStack.clear();
   undoStack.setClean();
   syncModifiedState();
}

void EditManager::setClean() {
   undoStack.setClean();
   syncModifiedState();
}

void EditManager::syncModifiedState() { project.modified = !undoStack.isClean(); }

void EditManager::moveNode(const int nodeId, const QPointF& oldPosition,
                           const QPointF& newPosition) {
   if (oldPosition != newPosition && !project.getNode(nodeId).isNull()) {
      undoStack.push(new MoveNodeCommand(project, nodeId, oldPosition, newPosition));
   }
}

void EditManager::changeSetting(const int nodeId, const QString& settingId,
                                const QVariant& newValue, const bool merge) {
   const TextureNodePtr node = project.getNode(nodeId);
   if (node.isNull()) {
      return;
   }
   const QVariant oldValue = node->getSettings().value(settingId);
   if (oldValue == newValue) {
      return;
   }
   undoStack.push(new ChangeSettingCommand(project, nodeId, settingId, oldValue, newValue, merge));
}

void EditManager::renameNode(const int nodeId, const QString& newName) {
   const TextureNodePtr node = project.getNode(nodeId);
   if (!node.isNull() && node->getName() != newName) {
      undoStack.push(new RenameNodeCommand(project, nodeId, node->getName(), newName));
   }
}

bool EditManager::setConnection(const int receiverId, int slot, const int sourceId) {
   const TextureNodePtr receiver = project.getNode(receiverId);
   if (receiver.isNull()) {
      return false;
   }
   if (slot == -1) {
      for (int candidate = 0; candidate < receiver->getNumSourceSlots(); ++candidate) {
         if (receiver->getSources().value(candidate) == 0) {
            slot = candidate;
            break;
         }
      }
   }
   if (slot < 0 || slot >= receiver->getNumSourceSlots()) {
      return false;
   }
   const int oldSourceId = receiver->getSources().value(slot);
   if (oldSourceId == sourceId) {
      return true;
   }
   if (!receiver->setSourceSlot(slot, sourceId)) {
      return false;
   }
   undoStack.push(new SetConnectionCommand(project, receiverId, slot, oldSourceId, sourceId));
   return true;
}

void EditManager::swapSources(const int receiverId) {
   const TextureNodePtr receiver = project.getNode(receiverId);
   if (receiver.isNull() || receiver->getNumSourceSlots() < 2) {
      return;
   }
   const QMap<int, int> oldSources = receiver->getSources();
   QMap<int, int> newSources;
   for (int slot = 0; slot < receiver->getNumSourceSlots(); ++slot) {
      newSources[slot] = oldSources.value((slot + 1) % receiver->getNumSourceSlots());
   }
   if (oldSources != newSources) {
      undoStack.push(new SetSourcesCommand(project, receiverId, oldSources, newSources));
   }
}

int EditManager::addNode(const TextureGeneratorPtr& generator, const QPointF& position) {
   if (generator.isNull()) {
      return 0;
   }
   auto* command = new AddNodeCommand(project, generator, position);
   undoStack.push(command);
   return command->nodeId();
}

bool EditManager::removeNode(const int nodeId) {
   if (project.getNode(nodeId).isNull()) {
      return false;
   }
   undoStack.push(new RemoveNodeCommand(project, nodeId));
   return true;
}

bool EditManager::clearProject() {
   const QList<int> nodeIds = project.getNodeIds();
   if (nodeIds.isEmpty()) {
      return false;
   }
   QList<NodeSnapshot> snapshots;
   snapshots.reserve(nodeIds.size());
   for (const int nodeId : nodeIds) {
      snapshots.append(captureNode(project, nodeId));
   }
   undoStack.push(new ClearProjectCommand(project, snapshots));
   return true;
}

int EditManager::pasteNodes(const QString& xml) {
   const QList<int> oldNodeIds = project.getNodeIds();
   const QSet<int> before(oldNodeIds.cbegin(), oldNodeIds.cend());
   const int count = project.pasteNodes(xml);
   if (count == 0) {
      return 0;
   }
   QList<NodeSnapshot> snapshots;
   for (const int nodeId : project.getNodeIds()) {
      if (!before.contains(nodeId)) {
         snapshots.append(captureNode(project, nodeId));
      }
   }
   undoStack.push(new AddNodesCommand(project, snapshots));
   return snapshots.size();
}
