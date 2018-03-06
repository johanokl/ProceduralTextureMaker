/**
 * Part of the ProceduralTextureMaker project.
 * http://github.com/johanokl/ProceduralTextureMaker
 * Released under GPLv3.
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QPushButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSet>
#include <QApplication>
#include <QGroupBox>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QScrollArea>
#include <QLabel>
#include "global.h"
#include "core/textureproject.h"
#include "generators/texturegenerator.h"
#include "gui/addnodepanel.h"
#include "gui/nodesettingswidget.h"
#include "gui/connectionwidget.h"
#include "generators/texturegenerator.h"

/**
 * @brief The AddNodeButton class
 *
 * Extended QPushButton class with functions for enabling
 * drag and drop of generator names.
 */
class AddNodeButton : public QPushButton
{
public:
   AddNodeButton(QWidget* parent, QString generatorName) : QPushButton(parent) {
      this->generatorName = generatorName;
   }
   void mousePressEvent(QMouseEvent *event)
   {
      if (event->button() == Qt::LeftButton) {
         dragStartPosition = event->pos();
      }
   }
   void mouseMoveEvent(QMouseEvent *event)
   {
      if ((event->buttons() & Qt::LeftButton)
          && (event->pos() - dragStartPosition).manhattanLength()
          > QApplication::startDragDistance()) {
         QDrag *drag = new QDrag(this);
         QMimeData *mimeData = new QMimeData;
         mimeData->setText(generatorName);
         drag->setMimeData(mimeData);
         drag->exec(Qt::CopyAction | Qt::MoveAction);
      }
   }
private:
   QPoint dragStartPosition;
   QString generatorName;
};

/**
 * @brief AddNodePanel::AddNodePanel
 * @param project
 * Displays the texgen classes in three groups, Generator, Filter and Combiner.
 */
AddNodePanel::AddNodePanel(TextureProject* project)
{
   this->project = project;

   QVBoxLayout* layout = new QVBoxLayout(this);
   QScrollArea* area = new QScrollArea;
   area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   area->setWidgetResizable(true);
   QWidget* contents = new QWidget;
   QVBoxLayout* contentsLayout = new QVBoxLayout(contents);
   layout->setContentsMargins(0, 0, 0, 0);
   contentsLayout->setContentsMargins(0, 0, 0, 0);
   area->setFrameShape(QFrame::NoFrame);
   layout->addWidget(area);
   area->setWidget(contents);
   setFixedWidth(250);
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   setLayout(layout);

   generatorsWidget = new QGroupBox("Generators");
   generatorsLayout = new QGridLayout();
   generatorsLayout->setContentsMargins(0, 0, 0, 0);
   generatorsWidget->setLayout(generatorsLayout);
   generatorsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(generatorsWidget);

   filtersWidget = new QGroupBox("Filters");
   filtersLayout = new QGridLayout();
   filtersLayout->setContentsMargins(0, 0, 0, 0);
   filtersWidget->setLayout(filtersLayout);
   filtersWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(filtersWidget);

   combinersWidget = new QGroupBox("Combiners");
   combinersLayout = new QGridLayout();
   combinersLayout->setContentsMargins(0, 0, 0, 0);
   combinersWidget->setLayout(combinersLayout);
   combinersWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(combinersWidget);

   QWidget *spacerWidget = new QWidget;
   spacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
   spacerWidget->setVisible(true);
   contentsLayout->addWidget(spacerWidget);
   QObject::connect(project, &TextureProject::generatorAdded,
                    this, &AddNodePanel::addGenerator);
   QObject::connect(project, &TextureProject::generatorRemoved,
                    this, &AddNodePanel::removeGenerator);
}

/**
 * @brief AddNodePanel::removeGenerator
 * @param generator
 * Removes a button.
 */
void AddNodePanel::removeGenerator(TextureGeneratorPtr generator)
{
   if (widgets.contains(generator)) {
      delete widgets.value(generator);
   }
}

/**
 * @brief AddNodePanel::addGenerator
 * @param generator
 *
 * Slot called when a generator has been added. Creates a new draggable
 * button and adds it to the generator types's widget group.
 */
void AddNodePanel::addGenerator(TextureGeneratorPtr generator)
{
   if (widgets.contains(generator)) {
      return;
   }
   QString generatorName = generator->getName();
   QMapIterator<TextureGeneratorPtr, QWidget*> widgetsIterator(widgets);
   while (widgetsIterator.hasNext()) {
      if (widgetsIterator.next().key()->getName() == generatorName) {
         return;
      }
   }
   AddNodeButton* newButton = new AddNodeButton(this, generatorName);
   widgets.insert(generator, newButton);
   newButton->setFixedSize(100, 60);
   newButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
   int hash = qHash(generatorName);
   QColor buttonColor(((hash & 0xFF0000) >> 16), ((hash & 0x00FF00) >> 8), (hash & 0x0000FF));
   QString fontColor("#ffffff");
   if ((buttonColor.red() * 0.299 + buttonColor.green() * 0.587 + buttonColor.blue() * 0.114) > 170) {
      fontColor = "#000000";
   }
   newButton->setStyleSheet(QString("background-color: %1; color: %2")
                            .arg(buttonColor.name()).arg(fontColor));
   QGridLayout* destLayout;
   switch (generator->getType()) {
   case TextureGenerator::Type::Combiner:
      destLayout = combinersLayout;
      break;
   case TextureGenerator::Type::Filter:
      destLayout = filtersLayout;
      break;
   default:
      destLayout = generatorsLayout;
   }
   int numButtons = destLayout->count();
   int row = numButtons / 2;
   int column = numButtons % 2;
   newButton->setText(generatorName);
   destLayout->addWidget(newButton, row, column);
}
