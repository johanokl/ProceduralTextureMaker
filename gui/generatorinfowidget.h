// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#ifndef GENERATORINFOWIDGET_H
#define GENERATORINFOWIDGET_H

#include "base/texturegenerator.h"
#include <QWidget>

class QLabel;
class QLayout;
class QVBoxLayout;

/// @brief Displays metadata for a generator selected in the add-node panel.
class GeneratorInfoWidget : public QWidget {
   Q_OBJECT

public:
   /// @brief Creates an initially empty generator information widget.
   /// @param parent Parent widget.
   explicit GeneratorInfoWidget(QWidget* parent = nullptr);

   /// @brief Displays all user-facing metadata exposed by a generator.
   /// @param generator Generator whose metadata is displayed.
   void setGenerator(const TextureGeneratorPtr& generator);

   /// @brief Returns the generator currently displayed by the widget.
   /// @return Shared pointer to the displayed generator, or a null pointer when empty.
   TextureGeneratorPtr getGenerator() const { return generator; }

private:
   /// @brief Deletes all widgets and layout items from a dynamic section.
   /// @param layout Layout whose child items are removed.
   void clearLayout(QLayout* layout);

   /// @brief Generator definition currently displayed by the inspector.
   TextureGeneratorPtr generator;
   /// @brief Header displaying the generator's public name.
   QLabel* nameLabel{nullptr};
   /// @brief Label displaying the generator category.
   QLabel* typeLabel{nullptr};
   /// @brief Label displaying whether the generator is built in or custom.
   QLabel* originLabel{nullptr};
   /// @brief Label displaying the generator's user-facing description.
   QLabel* descriptionLabel{nullptr};
   /// @brief Auto-sizing label displaying the generator's source identity.
   QLabel* sourceLabel{nullptr};
   /// @brief Layout containing one label for each generator input slot.
   QVBoxLayout* inputsLayout{nullptr};
   /// @brief Layout containing the generator's configurable property descriptions.
   QVBoxLayout* settingsLayout{nullptr};
};

#endif  // GENERATORINFOWIDGET_H
