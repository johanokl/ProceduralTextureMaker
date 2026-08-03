
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef ADDNODEPANEL_H
#define ADDNODEPANEL_H

#include "base/texturegenerator.h"
#include <QMap>
#include <QWidget>
class TextureProject;
class QGridLayout;
class QGroupBox;

/// @brief Displays all available node types in a vertical panel.
/// All generators are represented as buttons that can be dragged to
/// the scene to add a new node.
/// The button list is updated automatically when new generators are
/// added to the associated project.
class AddNodePanel : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates a panel populated with the project's registered generators.
   /// @param project Project whose generators are displayed.
   explicit AddNodePanel(TextureProject& project);

   /// @brief Destroys the panel and its generator buttons.
   ~AddNodePanel() override = default;

public slots:
   /// @brief Adds a draggable button for a newly registered generator.
   /// @param generator Generator represented by the new button.
   void addGenerator(const TextureGeneratorPtr& generator);

   /// @brief Removes the button associated with a generator.
   /// @param generator Generator whose button is removed.
   void removeGenerator(const TextureGeneratorPtr& generator);

private:
   QGridLayout* layoutFor(const TextureGeneratorPtr& generator) const;
   void updateCustomGroupVisibility();

   /// @brief Maps each registered generator to its draggable button.
   QMap<TextureGeneratorPtr, QWidget*> widgets;
   /// @brief Grid layout for filter generator buttons.
   QGridLayout* filtersLayout{nullptr};
   /// @brief Grid layout for combiner generator buttons.
   QGridLayout* combinersLayout{nullptr};
   /// @brief Grid layout for source generator buttons.
   QGridLayout* generatorsLayout{nullptr};
   QGridLayout* customFiltersLayout{nullptr};
   QGridLayout* customCombinersLayout{nullptr};
   QGridLayout* customGeneratorsLayout{nullptr};
   QGroupBox* customFiltersWidget{nullptr};
   QGroupBox* customCombinersWidget{nullptr};
   QGroupBox* customGeneratorsWidget{nullptr};
};

#endif  // ADDNODEPANEL_H
