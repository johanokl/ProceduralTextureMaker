/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef ADDNODEPANEL_H
#define ADDNODEPANEL_H

#include <QWidget>
#include <QMap>

class TextureProject;
class QGridLayout;
class QVBoxLayout;
class QGroupBox;
class QScrollArea;
class TextureGenerator;

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
   AddNodePanel(TextureProject*);
   virtual ~AddNodePanel() {}

public slots:
   void addGenerator(TextureGenerator*);
   void removeGenerator(TextureGenerator*);

private:
   QMap<TextureGenerator*, QWidget*> widgets;
   TextureProject* project;
   QGroupBox* combinersWidget;
   QGroupBox* filtersWidget;
   QGroupBox* generatorsWidget;
   QGridLayout* filtersLayout;
   QGridLayout* combinersLayout;
   QGridLayout* generatorsLayout;
};


#endif // ADDNODEPANEL_H
