#include "mainwindow.h"
#include "zoomablegraphicsview.h"
#include "imageutils.h"    // For image processing utilities

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QRandomGenerator>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QTimer>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QMenu>
#include <QDebug>
#include <random>
#include <algorithm>
#include <QApplication>
#include <settingsdialog.h>
#include "settingsdialog.h"
#include "imageutils.h"
#include <imageutils.h>

// Example of a small custom QSpinBox for convenience
class SelectAllSpinBox : public QSpinBox
{
public:
    SelectAllSpinBox(QWidget* parent = nullptr) : QSpinBox(parent)
    {
        setFocusPolicy(Qt::ClickFocus);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        QSpinBox::mousePressEvent(event);
        this->selectAll();
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
    m_countdownTimer(new QTimer(this)),
    isTimerRunning(false),
    countdownTime(QTime(0, 10, 0)),
    scheduleActive(false),
    m_copyPasteEnabled(false),
    m_isGrayscale(false),
    m_isPosterized(false),
    m_isBlurred(false),
    m_isMedianFiltered(false),
    currentScheduleIndex(0),
    m_originalPixmapItem(nullptr),
    m_grayscalePixmapItem(nullptr),
    m_view(nullptr)
{

    // Initialize m_actionNameMap
    m_actionNameMap[Action::OpenDirectory] = "Open Directory";
    m_actionNameMap[Action::History] = "History";
    m_actionNameMap[Action::NextImage] = "Next Image";

    // Window on top
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // Accept drops
    setAcceptDrops(true);

    // Suppose:
    m_actionKeyMap[Action::OpenDirectory] = Qt::Key_O;    // 'O' for open
    m_actionKeyMap[Action::History] = Qt::Key_H;
    m_actionKeyMap[Action::NextImage] = Qt::Key_Minus;
    m_actionKeyMap[Action::Flip] = Qt::Key_F;
    m_actionKeyMap[Action::Grayscale] = Qt::Key_G;
    // etc. add more for each button

    // Build the reverse map:
    m_keyActionMap.clear();
    for (auto it = m_actionKeyMap.cbegin(); it != m_actionKeyMap.cend(); ++it) {
        const Action action = it.key();
        const int key = it.value();
        m_keyActionMap[key] = action;
    }

    // Initialize m_timeButton as a member variable
    m_timeButton = new QPushButton(countdownTime.toString("hh:mm:ss"), this);
    m_timeButton->setToolTip("Start/Stop Timer");
    m_timeButton->setStyleSheet("QPushButton { color: red; }"); // Initial color
    connect(m_timeButton, &QPushButton::clicked, this, [this]() {
        // Toggle the timer running/stopped
        if (isTimerRunning) {
            // Stop
            m_countdownTimer->stop();
            m_timeButton->setStyleSheet("color: red;");
            qDebug() << "Timer stopped.";
        }
        else {
            // Start
            m_countdownTimer->start(1000); // every second
            m_timeButton->setStyleSheet("color: white;");
            qDebug() << "Timer started.";
        }
        isTimerRunning = !isTimerRunning;
        });

    // Apply a global dark theme style sheet
    qApp->setStyleSheet(R"(
        QWidget {
            background-color: gray;
            color: black;
            font: 18px 'Segoe UI';
        }
        QPushButton {
            background-color: #3E3E3E;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 6px;
            min-width: 40px;
            min-height: 40px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton:pressed {
            background-color: #1E1E1E;
        }
        QSpinBox, QCheckBox, QComboBox, QLineEdit {
            background-color: #3E3E3E;
            border: 1px solid #555;
            border-radius: 4px;
            padding: 2px;
            color: #FFFFFF;
        }
        QGroupBox {
            border: 1px solid #666;
            border-radius: 6px;
            margin-top: 6px;
            font: bold 18px 'Segoe UI';
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 2px 10px;
            background-color: #2D2D2D;
            color: #FFFFFF;
            border-radius: 4px;
        }
        QLabel {
            font: 18px 'Segoe UI';
        }
    )");

    // UI Layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout1 = new QHBoxLayout;
    QHBoxLayout* buttonLayout2 = new QHBoxLayout;

    // ----- Row 1 Buttons -----

    QPushButton* openButton = new QPushButton("📂");
    connect(openButton, &QPushButton::clicked, this, [this]() {
        performAction(Action::OpenDirectory);
        });
    buttonLayout1->addWidget(openButton);

    QPushButton* historyButton = new QPushButton("📜");
    connect(historyButton, &QPushButton::clicked, this, [this]() {
        performAction(Action::History);
        });
    buttonLayout1->addWidget(historyButton);

    QPushButton* nextButton = new QPushButton("➡️");
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextButtonClicked);
    buttonLayout1->addWidget(nextButton);
    

    QPushButton* flipButton = new QPushButton("🔄");
    connect(flipButton, &QPushButton::clicked, this, &MainWindow::onFlipButtonClicked);
    buttonLayout1->addWidget(flipButton);

    QPushButton* grayscaleButton = new QPushButton("⚫⚪");
    connect(grayscaleButton, &QPushButton::clicked, this, &MainWindow::onGrayscaleButtonClicked);
    buttonLayout1->addWidget(grayscaleButton);

    buttonLayout1->addWidget(m_timeButton);

    // Timer UI (example)
    // Attempt to load "countdownTime" from settings
    const QString ctime = settings.value("countdownTime", "00:10:00").toString();
    countdownTime = QTime::fromString(ctime, "hh:mm:ss");
    // Or if invalid, fallback to QTime(0,10)
    if (!countdownTime.isValid()) {
        countdownTime = QTime(0, 10);
    }

    // Load the action key map
    QVariantMap actionKeyMapVariant = settings.value("actionKeyMap").toMap();
    for (auto it = actionKeyMapVariant.cbegin(); it != actionKeyMapVariant.cend(); ++it) {
        m_actionKeyMap[static_cast<Action>(it.key().toInt())] = it.value().toInt();
    }
    qDebug() << "Loaded action key map:" << actionKeyMapVariant;


    // Rebuild the reverse map
    m_keyActionMap.clear();
    for (auto it = m_actionKeyMap.cbegin(); it != m_actionKeyMap.cend(); ++it) {
        m_keyActionMap[it.value()] = it.key();
    }

    // Create and configure the hours spin box
    SelectAllSpinBox* hoursSpinBox = new SelectAllSpinBox;
    hoursSpinBox->setRange(0, 23);
    hoursSpinBox->setValue(0);
    hoursSpinBox->setFixedWidth(50); // Set fixed width
    buttonLayout1->addWidget(hoursSpinBox);

    // Create and configure the minutes spin box
    SelectAllSpinBox* minutesSpinBox = new SelectAllSpinBox;
    minutesSpinBox->setRange(0, 59);
    minutesSpinBox->setValue(10);
    minutesSpinBox->setFixedWidth(50); // Set fixed width
    buttonLayout1->addWidget(minutesSpinBox);

    // Create and configure the seconds spin box
    SelectAllSpinBox* secondsSpinBox = new SelectAllSpinBox;
    secondsSpinBox->setRange(0, 59);
    secondsSpinBox->setValue(0);
    secondsSpinBox->setFixedWidth(50); // Set fixed width
    buttonLayout1->addWidget(secondsSpinBox);

    // Master "Start Timer" Button
    startTimerButton = new QPushButton("Start");
    connect(startTimerButton, &QPushButton::clicked, this, [this, hoursSpinBox, minutesSpinBox, secondsSpinBox]() {
        // Set the countdown time based on spin boxes
        this->countdownTime.setHMS(hoursSpinBox->value(), minutesSpinBox->value(), secondsSpinBox->value());
        // **Store the default countdown time**
        this->m_defaultCountdownTime = this->countdownTime;
        // Update the timer button display
        m_timeButton->setText(this->countdownTime.toString("hh:mm:ss"));
        qDebug() << "Timer set to: " << this->countdownTime.toString("hh:mm:ss");
        // **Start the timer automatically**
        if (!isTimerRunning) {
            m_timeButton->click(); // This will start the timer
        }
        });
    buttonLayout1->addWidget(startTimerButton);

    // Toggle row 2
    QPushButton* toggleButton = new QPushButton("Toggle");
    connect(toggleButton, &QPushButton::clicked, this, [buttonLayout2]() {
        bool isVisible = buttonLayout2->itemAt(0)->widget()->isVisible();
        for (int i = 0; i < buttonLayout2->count(); ++i) {
            QWidget* w = buttonLayout2->itemAt(i)->widget();
            if (w) w->setVisible(!isVisible);
        }
        });
    buttonLayout1->addWidget(toggleButton);

    // ----- Row 2 Buttons -----
    QPushButton* startScheduleButton = new QPushButton("📅");
    connect(startScheduleButton, &QPushButton::clicked, this, &MainWindow::startSchedule);
    buttonLayout2->addWidget(startScheduleButton);

    QPushButton* editScheduleButton = new QPushButton("📝📅");
    connect(editScheduleButton, &QPushButton::clicked, this, &MainWindow::editSchedule);
    buttonLayout2->addWidget(editScheduleButton);

    QPushButton* rotateClockwiseButton = new QPushButton("↩️");
    connect(rotateClockwiseButton, &QPushButton::clicked, this, [this]() {
        if (m_view) m_view->rotate(10);
        });
    buttonLayout2->addWidget(rotateClockwiseButton);

    QPushButton* rotateCounterclockwiseButton = new QPushButton("↪️");
    connect(rotateCounterclockwiseButton, &QPushButton::clicked, this, [this]() {
        if (m_view) m_view->rotate(-10);
        });
    buttonLayout2->addWidget(rotateCounterclockwiseButton);

    QPushButton* posterizeButton = new QPushButton("🖼️");
    buttonLayout2->addWidget(posterizeButton);

    QSpinBox* levelsSpinBox = new QSpinBox;
    levelsSpinBox->setRange(1, 100);
	levelsSpinBox->setValue(3);
    buttonLayout2->addWidget(levelsSpinBox);

    // Connect the posterize button to trigger posterization using the spinbox's current value
    connect(posterizeButton, &QPushButton::clicked, this, [this, levelsSpinBox]() {
        int levels = levelsSpinBox->value();
        onPosterizeButtonClicked(levels);
        });

    // Connect the spinbox value change to update the posterized image
    connect(levelsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int levels) {
        if (m_isPosterized) {
            applyPosterization(levels);
        }
        });

    QPushButton* blurButton = new QPushButton("💧");
    connect(blurButton, &QPushButton::clicked, this, &MainWindow::onDegradeButtonClicked);
    buttonLayout2->addWidget(blurButton);

    QPushButton* medianFilterButton = new QPushButton("📊");
    connect(medianFilterButton, &QPushButton::clicked, this, &MainWindow::onMedianFilterButtonClicked);
    buttonLayout2->addWidget(medianFilterButton);

    QPushButton* toggleLinesButton = new QPushButton("📏");
    connect(toggleLinesButton, &QPushButton::clicked, this, [this]() {
        if (m_view) {
            bool isVisible = m_view->areLinesVisible();
            m_view->setLinesVisibility(!isVisible);
        }
        });
    buttonLayout2->addWidget(toggleLinesButton);

    QPushButton* copyRulerButton = new QPushButton("📏⬆️", this);
    connect(copyRulerButton, &QPushButton::clicked, this, [this]() {
        if (!m_view) return;

        // 1) Ensure the ruler lines are visible.
        m_view->setLinesVisibility(true);

        // 2) Create the folder in the system temporary directory.
        QString tempFolder = QDir::tempPath();  // system temp folder
        QString folderPath = QDir(tempFolder).filePath("ruler_images");
        QDir().mkpath(folderPath);

        // 3) Build the file path for the ruler image.
        QString rulerFilePath = QDir(folderPath).filePath("ruler_image.png");

        // 4) Tell the view to save just the ruler lines to that file.
        m_view->saveRulerImage(rulerFilePath);

        // 5) Store this path in a member variable.
        m_tempRulerFilePath = rulerFilePath;
        qDebug() << "Saved ruler lines to" << rulerFilePath;

        // 6) Refresh the custom JSX script path from settings.
        refreshCustomJSXPath();  // This should update m_customJSXPath from QSettings.
        QString scriptPath;
        if (!m_customJSXPath.isEmpty()) {
            scriptPath = m_customJSXPath;
            qDebug() << "Using custom JSX script path:" << scriptPath;
        }
        else {
            qDebug() << "No custom JSX script path is set. Cannot launch JSX script.";
            return; // Exit early if no custom JSX file is available.
        }

        // 7) Build the argument list for cmd.exe using the "start" command.
        // The command syntax: cmd.exe /C start "" <scriptPath>
        QStringList args;
        args << "/C" << "start" << "" << scriptPath;
        qDebug() << "Launching JSX script using cmd.exe with args:" << args;

        // 8) Launch the JSX file using cmd.exe.
        bool processStarted = QProcess::startDetached("cmd.exe", args);
        if (processStarted) {
            qDebug() << "JSX script launched successfully.";
        }
        else {
            qDebug() << "Failed to launch JSX script.";
        }
        });
    buttonLayout2->addWidget(copyRulerButton);



    QPushButton* settingsButton = new QPushButton("⚙️");
    buttonLayout2->addWidget(settingsButton);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);

