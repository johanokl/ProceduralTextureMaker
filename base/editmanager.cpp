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

/// @brief One historical link, restored only if it still fits the graph.
struct ConnectionSnapshot {
   int sourceId{0};
   int receiverId{0};
   QString slot;
};

/// @brief Node values and schema without retaining an old compiled definition.
struct NodeSnapshot {
   int id{0};
   TextureGeneratorReferencePtr generator;
   TextureGeneratorSettings schema;
   QString name;
   QString identifier;
   QPointF position;
   TextureNodeSettings settings;
   QMap<QString, int> sources;
   QList<ConnectionSnapshot> outgoing;
};

NodeSnapshot captureNode(TextureProject& project, const int nodeId) {
   NodeSnapshot snapshot;
   const TextureNodePtr node = project.getNode(nodeId);
   if (node.isNull()) {
      return snapshot;
   }
   snapshot.id = nodeId;
   snapshot.generator = project.generatorReference(node->getGenerator());
   snapshot.schema = node->getGenerator()->getSettings();
   snapshot.name = node->getName();
   snapshot.identifier = node->getIdentifier();
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
      const QMap<QString, int> sources = receiver->getSources();
      for (auto source = sources.cbegin(); source != sources.cend(); ++source) {
         if (source.value() == nodeId) {
            snapshot.outgoing.append({nodeId, receiverId, source.key()});
         }
      }
   }
   return snapshot;
}

/// @brief Explains a partial replay through the normal status surface.
void reportAdjustedHistory(TextureProject& project) {
   emit project.historyReplayAdjusted(QStringLiteral(
       "Some historical settings or connections no longer fit the current generator."));
}

/// @brief Retains valid links and leaves the current link unchanged when replay is rejected.
void restoreConnection(TextureProject& project, int receiverId, const QString& slot, int sourceId) {
   const auto receiver = project.getNode(receiverId);
   if (!receiver || !receiver->setSourceSlot(slot, sourceId)) reportAdjustedHistory(project);
}

void restoreNodeState(TextureProject& project, const NodeSnapshot& snapshot) {
   if (snapshot.id == 0 || !project.getNode(snapshot.id).isNull()) {
      return;
   }
   if (!snapshot.generator->available) {
      emit project.historyReplayAdjusted(
          QStringLiteral("Cannot restore a node: its generator is unavailable."));
      return;
   }
   const auto generator = snapshot.generator->current;
   const auto settings =
       migrateTextureSettings(snapshot.schema, snapshot.settings, generator->getSettings());
   const TextureNodePtr node = project.newNode(snapshot.id, generator);
   node->setName(snapshot.name);
   node->setIdentifier(snapshot.identifier);
   node->setPos(snapshot.position);
   node->setSettings(settings);
   if (settings != snapshot.settings) reportAdjustedHistory(project);
}

