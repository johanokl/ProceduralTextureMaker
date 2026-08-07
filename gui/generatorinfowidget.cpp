// Part of the ProceduralTextureMaker project.
// http://github.com/johanokl/ProceduralTextureMaker
// Released under GPLv3.
// Johan Lindqvist (johan.lindqvist@gmail.com)

#include "gui/generatorinfowidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayoutItem>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace {

/// @brief Wraps long text and keeps its fixed height synchronized with the current width.
class AutoHeightLabel final : public QLabel {
public:
   using QLabel::QLabel;

   /// @brief Replaces the displayed text and recalculates the required wrapped height.
   /// @param text Plain text displayed by the label.
   void setWrappedText(const QString& text) {
      QLabel::setText(text);
      setMinimumHeight(0);
      setMaximumHeight(QWIDGETSIZE_MAX);
      updateGeometry();
      updateWrappedHeight();
      QTimer::singleShot(0, this, [this]() { updateWrappedHeight(); });
   }

   /// @brief Returns a minimum size that permits the label to shrink horizontally.
   /// @return QLabel's minimum size hint with no minimum width.
   QSize minimumSizeHint() const override {
      QSize hint = QLabel::minimumSizeHint();
      hint.setWidth(0);
      return hint;
   }

protected:
   /// @brief Recalculates the wrapped height after the available width changes.
   /// @param event Resize event containing the label's new dimensions.
   void resizeEvent(QResizeEvent* event) override {
      QLabel::resizeEvent(event);
      updateWrappedHeight();
   }

private:
   /// @brief Updates the fixed height to fit all text at the current width.
   void updateWrappedHeight() {
      if (updatingHeight || !wordWrap() || width() <= 0) {
         return;
      }
      const int wrappedHeight = heightForWidth(width());
      if (wrappedHeight >= 0 && height() != wrappedHeight) {
         updatingHeight = true;
         setFixedHeight(wrappedHeight);
         updateGeometry();
         updatingHeight = false;
      }
   }

   /// @brief Prevents recursive height updates while applying a new fixed height.
   bool updatingHeight{false};
};

/// @brief Returns the user-facing name of a generator category.
/// @param type Generator category to format.
/// @return Localized-ready category name displayed in the Details group.
QString typeName(TextureGenerator::Type type) {
   switch (type) {
      case TextureGenerator::Type::Filter:
         return QStringLiteral("Filter");
      case TextureGenerator::Type::Combiner:
         return QStringLiteral("Combiner");
      case TextureGenerator::Type::Generator:
         return QStringLiteral("Generator");
   }
   return QString();
}

}  // namespace

GeneratorInfoWidget::GeneratorInfoWidget(QWidget* parent) : QWidget(parent) {
   setObjectName(QStringLiteral("generatorInfoInspector"));
   auto* outerLayout = new QVBoxLayout(this);
   outerLayout->setContentsMargins(0, 0, 0, 0);

   auto* scrollArea = new QScrollArea;
   scrollArea->setFrameShape(QFrame::NoFrame);
   scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   scrollArea->setWidgetResizable(true);
   outerLayout->addWidget(scrollArea);

   auto* contents = new QWidget;
   auto* contentsLayout = new QVBoxLayout(contents);
   contentsLayout->setContentsMargins(12, 4, 12, 12);
   contentsLayout->setSpacing(8);
   scrollArea->setWidget(contents);

   nameLabel = new QLabel;
   nameLabel->setObjectName(QStringLiteral("generatorInfoName"));
   QFont headerFont = nameLabel->font();
   headerFont.setBold(true);
   if (headerFont.pointSizeF() > 0) {
      headerFont.setPointSizeF(headerFont.pointSizeF() + 8);
   } else {
      headerFont.setPixelSize(headerFont.pixelSize() + 11);
   }
   nameLabel->setFont(headerFont);
   nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   nameLabel->setContentsMargins(8, 4, 0, 0);
   nameLabel->setMinimumHeight(44);
   nameLabel->setWordWrap(true);
   contentsLayout->addWidget(nameLabel);

   auto* details = new QGroupBox(QStringLiteral("Details"));
   details->setObjectName(QStringLiteral("generatorInfoDetails"));
   details->setProperty("inspectorSection", true);
   auto* detailsLayout = new QFormLayout(details);
   detailsLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
   detailsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
   detailsLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
   detailsLayout->setHorizontalSpacing(12);
   detailsLayout->setVerticalSpacing(9);

   typeLabel = new QLabel;
   typeLabel->setObjectName(QStringLiteral("generatorInfoType"));
   originLabel = new QLabel;
   originLabel->setObjectName(QStringLiteral("generatorInfoOrigin"));
   sourceLabel = new AutoHeightLabel;
   sourceLabel->setObjectName(QStringLiteral("generatorInfoSource"));
   sourceLabel->setWordWrap(true);
   sourceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
   sourceLabel->setMinimumWidth(0);
   sourceLabel->setTextFormat(Qt::PlainText);
   sourceLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

   auto* typeKeyLabel = new QLabel(QStringLiteral("Type:"));
   typeKeyLabel->setObjectName(QStringLiteral("generatorInfoTypeKey"));
   auto* originKeyLabel = new QLabel(QStringLiteral("Origin:"));
   originKeyLabel->setObjectName(QStringLiteral("generatorInfoOriginKey"));
   auto* sourceKeyLabel = new QLabel(QStringLiteral("Source:"));
   sourceKeyLabel->setObjectName(QStringLiteral("generatorInfoSourceKey"));
   sourceKeyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   detailsLayout->addRow(typeKeyLabel, typeLabel);
   detailsLayout->addRow(originKeyLabel, originLabel);
   detailsLayout->addRow(sourceKeyLabel, sourceLabel);
   contentsLayout->addWidget(details);

   auto* description = new QGroupBox(QStringLiteral("Description"));
   description->setProperty("inspectorSection", true);
   auto* descriptionLayout = new QVBoxLayout(description);
   descriptionLabel = new QLabel;
   descriptionLabel->setObjectName(QStringLiteral("generatorInfoDescription"));
   descriptionLabel->setWordWrap(true);
   descriptionLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
   descriptionLabel->setTextFormat(Qt::PlainText);
   QFont descriptionFont = descriptionLabel->font();
   descriptionFont.setBold(true);
   descriptionLabel->setFont(descriptionFont);
   descriptionLayout->addWidget(descriptionLabel);
   contentsLayout->addWidget(description);

   auto* inputs = new QGroupBox(QStringLiteral("Inputs"));
   inputs->setProperty("inspectorSection", true);
   inputsLayout = new QVBoxLayout(inputs);
   inputsLayout->setSpacing(6);
   contentsLayout->addWidget(inputs);

   auto* settings = new QGroupBox(QStringLiteral("Available properties"));
   settings->setProperty("inspectorSection", true);
   settingsLayout = new QVBoxLayout(settings);
   settingsLayout->setSpacing(8);
   contentsLayout->addWidget(settings);
   contentsLayout->addStretch();
}