    QPushButton* copyDisplayedButton = new QPushButton("📋🖼️");
    buttonLayout2->addWidget(copyDisplayedButton);
    connect(copyDisplayedButton, &QPushButton::clicked, this, [this]() {
        // 1) Decide which pixmap is currently shown.
        //    If you do grayscale by showing/hiding m_grayscalePixmapItem, check isVisible().
        QPixmap displayedPixmap;
        if (m_grayscalePixmapItem && m_grayscalePixmapItem->isVisible()) {
            // The grayscale item is visible.
            displayedPixmap = m_grayscalePixmapItem->pixmap();
        }
        else if (m_originalPixmapItem) {
            // The "Original" item (which might be blurred, median, or posterized in your code).
            displayedPixmap = m_originalPixmapItem->pixmap();
        }
        else {
            QMessageBox::warning(this, "Error", "No image is currently displayed.");
            return;
        }

        // 2) Remove any old temp file.
        if (!m_tempDisplayedFilePath.isEmpty()) {
            QFile::remove(m_tempDisplayedFilePath);
            m_tempDisplayedFilePath.clear();
        }

        // 3) Construct a path in the system temporary directory.
        QString folderPath = QDir::tempPath() + "/displayed_image";  // Portable temp folder
        QDir().mkpath(folderPath);
        m_tempDisplayedFilePath = folderPath + "/current_image.png";

        // 4) Save the displayed image to this path.
        bool ok = displayedPixmap.save(m_tempDisplayedFilePath);
        if (!ok) {
            QMessageBox::warning(this, "Error", "Failed to save the displayed image.");
            return;
        }

        // 5) Copy the same pixmap to the system clipboard for paste capability.
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setPixmap(displayedPixmap);

        qDebug() << "Saved displayed image to" << m_tempDisplayedFilePath
            << "and copied to clipboard.";
        });


