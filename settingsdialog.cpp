#include "settingsdialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>

// A small helper function: convert Qt::Key to a string
static QString keyToString(int key)
{
    switch (key) {
    case Qt::Key_F1: return "F1";
    case Qt::Key_F2: return "F2";
    case Qt::Key_F3: return "F3";
    case Qt::Key_F4: return "F4";
    case Qt::Key_Minus: return "Minus";
        // etc. or a generic approach
    }
    // fallback
    QChar c = QChar(key);
    return QString(c);
}

// The reverse: convert the user string to a Qt key code
static int stringToKey(const QString& str)
{
    if (str.compare("F1", Qt::CaseInsensitive) == 0) return Qt::Key_F1;
    if (str.compare("F2", Qt::CaseInsensitive) == 0) return Qt::Key_F2;
    if (str.compare("F3", Qt::CaseInsensitive) == 0) return Qt::Key_F3;
    if (str.compare("F4", Qt::CaseInsensitive) == 0) return Qt::Key_F4;
    if (str.compare("Minus", Qt::CaseInsensitive) == 0) return Qt::Key_Minus;
    // fallback: e.g. single character
    if (str.length() == 1) {
        return str.at(0).toUpper().unicode();
    }
    // default
    return 0;
}

SettingsDialog::SettingsDialog(QWidget* parent, const QMap<MainWindow::Action, QString>& actionNameMap)
    : QDialog(parent), m_actionNameMap(actionNameMap)
{
    setWindowTitle("Assign Hotkeys");
    QVBoxLayout* layout = new QVBoxLayout(this);

    // --- Add our custom "Export Ruler to Photoshop" button ---
    QPushButton* exportJSXButton = new QPushButton("Export Ruler to Photoshop", this);
    exportJSXButton->setToolTip("Generate JSX file for exporting ruler to Photoshop");
    layout->addWidget(exportJSXButton);
    connect(exportJSXButton, &QPushButton::clicked, this, [this]() {
        // 1) Use QDir::tempPath() to get a system-wide temporary folder
        QString tempFolder = QDir::tempPath();
        // Create (if needed) a subfolder "ruler_images" in the temp folder
        QString folder = QDir(tempFolder).filePath("ruler_images");
        QDir().mkpath(folder);

        // 2) Build the full file path (use forward slashes for JSX)
        QString filePath = QDir(folder).filePath("export ruler to photoshop.jsx");
        filePath.replace("\\", "/");

        // 3) Open the file for writing
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Could not open file for writing:" << filePath;
            return;
        }
        QTextStream out(&file);

        // 4) Write your exact JSX code with the dynamic file path
        out << "// Ensure the script targets the active document" << "\n";
        out << "app.refresh();" << "\n";
        out << "var doc = app.activeDocument;" << "\n\n";
        out << "// Get the currently selected layer" << "\n";
        out << "var imageLayer = doc.activeLayer;" << "\n\n";
        out << "// Open and import the external image on top" << "\n";
        out << "var filePath = \"" << QDir(folder).filePath("ruler_image.png").replace("\\", "/") << "\";" << "\n";
        out << "var importedDoc = open(new File(filePath));" << "\n";
        out << "importedDoc.activeLayer.duplicate(doc, ElementPlacement.PLACEATBEGINNING);" << "\n";
        out << "importedDoc.close(SaveOptions.DONOTSAVECHANGES);" << "\n\n";
        out << "// Ensure the new layer is selected" << "\n";
        out << "var importedLayer = doc.activeLayer;" << "\n\n";
        out << "// Resize and center the imported layer" << "\n";
        out << "var importedBounds = importedLayer.bounds;" << "\n";
        out << "var importedWidth = importedBounds[2] - importedBounds[0];" << "\n";
        out << "var importedHeight = importedBounds[3] - importedBounds[1];" << "\n";
        out << "var canvasWidth = doc.width;" << "\n";
        out << "var canvasHeight = doc.height;" << "\n\n";
        out << "var scaleX = (canvasWidth / importedWidth) * 100;" << "\n";
        out << "var scaleY = (canvasHeight / importedHeight) * 100;" << "\n";
        out << "var scale = Math.min(scaleX, scaleY);" << "\n\n";
        out << "importedLayer.resize(scale, scale, AnchorPosition.MIDDLECENTER);" << "\n\n";
        out << "// Recalculate the bounds after resizing" << "\n";
        out << "var newBounds = importedLayer.bounds;" << "\n";
        out << "var newWidth = newBounds[2] - newBounds[0];" << "\n";
        out << "var newHeight = newBounds[3] - newBounds[1];" << "\n\n";
        out << "// Translate the layer to center it on the canvas" << "\n";
        out << "var offsetX = (canvasWidth - newWidth) / 2 - newBounds[0];" << "\n";
        out << "var offsetY = (canvasHeight - newHeight) / 2 - newBounds[1];" << "\n";
        out << "importedLayer.translate(offsetX, offsetY);" << "\n\n";
        out << "// Recalculate the bounds after translation" << "\n";
        out << "newBounds = importedLayer.bounds;" << "\n\n";
        out << "// Create a new layer and fill it with the desired color" << "\n";
        out << "var colorFillLayer = doc.artLayers.add();" << "\n";
        out << "colorFillLayer.name = \"Color Fill\";" << "\n";
        out << "colorFillLayer.move(importedLayer, ElementPlacement.PLACEAFTER);" << "\n\n";
        out << "// Fill the new layer with a solid color within the imported layer's bounds" << "\n";
        out << "var fillColor = new SolidColor();" << "\n";
        out << "fillColor.rgb.red = 204; // 80% grey" << "\n";
        out << "fillColor.rgb.green = 204;" << "\n";
        out << "fillColor.rgb.blue = 204;" << "\n\n";
        out << "doc.selection.select([" << "\n";
        out << "    [newBounds[0], newBounds[1]]," << "\n";
        out << "    [newBounds[2], newBounds[1]]," << "\n";
        out << "    [newBounds[2], newBounds[3]]," << "\n";
        out << "    [newBounds[0], newBounds[3]]" << "\n";
        out << "]);" << "\n";
        out << "doc.selection.fill(fillColor);" << "\n";
        out << "doc.selection.deselect();" << "\n\n";
        out << "// Create a new clipped layer above the color fill layer" << "\n";
        out << "var clippedLayer = doc.artLayers.add();" << "\n";
        out << "clippedLayer.name = \"Clipped Layer\";" << "\n";
        out << "clippedLayer.move(colorFillLayer, ElementPlacement.PLACEAFTER);" << "\n";
        out << "clippedLayer.grouped = true; // Properly clip this layer to the \"Color Fill\" layer" << "\n\n";
        out << "// Group all layers into a folder" << "\n";
        out << "var group = doc.layerSets.add();" << "\n";
        out << "group.name = \"Image and Fill Group\";" << "\n\n";
        out << "// Reorder layers: ensure image layer is at the top" << "\n";
        out << "importedLayer.move(group, ElementPlacement.INSIDE);" << "\n";
        out << "colorFillLayer.move(group, ElementPlacement.INSIDE);" << "\n";
        out << "clippedLayer.move(group, ElementPlacement.INSIDE);" << "\n\n";
        out << "// Ensure importedLayer is at the top of the group" << "\n";
        out << "importedLayer.move(group, ElementPlacement.PLACEATBEGINNING);" << "\n\n";
        out << "// Lock the importedLayer" << "\n";
        out << "importedLayer.allLocked = true;" << "\n\n";
        out << "// Ensure the clipped layer is properly clipped to the color fill layer" << "\n";
        out << "clippedLayer.grouped = true;" << "\n\n";
        out << "// Set the clipped layer as the active layer" << "\n";
        out << "doc.activeLayer = clippedLayer;" << "\n\n";
        out << "// Create a new dark layer and move it to the bottom of the group" << "\n";
        out << "var darkLayer = doc.artLayers.add();" << "\n";
        out << "darkLayer.name = \"Dark Background\";" << "\n";
        out << "darkLayer.move(group, ElementPlacement.INSIDE);" << "\n";
        out << "darkLayer.move(group, ElementPlacement.PLACEATEND);" << "\n\n";
        out << "// Fill the new layer with a dark color" << "\n";
        out << "var darkColor = new SolidColor();" << "\n";
        out << "darkColor.rgb.red = 25;" << "\n";
        out << "darkColor.rgb.green = 25;" << "\n";
        out << "darkColor.rgb.blue = 25;" << "\n\n";
        out << "doc.selection.selectAll();" << "\n";
        out << "doc.selection.fill(darkColor);" << "\n";
        out << "doc.selection.deselect();" << "\n\n";
        out << "// Final message" << "\n";
        out << "$.writeln(\"Layers grouped, and the clipped layer is correctly clipped to the color fill. Clipped layer is now active. Dark background layer added.\");" << "\n";

        file.close();
        qDebug() << "JSX file generated at:" << filePath;

        // Store the custom JSX path in QSettings so MainWindow can retrieve it later
        QSettings settings("YourCompany", "YourApp");
        settings.setValue("customJSXPath", filePath);
        qDebug() << "Stored custom JSX path:" << filePath;
        });


    // Add the new button to the layout (for example, at the bottom of the dialog)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());

    if (mainLayout)
        mainLayout->addWidget(exportJSXButton);

    // Existing UI setup for hotkeys follows...
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::onCancelClicked);

    mainLayout->addWidget(buttonBox);
}

