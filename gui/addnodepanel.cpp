
// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "base/textureproject.h"
#include "base/texturegenerator.h"
#include "global.h"
#include "gui/addnodepanel.h"
#include "gui/connectionwidget.h"
#include "gui/nodesettingswidget.h"
#include <QApplication>
#include <QButtonGroup>
#include <QDrag>
#include <QGroupBox>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QVBoxLayout>
#include <utility>

/// @brief Extended QPushButton class with functions for enabling drag and drop of generator names.
class AddNodeButton : public QPushButton {
public:
   /// @brief Creates a button that starts drags containing a generator name.
   /// @param parent Parent widget.
   /// @param generatorName Generator name placed in the drag data.
   AddNodeButton(QWidget* parent, QString generatorName)
       : QPushButton(parent), generatorName(std::move(generatorName)) {}

   /// @brief Destroys the draggable generator button.
   ~AddNodeButton() override;

   /// @brief Stores the starting position of a possible drag operation.
   /// @param event Mouse press event.
   void mousePressEvent(QMouseEvent* event) override;

   /// @brief Starts a generator-name drag after the pointer moves far enough.
   /// @param event Mouse move event.
   void mouseMoveEvent(QMouseEvent* event) override;

   /// @brief Completes an ordinary click unless the press started a drag.
   void mouseReleaseEvent(QMouseEvent* event) override;

private:
   /// @brief Position where the current mouse press began.
   QPoint dragStartPosition;
   /// @brief Generator name placed in drag mime data.
   QString generatorName;
   /// @brief Whether the current press has started a drag operation.
   bool dragStarted{false};
};

AddNodeButton::~AddNodeButton() = default;

void AddNodeButton::mousePressEvent(QMouseEvent* event) {
   if (event->button() == Qt::LeftButton) {
      dragStartPosition = event->pos();
      dragStarted = false;
   }
   QPushButton::mousePressEvent(event);
}