    // Add both button rows to the main layout
    mainLayout->addLayout(buttonLayout1);
    mainLayout->addLayout(buttonLayout2);

    

    // Create and set up the ZoomableGraphicsView
    m_view = new ZoomableGraphicsView(this);
    mainLayout->addWidget(m_view);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);

    // Restore settings

    m_customJSXPath = settings.value("customJSXPath").toString();
    qDebug() << "Loaded custom JSX path:" << m_customJSXPath;

    QStringList scheduleStringList = settings.value("schedule").toStringList();
    for (const QString& timeString : scheduleStringList) {
        schedule.append(QTime::fromString(timeString));
    }
    qDebug() << "Loaded schedule:" << scheduleStringList;

    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    qDebug() << "Loaded main window geometry.";

    m_directory = settings.value("lastDirectory").toString();
    qDebug() << "Loaded last directory:" << m_directory;

    directoryHistory = settings.value("directoryHistory").toStringList();
    qDebug() << "Loaded directory history:" << directoryHistory;

    if (!m_directory.isEmpty()) {
        setDirectory(m_directory);
        QTimer::singleShot(100, this, [this]() {
            loadImageFromDirectory(m_directory);
            });
    }

    // Connect Timer
    connect(m_countdownTimer, &QTimer::timeout, this, [this]() {
        if (!isTimerRunning) return; // If somehow the timer is on but we don't want to run

        // Subtract 1 second
        countdownTime = countdownTime.addSecs(-1);
        m_timeButton->setText(countdownTime.toString("hh:mm:ss"));
        qDebug() << "Timer tick: " << countdownTime.toString("hh:mm:ss");

        // If time is up
        if (countdownTime <= QTime(0, 0, 0)) {
            qDebug() << "Time is up.";
            // 1) Load new image
            loadImageFromDirectory(m_directory);

            // 2) If scheduleActive, move to next schedule item
            if (scheduleActive) {
                currentScheduleIndex++;
                startNextTimerInSchedule();
                // This will set countdownTime to the next item or loop the schedule
            }
            else {
                // **In non-schedule mode, reset the countdown time and continue**
                countdownTime = m_defaultCountdownTime;
                m_timeButton->setText(countdownTime.toString("hh:mm:ss"));
                qDebug() << "Timer reset to default: " << countdownTime.toString("hh:mm:ss");
                // **No need to stop the timer; it continues running**
            }
        }
        });
}