void SettingsDialog::setKeyMap(const QMap<MainWindow::Action, int>& actionKeyMap)
{
    m_tempKeyMap = actionKeyMap;  // local copy
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) return;

    QFormLayout* form = new QFormLayout;
    mainLayout->insertLayout(0, form);

    for (auto it = actionKeyMap.cbegin(); it != actionKeyMap.cend(); ++it)
    {
        MainWindow::Action action = it.key();
        int key = it.value();

        QLabel* label = new QLabel(this);
        QString actionName = m_actionNameMap.value(action, QString::number(static_cast<int>(action)) + " (Action)");
        label->setText(actionName);

        QLineEdit* edit = new QLineEdit(this);
        edit->setText(keyToString(key));

        form->addRow(label, edit);

        ActionEdit ae;
        ae.label = label;
        ae.lineEdit = edit;
        ae.action = action;
        m_actionsEdits.append(ae);
    }
}

QMap<MainWindow::Action, int> SettingsDialog::getKeyMap() const
{
    return m_tempKeyMap;
}

void SettingsDialog::onOkClicked()
{
    // For each line edit, parse the user input
    for (auto& ae : m_actionsEdits) {
        QString text = ae.lineEdit->text();
        int key = stringToKey(text);
        if (key != 0) {
            m_tempKeyMap[ae.action] = key;
        }
        else {
            // If parse fails, do something (like keep old value or set to 0)
            qDebug() << "Could not parse key from" << text << "using default 0";
            m_tempKeyMap[ae.action] = 0;
        }
    }
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}