void AddNodeButton::mouseMoveEvent(QMouseEvent* event) {
   if (((event->buttons() & Qt::LeftButton) > 0) &&
       (event->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
      dragStarted = true;
      setDown(false);
      auto* drag = new QDrag(this);
      auto* mimeData = new QMimeData;
      mimeData->setText(generatorName);
      drag->setMimeData(mimeData);
      drag->exec(Qt::CopyAction | Qt::MoveAction);
      return;
   }
   QPushButton::mouseMoveEvent(event);
}

void AddNodeButton::mouseReleaseEvent(QMouseEvent* event) {
   if (dragStarted) {
      dragStarted = false;
      setDown(false);
      event->accept();
      return;
   }
   QPushButton::mouseReleaseEvent(event);
}

AddNodePanel::AddNodePanel(TextureProject& project) {
   setObjectName(QStringLiteral("addNodePanel"));
   buttonGroup = new QButtonGroup(this);
   buttonGroup->setExclusive(true);
   auto* layout = new QVBoxLayout(this);
   auto* area = new QScrollArea;
   area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   area->setWidgetResizable(true);
   QWidget* contents = new QWidget;
   auto* contentsLayout = new QVBoxLayout(contents);
   layout->setContentsMargins(0, 0, 0, 0);
   contentsLayout->setContentsMargins(0, 0, 0, 0);
   area->setFrameShape(QFrame::NoFrame);
   layout->addWidget(area);
   area->setWidget(contents);
#ifdef Q_OS_MAC
   // Leave room for the larger macOS UI font without truncating longer generator names.
   setFixedWidth(280);
#else
   setFixedWidth(250);
#endif
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
   setLayout(layout);

   auto* generatorsWidget = new QGroupBox("Generators");
   generatorsLayout = new QGridLayout();
   generatorsLayout->setContentsMargins(0, 0, 0, 0);
   generatorsWidget->setLayout(generatorsLayout);
   generatorsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(generatorsWidget);

   auto* filtersWidget = new QGroupBox("Filters");
   filtersLayout = new QGridLayout();
   filtersLayout->setContentsMargins(0, 0, 0, 0);
   filtersWidget->setLayout(filtersLayout);
   filtersWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(filtersWidget);

   auto* combinersWidget = new QGroupBox("Combiners");
   combinersLayout = new QGridLayout();
   combinersLayout->setContentsMargins(0, 0, 0, 0);
   combinersWidget->setLayout(combinersLayout);
   combinersWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   contentsLayout->addWidget(combinersWidget);

   customGeneratorsWidget = new QGroupBox("Custom Generators");
   customGeneratorsLayout = new QGridLayout;
   customGeneratorsLayout->setContentsMargins(0, 0, 0, 0);
   customGeneratorsWidget->setLayout(customGeneratorsLayout);
   contentsLayout->addWidget(customGeneratorsWidget);

   customFiltersWidget = new QGroupBox("Custom Filters");
   customFiltersLayout = new QGridLayout;
   customFiltersLayout->setContentsMargins(0, 0, 0, 0);
   customFiltersWidget->setLayout(customFiltersLayout);
   contentsLayout->addWidget(customFiltersWidget);

   customCombinersWidget = new QGroupBox("Custom Combiners");
   customCombinersLayout = new QGridLayout;
   customCombinersLayout->setContentsMargins(0, 0, 0, 0);
   customCombinersWidget->setLayout(customCombinersLayout);
   contentsLayout->addWidget(customCombinersWidget);
   updateCustomGroupVisibility();

   QWidget* spacerWidget = new QWidget;
   spacerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
   spacerWidget->setVisible(true);
   contentsLayout->addWidget(spacerWidget);
   QObject::connect(&project, &TextureProject::generatorAdded, this, &AddNodePanel::addGenerator);
   QObject::connect(&project, &TextureProject::generatorRemoved, this,
                    &AddNodePanel::removeGenerator);
}

void AddNodePanel::removeGenerator(const TextureGeneratorPtr& generator) {
   const auto iterator = widgets.find(generator);
   if (iterator != widgets.end()) {
      delete iterator.value();
      widgets.erase(iterator);
      updateCustomGroupVisibility();
   }
}

void AddNodePanel::clearGeneratorSelection() {
   buttonGroup->setExclusive(false);
   for (QAbstractButton* button : buttonGroup->buttons()) {
      button->setChecked(false);
   }
   buttonGroup->setExclusive(true);
}

void AddNodePanel::addGenerator(const TextureGeneratorPtr& generator) {
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
   auto* newButton = new AddNodeButton(this, generatorName);
   newButton->setCheckable(true);
   buttonGroup->addButton(newButton);
   QObject::connect(newButton, &QPushButton::clicked, this,
                    [this, generator]() { emit generatorSelected(generator); });
   widgets.insert(generator, newButton);
#ifdef Q_OS_MAC
   newButton->setFixedSize(112, 56);
#else
   newButton->setFixedSize(100, 60);
#endif
   newButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
   unsigned int hash = qHash(generatorName);
   QColor buttonColor(((hash & 0xFF0000) >> 16), ((hash & 0x00FF00) >> 8), (hash & 0x0000FF));
   QString fontColor("#ffffff");
   if ((buttonColor.red() * 0.299 + buttonColor.green() * 0.587 + buttonColor.blue() * 0.114) >
       170) {
      fontColor = "#000000";
   }
   newButton->setStyleSheet(QString("QPushButton {"
                                    "  background-color: %1;"
                                    "  color: %2;"
                                    "  border: 1px solid rgba(255, 255, 255, 32);"
                                    "  border-radius: 7px;"
                                    "  font-weight: 600;"
                                    "  padding: 6px;"
                                    "}"
                                    "QPushButton:hover { background-color: %3; }"
                                    "QPushButton:pressed { background-color: %4; }"
                                    "QPushButton:checked {"
                                    "  background-color: %4;"
                                    "  border: 3px solid palette(highlight);"
                                    "}")
                                .arg(buttonColor.name(), fontColor, buttonColor.lighter(112).name(),
                                     buttonColor.darker(120).name()));
   QGridLayout* destLayout = layoutFor(generator);
   int numButtons = destLayout->count();
   int row = numButtons / 2;
   int column = numButtons % 2;
   newButton->setText(generatorName);
   destLayout->addWidget(newButton, row, column, Qt::AlignLeft);
   updateCustomGroupVisibility();
}

QGridLayout* AddNodePanel::layoutFor(const TextureGeneratorPtr& generator) const {
   const bool custom = generator->getOrigin() == TextureGenerator::Origin::Custom;
   switch (generator->getType()) {
      case TextureGenerator::Type::Combiner:
         return custom ? customCombinersLayout : combinersLayout;
      case TextureGenerator::Type::Filter:
         return custom ? customFiltersLayout : filtersLayout;
      case TextureGenerator::Type::Generator:
         return custom ? customGeneratorsLayout : generatorsLayout;
   }
   return custom ? customGeneratorsLayout : generatorsLayout;
}

void AddNodePanel::updateCustomGroupVisibility() {
   customGeneratorsWidget->setVisible(customGeneratorsLayout->count() > 0);
   customFiltersWidget->setVisible(customFiltersLayout->count() > 0);
   customCombinersWidget->setVisible(customCombinersLayout->count() > 0);
}