// ---------------  Destructor ---------------
MainWindow::~MainWindow()
{
    if (!m_tempRulerFilePath.isEmpty()) {
        QFile::remove(m_tempRulerFilePath);
    }
    // ...
}


// ---------------  closeEvent ---------------
void MainWindow::closeEvent(QCloseEvent* event)
{
    // Convert schedule to a QStringList
    QStringList scheduleStringList;
    for (const QTime& t : schedule) {
        scheduleStringList.append(t.toString("hh:mm:ss"));
    }
    // Remove the displayed image file if it exists
    if (!m_tempDisplayedFilePath.isEmpty()) {
        QFile::remove(m_tempDisplayedFilePath);
    }
    // Save settings
    settings.setValue("schedule", scheduleStringList);
    qDebug() << "Saved schedule:" << scheduleStringList;

    settings.setValue("mainWindowGeometry", saveGeometry());
    qDebug() << "Saved main window geometry.";

    settings.setValue("lastDirectory", m_directory);
    qDebug() << "Saved last directory:" << m_directory;

    settings.setValue("directoryHistory", directoryHistory);
    qDebug() << "Saved directory history:" << directoryHistory;

    settings.setValue("countdownTime", countdownTime.toString("hh:mm:ss"));
    qDebug() << "Saved countdown time:" << countdownTime.toString("hh:mm:ss");

    // Save the action key map
    QVariantMap actionKeyMapVariant;
    for (auto it = m_actionKeyMap.cbegin(); it != m_actionKeyMap.cend(); ++it) {
        actionKeyMapVariant.insert(QString::number(static_cast<int>(it.key())), it.value());
    }
    settings.setValue("actionKeyMap", actionKeyMapVariant);
    qDebug() << "Saved action key map:" << actionKeyMapVariant;

    QWidget::closeEvent(event);
}