void restoreConnections(TextureProject& project, const NodeSnapshot& snapshot) {
   const TextureNodePtr node = project.getNode(snapshot.id);
   if (node.isNull()) {
      return;
   }
   for (auto source = snapshot.sources.cbegin(); source != snapshot.sources.cend(); ++source) {
      if (source.value() != 0) {
         restoreConnection(project, snapshot.id, source.key(), source.value());
      }
   }
   for (const ConnectionSnapshot& connection : snapshot.outgoing) {
      restoreConnection(project, connection.receiverId, connection.slot, connection.sourceId);
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
                        QVariant oldValue, QVariant newValue, const bool merge, quint64 generation)
       : project(project),
         nodeId(nodeId),
         settingId(std::move(settingId)),
         oldValue(std::move(oldValue)),
         newValue(std::move(newValue)),
         merge(merge),
         generation(generation) {
      const auto definition = project.getNode(nodeId)->getGenerator();
      schema = *findTextureGeneratorSetting(definition->getSettings(), this->settingId);
      setText(QStringLiteral("Change %1").arg(this->settingId));
   }
   int id() const override { return merge ? 0x50544d01 : -1; }
   bool mergeWith(const QUndoCommand* other) override {
      const auto* command = dynamic_cast<const ChangeSettingCommand*>(other);
      if (!command || !merge || !command->merge || &project != &command->project ||
          nodeId != command->nodeId || settingId != command->settingId ||
          generation != command->generation ||
          schema.defaultvalue.typeId() != command->schema.defaultvalue.typeId() ||
          schema.multiline != command->schema.multiline) {
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
      const auto definition = node->getGenerator();
      const auto* current = findTextureGeneratorSetting(definition->getSettings(), settingId);
      // Setting identity and type decide compatibility, even across generator definitions.
      const QVariant migrated =
          current ? migrateTextureSetting(schema, *current, value) : QVariant();
      if (!migrated.isValid()) {
         reportAdjustedHistory(project);
         return;
      }
      TextureNodeSettings settings = node->getSettings();
      settings[settingId] = migrated;
      node->setSettings(settings);
      if (migrated != value) reportAdjustedHistory(project);
   }
   TextureProject& project;
   int nodeId;
   QString settingId;
   QVariant oldValue;
   QVariant newValue;
   bool merge;
   quint64 generation;
   TextureGeneratorSetting schema;
};

class RenameNodeCommand final : public QUndoCommand {
public:
   RenameNodeCommand(TextureProject& project, const int nodeId, QString oldName, QString newName,
                     bool identifier = false)
       : project(project),
         nodeId(nodeId),
         oldName(std::move(oldName)),
         newName(std::move(newName)),
         identifier(identifier) {
      setText(identifier ? QStringLiteral("Change node ID") : QStringLiteral("Rename node"));
   }
   void undo() override { apply(oldName); }
   void redo() override { apply(newName); }

private:
   void apply(const QString& name) {
      const TextureNodePtr node = project.getNode(nodeId);
      if (!node.isNull()) {
         if (identifier) {
            node->setIdentifier(name);
         } else {
            node->setName(name);
         }
      }
   }
   TextureProject& project;
   int nodeId;
   QString oldName;
   QString newName;
   bool identifier;
};

class SetConnectionCommand final : public QUndoCommand {
public:
   SetConnectionCommand(TextureProject& project, const int receiverId, QString slot,
                        const int oldSourceId, const int newSourceId)
       : project(project),
         receiverId(receiverId),
         slot(std::move(slot)),
         oldSourceId(oldSourceId),
         newSourceId(newSourceId) {
      setText(newSourceId == 0 ? QStringLiteral("Remove connection")
                               : QStringLiteral("Add connection"));
   }
   void undo() override { apply(oldSourceId); }
   void redo() override { apply(newSourceId); }

private:
   void apply(const int sourceId) { restoreConnection(project, receiverId, slot, sourceId); }
   TextureProject& project;
   int receiverId;
   QString slot;
   int oldSourceId;
   int newSourceId;
};

class SetSourcesCommand final : public QUndoCommand {
public:
   SetSourcesCommand(TextureProject& project, const int receiverId, QMap<QString, int> oldSources,
                     QMap<QString, int> newSources)
       : project(project),
         receiverId(receiverId),
         oldSources(std::move(oldSources)),
         newSources(std::move(newSources)) {
      setText(QStringLiteral("Swap connections"));
   }
   void undo() override { apply(oldSources); }
   void redo() override { apply(newSources); }

private:
   void apply(const QMap<QString, int>& sources) {
      const TextureNodePtr receiver = project.getNode(receiverId);
      if (receiver.isNull()) {
         return;
      }
      // Touch only historical inputs; new inputs keep their current connections.
      for (auto source = sources.cbegin(); source != sources.cend(); ++source) {
         restoreConnection(project, receiverId, source.key(), source.value());
      }
   }
   TextureProject& project;
   int receiverId;
   QMap<QString, int> oldSources;
   QMap<QString, int> newSources;
};

class AddNodeCommand final : public QUndoCommand {
public:
   AddNodeCommand(TextureProject& project, TextureGeneratorPtr generator, QPointF position)
       : project(project), generator(project.generatorReference(generator)), position(position) {
      setText(QStringLiteral("Add node"));
   }
   int nodeId() const { return snapshot.id; }
   void undo() override { project.removeNode(snapshot.id); }
   void redo() override {
      if (snapshot.id != 0) {
         restoreNode(project, snapshot);
         return;
      }
      const TextureNodePtr node = project.newNode(0, generator->current);
      node->setPos(position);
      snapshot = captureNode(project, node->getId());
      setText(QStringLiteral("Add %1").arg(node->getName()));
   }

private:
   TextureProject& project;
   TextureGeneratorReferencePtr generator;
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
   QObject::connect(
       &project, &TextureProject::generatorReplaced, &undoStack, [this](bool affectsNodes) {
          ++mergeGeneration;
          const bool dirty = this->project.isModified() || affectsNodes || undoStack.count() > 0;
          if (dirty) undoStack.resetClean();
          syncModifiedState();
       });
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
   if (node.isNull() ||
       !findTextureGeneratorSetting(node->getGenerator()->getSettings(), settingId)) {
      return;
   }
   const QVariant oldValue = node->getSettings().value(settingId);
   if (oldValue == newValue) {
      return;
   }
   undoStack.push(new ChangeSettingCommand(project, nodeId, settingId, oldValue, newValue, merge,
                                           mergeGeneration));
}

void EditManager::renameNode(const int nodeId, const QString& newName) {
   const TextureNodePtr node = project.getNode(nodeId);
   if (!node.isNull() && node->getName() != newName) {
      undoStack.push(new RenameNodeCommand(project, nodeId, node->getName(), newName));
   }
}

bool EditManager::changeNodeIdentifier(int nodeId, const QString& value) {
   const QString trimmed = value.trimmed();
   const auto node = project.getNode(nodeId);
   if (!node || !TextureNode::isValidIdentifier(trimmed)) {
      return false;
   }
   const auto existing = project.getNode(trimmed);
   if (existing && existing != node) {
      return false;
   }
   if (node->getIdentifier() != trimmed) {
      undoStack.push(new RenameNodeCommand(project, nodeId, node->getIdentifier(), trimmed, true));
   }
   return true;
}

bool EditManager::setConnection(const int receiverId, const QString& slot, const int sourceId) {
   const TextureNodePtr receiver = project.getNode(receiverId);
   if (receiver.isNull()) {
      return false;
   }
   if (!receiver->getSourceSlots().contains(slot)) {
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

bool EditManager::setConnectionToFirstAvailable(const int receiverId, const int sourceId) {
   const TextureNodePtr receiver = project.getNode(receiverId);
   if (receiver.isNull()) {
      return false;
   }
   const QString slot = receiver->getFirstAvailableSourceSlot();
   return !slot.isNull() && setConnection(receiverId, slot, sourceId);
}

void EditManager::swapSources(const int receiverId) {
   const TextureNodePtr receiver = project.getNode(receiverId);
   if (receiver.isNull() || receiver->getSourceSlots().size() < 2) {
      return;
   }
   const QStringList sourceSlots = receiver->getSourceSlots();
   const QMap<QString, int> oldSources = receiver->getSources();
   QMap<QString, int> newSources;
   for (int i = 0; i < sourceSlots.size(); ++i) {
      newSources[sourceSlots.at(i)] =
          oldSources.value(sourceSlots.at((i + 1) % sourceSlots.size()));
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

int EditManager::pasteNodes(const QString& json) {
   const QList<int> oldNodeIds = project.getNodeIds();
   const QSet<int> before(oldNodeIds.cbegin(), oldNodeIds.cend());
   const int count = project.pasteNodes(json);
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
