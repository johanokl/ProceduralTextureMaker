/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef ADDNODEPANEL_H
#define ADDNODEPANEL_H

#include "generators/texturegenerator.h"
#include <QMap>
#include <QWidget>

class TextureProject;
class QGridLayout;
class QVBoxLayout;
class QGroupBox;
class QScrollArea;

/**
 * @brief The AddNodePanel class
 *
 * Vertical panel that displays all available node types.
 * All generators are represented as buttons that can be dragged to
 * the scene to add a new node.
 * The button list is updated automatically when new generators are
 * added to the associated project.
 */
class AddNodePanel : public QWidget
{
   Q_OBJECT

public:
   explicit AddNodePanel(TextureProject*);
   ~AddNodePanel() override = default;

public slots:
   void addGenerator(const TextureGeneratorPtr& generator);
   void removeGenerator(const TextureGeneratorPtr& generator);

private:
   QMap<TextureGeneratorPtr, QWidget*> widgets;
   TextureProject* project;
   QGroupBox* combinersWidget;
   QGroupBox* filtersWidget;
   QGroupBox* generatorsWidget;
   QGridLayout* filtersLayout;
   QGridLayout* combinersLayout;
   QGridLayout* generatorsLayout;
};


#endif // ADDNODEPANEL_H