// ---------------  Key Press for Shortcuts ---------------
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();

    // check if there's an action
    if (m_keyActionMap.contains(key)) {
        Action action = m_keyActionMap[key];
        performAction(action);
    }
    else if ((key == Qt::Key_V) && (event->modifiers() & Qt::ControlModifier)) {
        // A special case for Ctrl+V
        processClipboardImage();
    }
    else {
        QWidget::keyPressEvent(event);
    }
}


void MainWindow::performAction(Action action)
{
    switch (action)
    {
    case Action::OpenDirectory:
        onOpenButtonClicked();  // or the same logic inline
        break;
    case Action::History:
        onHistoryButtonClicked();
        break;
    case Action::NextImage:
        onNextButtonClicked();
        break;
    case Action::Flip:
        onFlipButtonClicked();
        break;
    case Action::Grayscale:
        onGrayscaleButtonClicked();
        break;
        // ...
    case Action::SaveRuler:
        // The same code you'd run in copyRulerButton clicked-lambda
        break;
    case Action::CopyDisplayedImage:
        // The same code you'd run in your copyDisplayedButton lambda
        break;
        // etc. for each Action
    default:
        qDebug() << "No matching Action logic.";
        break;
    }
}



// settings
void MainWindow::onSettingsButtonClicked()
{
    QMap<Action, QString> actionNameMap = {
        { Action::OpenDirectory, "Open Directory" },
		{ Action::History, "History" },
		{ Action::NextImage, "Next Image" },
		{ Action::Flip, "Flip" },
		{ Action::Grayscale, "Grayscale" },
		{ Action::StartSchedule, "Start Schedule" },
		{ Action::EditSchedule, "Edit Schedule" },
		{ Action::RotateClockwise, "Rotate Clockwise" },
		{ Action::RotateCounterclockwise, "Rotate Counterclockwise" },
		{ Action::Posterize, "Posterize" },
		{ Action::Blur, "Blur" },
		{ Action::MedianFilter, "Median Filter" },
		{ Action::ToggleLines, "Toggle Lines" },
		{ Action::SaveRuler, "Save Ruler" },
		{ Action::CopyDisplayedImage, "Copy Displayed Image" },
		{ Action::ProcessClipboard, "Process Clipboard" },
        // Add other actions here
    };

    SettingsDialog dialog(this, actionNameMap);
    dialog.setKeyMap(m_actionKeyMap);
    if (dialog.exec() == QDialog::Accepted) {
        m_actionKeyMap = dialog.getKeyMap();
        // Rebuild the reverse map
        m_keyActionMap.clear();
        for (auto it = m_actionKeyMap.cbegin(); it != m_actionKeyMap.cend(); ++it) {
            m_keyActionMap[it.value()] = it.key();
        }
    }
}


void MainWindow::refreshCustomJSXPath() {
    QSettings settings("YourCompany", "YourApp");
    m_customJSXPath = settings.value("customJSXPath").toString();
    qDebug() << "Refreshed custom JSX path:" << m_customJSXPath;
}

// ---------------  Directory Setup ---------------
void MainWindow::setDirectory(const QString& directory)
{
    m_directory = directory;
    directoryHistory.removeAll(directory);
    directoryHistory.prepend(directory);

    if (directoryHistory.size() > 10) {
        directoryHistory.removeLast();
    }

    settings.setValue("lastDirectory", m_directory);
    settings.setValue("directoryHistory", directoryHistory);

    // Populate files
    m_files.clear();
    QDirIterator it(directory, QStringList()
        << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif" << "*.webp",
        QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        m_files << it.fileInfo();
    }

    // Shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_files.begin(), m_files.end(), g);

    m_currentIndex = 0;
}

QString MainWindow::getRandomImage(const QString& directory)
{
    Q_UNUSED(directory);
    if (m_currentIndex >= m_files.size()) {
        // reshuffle
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_files.begin(), m_files.end(), g);
        m_currentIndex = 0;
    }
    if (m_files.isEmpty()) {
        return QString();
    }
    return m_files[m_currentIndex++].filePath();
}