void GeneratorInfoWidget::clearLayout(QLayout* layout) {
   while (QLayoutItem* item = layout->takeAt(0)) {
      if (QWidget* widget = item->widget()) {
         delete widget;
      }
      delete item;
   }
}

void GeneratorInfoWidget::setGenerator(const TextureGeneratorPtr& newGenerator) {
   generator = newGenerator;
   clearLayout(inputsLayout);
   clearLayout(settingsLayout);
   if (generator.isNull()) {
      return;
   }

   nameLabel->setText(generator->getName());
   typeLabel->setText(typeName(generator->getType()));
   originLabel->setText(generator->getOrigin() == TextureGenerator::Origin::BuiltIn
                            ? QStringLiteral("Built-in")
                            : QStringLiteral("Custom"));
   const QString description = generator->getDescription();
   descriptionLabel->setText(description.isEmpty() ? QStringLiteral("None") : description);
   const QString source = generator->getSourceIdentity();
   static_cast<AutoHeightLabel*>(sourceLabel)
       ->setWrappedText(source.isEmpty() ? QStringLiteral("Built-in C++") : source);
   const QStringList inputs = generator->getSourceSlots();
   const QStringList displayedInputs =
       inputs.isEmpty() ? QStringList{QStringLiteral("None")} : inputs;
   for (const QString& input : displayedInputs) {
      auto* inputLabel = new QLabel(input);
      inputLabel->setObjectName(QStringLiteral("generatorInfoInput"));
      QFont inputFont = inputLabel->font();
      inputFont.setBold(true);
      inputLabel->setFont(inputFont);
      inputsLayout->addWidget(inputLabel);
   }

   QList<TextureGeneratorSetting> definitions = generator->getSettings().values();
   std::stable_sort(definitions.begin(), definitions.end(),
                    [](const TextureGeneratorSetting& left, const TextureGeneratorSetting& right) {
                       return left.order < right.order;
                    });
   if (definitions.isEmpty()) {
      settingsLayout->addWidget(new QLabel(QStringLiteral("No configurable properties.")));
      return;
   }

   for (const TextureGeneratorSetting& definition : definitions) {
      auto* label = new QLabel;
      label->setWordWrap(true);
      const QString displayName =
          definition.name.isEmpty() ? QStringLiteral("Unnamed property") : definition.name;
      QString details = QStringLiteral("<b>%1</b>").arg(displayName.toHtmlEscaped());
      if (!definition.description.isEmpty()) {
         details += QStringLiteral("<br>%1").arg(definition.description.toHtmlEscaped());
      }
      label->setText(details);
      settingsLayout->addWidget(label);
   }
}