// ---------------  Slots ---------------
void MainWindow::onOpenButtonClicked()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Open Directory");
    if (!directory.isEmpty()) {
        setDirectory(directory);
        loadImageFromDirectory(directory);
    }
}

void MainWindow::onHistoryButtonClicked()
{
    QMenu menu;
    for (const QString& directory : directoryHistory) {
        menu.addAction(directory, this, [this, directory]() {
            setDirectory(directory);
            loadImageFromDirectory(directory);
            });
    }
    menu.exec(QCursor::pos());
}

void MainWindow::onNextButtonClicked()
{
    if (m_directory.isEmpty()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirmation",
        "Are you sure you want to load the next image?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (scheduleActive) {
            // 1) Manually skip the countdown. That means we treat the current
            //    countdown as finished. We load the image:
            loadImageFromDirectory(m_directory);

            // 2) Move on to the next schedule item
            currentScheduleIndex++;
            // 3) Start the next timer in schedule
            startNextTimerInSchedule();
        }
        else {
            // No schedule: normal behavior
            loadImageFromDirectory(m_directory);
        }
    }
}


void MainWindow::onFlipButtonClicked()
{
    if (!m_originalPixmapItem || !m_grayscalePixmapItem) return;

    // Flip original
    QImage origImg = m_originalPixmapItem->pixmap().toImage().mirrored(true, false);
    m_originalPixmapItem->setPixmap(QPixmap::fromImage(origImg));

    // Flip grayscale
    QImage grayImg = m_grayscalePixmapItem->pixmap().toImage().mirrored(true, false);
    m_grayscalePixmapItem->setPixmap(QPixmap::fromImage(grayImg));
}

void MainWindow::onGrayscaleButtonClicked()
{
    if (!m_originalPixmapItem || !m_grayscalePixmapItem) return;
    bool grayVisible = m_grayscalePixmapItem->isVisible();

    m_originalPixmapItem->setVisible(grayVisible);
    m_grayscalePixmapItem->setVisible(!grayVisible);

    if (!grayVisible && m_copyPasteEnabled) {
        // If turning on grayscale and copy/paste is enabled, save grayscale
        QImage grayscaleImg = m_grayscalePixmapItem->pixmap().toImage();
        saveImageToSharedFolder(grayscaleImg, "grayscale_shared");
    }
}

// New method to apply posterization
void MainWindow::applyPosterization(int levels)
{
    if (!m_originalPixmapItem || !m_grayscalePixmapItem) return;

    // Use the original image for posterization
    QImage orig = m_originalPixmap.toImage();
    QImage posterizedOrig = ImageUtils::posterize(orig, levels);

    QImage posterGray = ImageUtils::convertToGrayscale(posterizedOrig);

    m_originalPixmapItem->setPixmap(QPixmap::fromImage(posterizedOrig));
    m_grayscalePixmapItem->setPixmap(QPixmap::fromImage(posterGray));

    if (m_copyPasteEnabled) {
        saveImageToSharedFolder(posterizedOrig, "posterized");
        saveImageToSharedFolder(posterGray, "posterized_grayscale");
    }
}

// Updated method to toggle posterization state
void MainWindow::onPosterizeButtonClicked(int levels)
{
    if (!m_originalPixmapItem || !m_grayscalePixmapItem) return;

    if (!m_isPosterized) {
        applyPosterization(levels);
        m_isPosterized = true;
    }
    else {
        // Revert to original images
        m_originalPixmapItem->setPixmap(m_originalPixmap);
        m_grayscalePixmapItem->setPixmap(m_grayscalePixmap);
        m_isPosterized = false;
    }
}

void MainWindow::onDegradeButtonClicked()
{
    if (!m_originalPixmapItem || !m_grayscalePixmapItem) return;

    if (!m_isBlurred) {
        // blur
        QImage orig = m_originalPixmapItem->pixmap().toImage();
        QImage gray = m_grayscalePixmapItem->pixmap().toImage();

        QImage blurredOrig = ImageUtils::gaussianBlur(orig, 5);
        QImage blurredGray = ImageUtils::gaussianBlur(gray, 5);

        m_originalPixmapItem->setPixmap(QPixmap::fromImage(blurredOrig));
        m_grayscalePixmapItem->setPixmap(QPixmap::fromImage(blurredGray));

        m_isBlurred = true;
    }
    else {
        // revert
        m_originalPixmapItem->setPixmap(m_originalPixmap);
        m_grayscalePixmapItem->setPixmap(m_grayscalePixmap);
        m_isBlurred = false;
    }
}

void MainWindow::onMedianFilterButtonClicked()
{
    if (!m_originalPixmapItem) return;

    if (!m_isMedianFiltered) {
        QImage orig = m_originalPixmapItem->pixmap().toImage();
        QImage filtered = ImageUtils::medianFilter(orig);
        m_originalPixmapItem->setPixmap(QPixmap::fromImage(filtered));
        m_isMedianFiltered = true;
    }
    else {
        // revert
        m_originalPixmapItem->setPixmap(m_originalPixmap);
        m_isMedianFiltered = false;
    }
}

void MainWindow::confirmAndMoveFileToDeleteFolder()
{
    if (m_currentImagePath.isEmpty()) {
        QMessageBox::information(this, "No Image", "No image is currently loaded.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        "Are you sure you want to delete this file?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString deleteFolder = "C:/REFERENCE/test/delete";
        QDir dir(deleteFolder);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        QFileInfo fi(m_currentImagePath);
        QString newFilePath = deleteFolder + "/" + fi.fileName();
        if (QFile::rename(m_currentImagePath, newFilePath)) {
            m_currentImagePath.clear();
            m_view->scene()->clear();
            loadImageFromDirectory(m_directory);
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to move the file.");
        }
    }
}

// ---------------  Scheduling  ---------------
void MainWindow::startSchedule()
{
    qDebug() << "Starting schedule...";
    // If you have multiple schedules, you want to cancel any existing normal timer
    // or unify them into one approach.
    isTimerRunning = false;            // Make sure the manual timer is considered off
    scheduleActive = true;

    if (!schedule.isEmpty()) {
        currentScheduleIndex = 0;
        qDebug() << "Schedule is not empty. Starting with the first item.";
        startNextTimerInSchedule();
    }
    else {
        qDebug() << "Schedule is empty. Nothing to start.";
    }
}

void MainWindow::startNextTimerInSchedule()
{
    if (currentScheduleIndex < schedule.size()) {
        qDebug() << "Starting next timer in schedule. Index:" << currentScheduleIndex;
        // 1) Set countdownTime to the next schedule item
        countdownTime = schedule[currentScheduleIndex];
        qDebug() << "Countdown time set to:" << countdownTime.toString("hh:mm:ss");

        // 2) Update m_timeButton text
        m_timeButton->setText(countdownTime.toString("hh:mm:ss"));
        qDebug() << "Time button text updated to:" << countdownTime.toString("hh:mm:ss");

        // 3) Start the m_countdownTimer if not already started
        //    or forcibly restart it
        //    We'll do forcibly restart to ensure it's counting from the new time
        m_countdownTimer->stop();  // Stop any old countdown
        m_countdownTimer->start(1000);
        qDebug() << "Countdown timer started.";

        // Mark isTimerRunning to true
        isTimerRunning = true;
        qDebug() << "Timer is now running.";
    }
    else {
        // All schedule items done
        qDebug() << "All schedule items are done.";
        scheduleActive = false;
        isTimerRunning = false;
        m_countdownTimer->stop();
        qDebug() << "Countdown timer stopped.";

        // Optionally reset the m_timeButton to something else
        m_timeButton->setText("Schedule Done");
        qDebug() << "Time button text set to 'Schedule Done'.";

        // **Loop the schedule by resetting the index and restarting**
        currentScheduleIndex = 0;
        scheduleActive = true;
        qDebug() << "Restarting schedule.";
        startNextTimerInSchedule();
    }
}


#include "scheduledialog.h"  // <--- Make sure this is included

void MainWindow::editSchedule()
{
    qDebug() << "Editing schedule...";
    // Pass the current 'schedule' to the dialog, so it can list it
    ScheduleDialog dialog(this, schedule);
    // Show the dialog modally
    if (dialog.exec() == QDialog::Accepted) {
        // If user pressed OK, retrieve the new schedule
        schedule = dialog.getSchedule();
        qDebug() << "New schedule accepted:" << schedule;
    }
    else {
        qDebug() << "Schedule editing canceled.";
    }
}


// ---------------  Loading Images  ---------------
void MainWindow::loadImageFromDirectory(const QString& directory)
{
    QString imagePath = getRandomImage(directory);
    if (imagePath.isEmpty()) {
        QMessageBox::information(this, "No Image Found", "No image found in directory");
        return;
    }
    processAndDisplayImage(imagePath);
}

void MainWindow::processAndDisplayImage(const QString& filePath)
{
    if (filePath.isEmpty()) return;
    m_currentImagePath = filePath;

    // (1) Load the cv::Mat
    cv::Mat mat = ImageUtils::loadAndApplyColorProfile(filePath);
    if (mat.empty()) {
        QMessageBox::warning(this, "Image Load Error", "Failed to load image: " + filePath);
        return;
    }

    // (2) Convert to QImage, then QPixmap
    cv::Mat resampled = ImageUtils::lanczosResizeIfNeeded(mat);
    QImage qimg = ImageUtils::convertMatToQImage(resampled);
    m_view->scene()->clear();  // Clear old items

    // (3) Create original pixmap
    QPixmap pixmap = QPixmap::fromImage(qimg);
    m_originalPixmap = pixmap;
    m_originalPixmapItem = new QGraphicsPixmapItem(pixmap);
    m_view->scene()->addItem(m_originalPixmapItem);

    // (4) Create grayscale pixmap
    QImage grayImg = ImageUtils::convertToGrayscale(qimg);
    m_grayscalePixmap = QPixmap::fromImage(grayImg);
    m_grayscalePixmapItem = new QGraphicsPixmapItem(m_grayscalePixmap);
    m_grayscalePixmapItem->setVisible(false);
    m_view->scene()->addItem(m_grayscalePixmapItem);

    // (5) Determine the image's bounding rectangle
    QRectF imageRect = m_originalPixmapItem->boundingRect();

    // (6) Create lines based on imageRect
    m_view->createAndAddLines(imageRect);
    m_view->setLinesVisibility(false); // Hide lines initially if desired

    // (7) Save the ruler image after scene setup and before setting large margins
    QString tempFilePath = QDir::tempPath() + "/ruler_images/ruler_image.png";
    m_view->saveRulerImage(tempFilePath);

    // (8) Set the scene rectangle to imageRect plus margins for panning
    qreal margin = 100000.0; // Adjust margin as needed
    QRectF biggerRect = imageRect.adjusted(-margin, -margin, margin, margin);
    m_view->scene()->setSceneRect(biggerRect);

    // (9) Reset transformations to start fresh
    m_view->resetTransform();

    // (10) Fit the image within the view while maintaining aspect ratio
    m_view->fitInView(imageRect, Qt::KeepAspectRatio);

    // (11) Center the view on the original pixmap item
    m_view->centerOn(m_originalPixmapItem);

    // (12) Start the countdown timer
    startTimerButton->click();

    // (13) Save the image to a shared folder if enabled
    if (m_copyPasteEnabled) {
        saveImageToSharedFolder(qimg, "suffix");
    }

    // (14) Reset effect toggles
    m_isBlurred = false;
    m_isMedianFiltered = false;
    m_isPosterized = false;
    m_isGrayscale = false;
}


void MainWindow::processClipboardImage()
{
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        processImage(image);
    }
    else if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            processAndDisplayImage(urls.first().toLocalFile());
        }
        else {
            QMessageBox::warning(this, "Error", "Clipboard does not contain a valid image path.");
        }
    }
    else {
        QMessageBox::warning(this, "Error", "No image found in clipboard.");
    }
}

void MainWindow::processImage(const QImage& image)
{
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Clipboard does not contain a valid image.");
        return;
    }
    // Save to temp + display
    QString tempImagePath = QDir::tempPath() + "/tempImage.png";
    if (image.save(tempImagePath)) {
        processAndDisplayImage(tempImagePath);
        if (m_copyPasteEnabled) {
            saveImageToSharedFolder(image, "suffix");
        }
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to save the clipboard image to a temporary file.");
    }
}

void MainWindow::saveImageToSharedFolder(const QImage& image, const QString& suffix)
{
    QString baseName = QFileInfo(m_currentImagePath).completeBaseName();
    QString folderPath = QString("C:/REFERENCE/Sharing/%1").arg(baseName);

    if (!m_folderCreationTimes.contains(folderPath)) {
        QDir().mkpath(folderPath);
        m_folderCreationTimes[folderPath] = QDateTime::currentDateTime();
        deleteOldestFolderIfNeeded();
    }
    QString savePath = QString("%1/%2_%3.png").arg(folderPath).arg(baseName).arg(suffix);
    if (!image.save(savePath)) {
        qDebug() << "Failed to save image to" << savePath;
    }
}

void MainWindow::deleteOldestFolderIfNeeded()
{
    const int maxFolders = 5;
    if (m_folderCreationTimes.size() > maxFolders) {
        QString oldestFolder;
        QDateTime oldestTime = QDateTime::currentDateTime();
        for (auto it = m_folderCreationTimes.begin(); it != m_folderCreationTimes.end(); ++it) {
            if (it.key().startsWith("C:/REFERENCE/Sharing") && it.value() < oldestTime) {
                oldestTime = it.value();
                oldestFolder = it.key();
            }
        }
        if (!oldestFolder.isEmpty()) {
            QDir dir(oldestFolder);
            if (dir.removeRecursively()) {
                m_folderCreationTimes.remove(oldestFolder);
                qDebug() << "Deleted oldest folder:" << oldestFolder;
            }
        }
    }
}
