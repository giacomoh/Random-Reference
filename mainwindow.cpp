#include <opencv2/opencv.hpp>
#include "mainwindow.h"
#include <QRandomGenerator>
#include <QDir>
#include <QDirIterator>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSettings>
#include <opencv2/opencv.hpp>
#include <QImageReader>
#include <QTimer>
#include <QMessageBox>
#include <vector>
#include <cstdint>
#include <QTimeEdit>
#include <QLabel>
#include <QMenu>

#include <lcms2.h>
#include <windows.h>
#include <icm.h>

#include <MagickWand.h>
#include <QSpinBox>
#include <QGraphicsBlurEffect>
#include <QSplitter>

#include <QListWidget>

#include <QSettings>  // Add this line

class SelectAllSpinBox : public QSpinBox
{
public:
    SelectAllSpinBox(QWidget* parent = nullptr) : QSpinBox(parent)
    {
        setFocusPolicy(Qt::ClickFocus);  // Set the focus policy in the constructor
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        QSpinBox::mousePressEvent(event);
        this->selectAll();
    }
};

class ZoomableGraphicsView : public QGraphicsView
{
public:
    ZoomableGraphicsView(QGraphicsScene* scene) : QGraphicsView(scene)
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    QGraphicsLineItem* horizontalLine() const { return m_horizontalLine; }
    QGraphicsLineItem* verticalLine() const { return m_verticalLine; }

    void setHorizontalLine(QGraphicsLineItem* line) { m_horizontalLine = line; }
    void setVerticalLine(QGraphicsLineItem* line) { m_verticalLine = line; }


protected:
    void wheelEvent(QWheelEvent* event) override
    {
        // Zoom Factor
        double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            // Zoom in
            QPointF scenePoint = mapToScene(event->position().toPoint());
            QGraphicsItem* item = scene()->itemAt(scenePoint, transform());
            if (item) {
                setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            }
            else {
                setTransformationAnchor(QGraphicsView::AnchorViewCenter);
            }
            scale(scaleFactor, scaleFactor);
        }
        else {
            // Zooming out
            setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            scale(1 / scaleFactor, 1 / scaleFactor);
        }
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_lastPanPoint = event->pos();
            if (event->modifiers() == Qt::ControlModifier) {
                QPen pen(Qt::red, 2); // Set color to red and width to 2
                QPointF scenePoint = mapToScene(m_lastPanPoint);
                if (!m_horizontalLine) {
                    m_horizontalLine = new QGraphicsLineItem(scenePoint.x(), 0, scenePoint.x(), scene()->height());
                    m_horizontalLine->setPen(pen);
                    scene()->addItem(m_horizontalLine);
                }
                else {
                    m_horizontalLine->setLine(scenePoint.x(), 0, scenePoint.x(), scene()->height());
                }
                if (!m_verticalLine) {
                    m_verticalLine = new QGraphicsLineItem(0, scenePoint.y(), scene()->width(), scenePoint.y());
                    m_verticalLine->setPen(pen);
                    scene()->addItem(m_verticalLine);
                }
                else {
                    m_verticalLine->setLine(0, scenePoint.y(), scene()->width(), scenePoint.y());
                }
            }
        }
        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (!m_lastPanPoint.isNull()) {
            if (m_horizontalLine && m_verticalLine && (event->modifiers() & Qt::ControlModifier)) {
                QPointF scenePoint = mapToScene(event->pos());
                m_horizontalLine->setLine(scenePoint.x(), 0, scenePoint.x(), scene()->height());
                m_verticalLine->setLine(0, scenePoint.y(), scene()->width(), scenePoint.y());
            }
            else {
                // Get how much we panned
                QPointF delta = mapToScene(m_lastPanPoint) - mapToScene(event->pos());
                m_lastPanPoint = event->pos();

                // Update the center (i.e., adjust the view)
                setSceneRect(sceneRect().translated(delta.x(), delta.y()));
            }
        }
        // Only call the base class implementation if Control is not pressed
        if (!(event->modifiers() & Qt::ControlModifier)) {
            QGraphicsView::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_lastPanPoint = QPoint();
            setCursor(Qt::ArrowCursor);
        }
        QGraphicsView::mouseReleaseEvent(event);
    }


private:
    QGraphicsLineItem* m_horizontalLine = nullptr;
    QGraphicsLineItem* m_verticalLine = nullptr;
    QPoint m_lastPanPoint;

};


MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
    m_isGrayscale(false),  // Initialize m_isGrayscale to false
    m_isPosterized(false),  // Initialize m_isPosterized to false
    m_grayscalePixmapItem(nullptr),  // Initialize m_grayscalePixmapItem to nullptr
    m_originalPixmapItem(nullptr),  // Initialize m_originalPixmapItem to nullptr
    countdownTime(QTime(0, 10)), // Initialize countdownTime to 00:10:00
    timer(new QTimer(this)),
    levels(4), // Initialize levels to 4
    isTimerRunning(false), // Initialize isTimerRunning to false
    startTimerButton(new QPushButton("Start")),// Initialize startTimerButton
    m_isMedianFiltered(false), // Initialize m_isMedianFiltered to false
    currentScheduleIndex(0),
    scheduleActive(false),  // Initialize scheduleActive to false
    m_horizontalLine(nullptr),
    m_verticalLine(nullptr)

{
    // Initialize m_view and its scene
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view = new ZoomableGraphicsView(scene);

    // Now you can use m_view and its scene
    // Initialize the lines
    m_horizontalLine = new QGraphicsLineItem(0, m_view->scene()->height() / 2, m_view->scene()->width(), m_view->scene()->height() / 2);
    m_verticalLine = new QGraphicsLineItem(m_view->scene()->width() / 2, 0, m_view->scene()->width() / 2, m_view->scene()->height());

    // Add the lines to the scene
    m_view->scene()->addItem(m_horizontalLine);
    m_view->scene()->addItem(m_verticalLine);

    // Initially hide the lines
    m_horizontalLine->setVisible(false);
    m_verticalLine->setVisible(false);



    qDebug() << "Final loaded schedule:" << schedule;

    // Make the window always stay on top
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);


    // Create two QHBoxLayouts for two rows of buttons and spinboxes
    QHBoxLayout* buttonLayout1 = new QHBoxLayout;
    QHBoxLayout* buttonLayout2 = new QHBoxLayout;


    // Add buttons and spinboxes to the first row
    QPushButton* openButton = new QPushButton("Open");
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenButtonClicked);
    buttonLayout1->addWidget(openButton);

    QPushButton* historyButton = new QPushButton("History");
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::onHistoryButtonClicked);
    buttonLayout1->addWidget(historyButton);

    QPushButton* nextButton = new QPushButton("Next");
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextButtonClicked);
    buttonLayout1->addWidget(nextButton);

    QPushButton* flipButton = new QPushButton("Flip");
    connect(flipButton, &QPushButton::clicked, this, &MainWindow::onFlipButtonClicked);
    buttonLayout1->addWidget(flipButton);

    QPushButton* grayscaleButton = new QPushButton("Grayscale");
    connect(grayscaleButton, &QPushButton::clicked, this, &MainWindow::onGrayscaleButtonClicked);
    buttonLayout1->addWidget(grayscaleButton);


    // Add buttons and spinboxes to the second row

    // Add a button to start the schedule
    QPushButton* startScheduleButton = new QPushButton("Start Schedule");
    connect(startScheduleButton, &QPushButton::clicked, this, &MainWindow::startSchedule);
    buttonLayout2->addWidget(startScheduleButton);

    // Add a button to edit the schedule
    QPushButton* editScheduleButton = new QPushButton("Edit Schedule");
    connect(editScheduleButton, &QPushButton::clicked, this, &MainWindow::editSchedule);
    buttonLayout2->addWidget(editScheduleButton);

        // Add a button to rotate the image view clockwise
    QPushButton* rotateClockwiseButton = new QPushButton("<-");
    connect(rotateClockwiseButton, &QPushButton::clicked, this, [this]() {
        // Assuming m_graphicsView is your QGraphicsView object
        m_view->rotate(10);
        });
    buttonLayout2->addWidget(rotateClockwiseButton);

    // Add a button to rotate the image view counterclockwise
    QPushButton* rotateCounterclockwiseButton = new QPushButton("->");
    connect(rotateCounterclockwiseButton, &QPushButton::clicked, this, [this]() {
        // Assuming m_graphicsView is your QGraphicsView object
        m_view->rotate(-10);
        });
    buttonLayout2->addWidget(rotateCounterclockwiseButton);


    QPushButton* posterizeButton = new QPushButton("Posterize");
    buttonLayout2->addWidget(posterizeButton);

    QSpinBox* levelsSpinBox = new QSpinBox;
    levelsSpinBox->setRange(2, 99);
    levelsSpinBox->setValue(4);
    buttonLayout2->addWidget(levelsSpinBox);

    // Connect the spin box valueChanged signal to a lambda function that updates the levels variable
    connect(levelsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        this->levels = value;  // Make sure to use the levels member variable of the MainWindow class
        qDebug() << "Updated levels: " << this->levels;
        });

    // Modify the posterizeButton clicked connection
    connect(posterizeButton, &QPushButton::clicked, this, [this]() {
        onPosterizeButtonClicked(this->levels);  // Make sure to use the levels member variable of the MainWindow class
        qDebug() << "Updated levels: " << this->levels;
        });

    QPushButton* blurButton = new QPushButton("Degrade");
    connect(blurButton, &QPushButton::clicked, this, &MainWindow::onDegradeButtonClicked);
    buttonLayout2->addWidget(blurButton);

    QPushButton* medianFilterButton = new QPushButton("Median");
    connect(medianFilterButton, &QPushButton::clicked, this, &MainWindow::onMedianFilterButtonClicked);
    buttonLayout2->addWidget(medianFilterButton);

    QPushButton* toggleLinesButton = new QPushButton("Toggle Lines");
    buttonLayout2->addWidget(toggleLinesButton);
    connect(toggleLinesButton, &QPushButton::clicked, this, [this]() {
        bool isVisible = m_view->horizontalLine()->isVisible();
        m_view->horizontalLine()->setVisible(!isVisible);
        m_view->verticalLine()->setVisible(!isVisible);
        });

    // Add both QHBoxLayouts to the main QVBoxLayout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout1);
    layout->addLayout(buttonLayout2);

   

    QPushButton* timeButton = new QPushButton(this);
    timeButton->setText(QTime(0, 10).toString("hh:mm:ss"));  // Set initial text
    buttonLayout1->addWidget(timeButton);

    // Create spin boxes for hours, minutes, and seconds
    SelectAllSpinBox* hoursSpinBox = new SelectAllSpinBox;
    hoursSpinBox->setRange(0, 23);  // Set the range of the spin box to fit hours
    hoursSpinBox->setValue(0);  // Set the initial value
    buttonLayout1->addWidget(hoursSpinBox);

    SelectAllSpinBox* minutesSpinBox = new SelectAllSpinBox;
    minutesSpinBox->setRange(0, 59);  // Set the range of the spin box to fit minutes
    minutesSpinBox->setValue(10);  // Set the initial value
    buttonLayout1->addWidget(minutesSpinBox);

    SelectAllSpinBox* secondsSpinBox = new SelectAllSpinBox;
    secondsSpinBox->setRange(0, 59);  // Set the range of the spin box to fit seconds
    secondsSpinBox->setValue(0);  // Set the initial value
    buttonLayout1->addWidget(secondsSpinBox);

    QTimer* timer = new QTimer(this);
    bool isTimerRunning = true;  // Add a flag to track the timer state

    connect(timer, &QTimer::timeout, this, [this, timer, timeButton, &isTimerRunning, hoursSpinBox, minutesSpinBox, secondsSpinBox]() {
        this->countdownTime = this->countdownTime.addSecs(-1);
        qDebug() << "Updated countdown time: " << this->countdownTime.toString("hh : mm : ss");
        QString timeText = this->countdownTime.toString("h : mm : ss");
        if (this->countdownTime.hour() == 0) {
            timeText = this->countdownTime.toString("m : ss");
        }
        timeButton->setText(timeText);
        if (this->countdownTime == QTime(0, 0)) {
            this->loadImageFromDirectory(m_directory);  // Load a new image from the directory
            if (scheduleActive) {
                if (currentScheduleIndex < schedule.size()) {
                    countdownTime = schedule[currentScheduleIndex++];
                    // Update the timeButton text immediately after setting the countdownTime
                    QString timeText = this->countdownTime.toString("h : mm : ss");
                    if (this->countdownTime.hour() == 0) {
                        timeText = this->countdownTime.toString("m : ss");
                    }
                    timeButton->setText(timeText);
                }
                else {
                    // No more times in the schedule, stop the timer
                    timer->stop();
                    isTimerRunning = false;
                    scheduleActive = false;  // Set scheduleActive to false when the schedule is exhausted
                }
            }
        }
        });


    connect(timeButton, &QPushButton::clicked, this, [this, timer, timeButton]() {
        if (this->isTimerRunning) {
            timer->stop();
            timeButton->setStyleSheet("color: red");
        }
        else {
            timer->start(1000);
            timeButton->setStyleSheet("color: black");
        }
        this->isTimerRunning = !this->isTimerRunning;
        });

    startTimerButton->setText("Start");
    connect(startTimerButton, &QPushButton::clicked, this, [this, timer, timeButton, hoursSpinBox, minutesSpinBox, secondsSpinBox]() {
        this->countdownTime.setHMS(hoursSpinBox->value(), minutesSpinBox->value(), secondsSpinBox->value());
        qDebug() << "Countdown time: " << this->countdownTime.toString("hh:mm:ss");

        // Update the timeButton text immediately after setting the countdownTime
        QString timeText = this->countdownTime.toString("h : mm : ss");
        if (this->countdownTime.hour() == 0) {
            timeText = this->countdownTime.toString("m : ss");
        }
        timeButton->setText(timeText);

        QPalette palette = timeButton->palette();
        palette.setColor(QPalette::ButtonText, Qt::black);
        timeButton->setPalette(palette);
        });
    buttonLayout1->addWidget(startTimerButton);


    // Add a button to buttonLayout1 that toggles the visibility of buttonLayout2
    QPushButton* toggleButton = new QPushButton("Toggle");
    connect(toggleButton, &QPushButton::clicked, this, [buttonLayout2]() {
        bool isVisible = buttonLayout2->itemAt(0)->widget()->isVisible(); // Check the visibility of the first widget
        for (int i = 0; i < buttonLayout2->count(); ++i) {
            QWidget* widget = buttonLayout2->itemAt(i)->widget();
            if (widget) {
                widget->setVisible(!isVisible);
            }
        }
        });
    buttonLayout1->addWidget(toggleButton);

    // Create a QFont object
    QFont font;
    font.setPointSize(12); // Set the font size to 14

    // Create a QFont object for the timer
    QFont timerFont;
    timerFont.setPointSize(12); // Set the font size to 12
    timerFont.setBold(true); // Set the font weight to bold


    // Set the font for the labels
    openButton->setFont(font);
    nextButton->setFont(font);
    flipButton->setFont(font);
    grayscaleButton->setFont(font);
    startTimerButton->setFont(font);
    posterizeButton->setFont(font);
    blurButton->setFont(font);
    historyButton->setFont(font);
    medianFilterButton->setFont(font);
    toggleButton->setFont(font);
    rotateClockwiseButton->setFont(font);
    rotateCounterclockwiseButton->setFont(font);
    startScheduleButton->setFont(font);
    editScheduleButton->setFont(font);
	toggleLinesButton->setFont(font);

    // Set the font for the timer
    timeButton->setFont(timerFont);

    // Set the font for the spin boxes
    hoursSpinBox->setFont(font);
    minutesSpinBox->setFont(font);
    secondsSpinBox->setFont(font);
    levelsSpinBox->setFont(font);

    // Set the stylesheet for the spin boxes
    QString spinBoxStyle = "QSpinBox { background-color: #d3d3d3; color: #000000; border-radius: 5px; padding: 5px; } QSpinBox:hover { background-color: #c0c0c0; }";
    hoursSpinBox->setStyleSheet(spinBoxStyle);
    minutesSpinBox->setStyleSheet(spinBoxStyle);
    secondsSpinBox->setStyleSheet(spinBoxStyle);
    levelsSpinBox->setStyleSheet(spinBoxStyle);


    // Set the size policy for the buttons
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    openButton->setSizePolicy(sizePolicy);
    nextButton->setSizePolicy(sizePolicy);
    flipButton->setSizePolicy(sizePolicy);
    grayscaleButton->setSizePolicy(sizePolicy);
    posterizeButton->setSizePolicy(sizePolicy);
    blurButton->setSizePolicy(sizePolicy);
    startTimerButton->setSizePolicy(sizePolicy);
    timeButton->setSizePolicy(sizePolicy);
    startScheduleButton->setSizePolicy(sizePolicy);
    editScheduleButton->setSizePolicy(sizePolicy);

    // Set the size policy for the spin boxes
    hoursSpinBox->setSizePolicy(sizePolicy);
    minutesSpinBox->setSizePolicy(sizePolicy);
    secondsSpinBox->setSizePolicy(sizePolicy);
    levelsSpinBox->setSizePolicy(sizePolicy);

    // Create a ZoomableGraphicsView and add it to the layout
    m_view = new ZoomableGraphicsView(new QGraphicsScene(this));
    m_view->setBackgroundBrush(QColor(128, 128, 128));  // Set the background to 50% grey
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);  // Enable smooth pixmap transformation
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);  // Enable panning
    m_view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    layout->addWidget(m_view);

    setLayout(layout);

    // Restore the size and position of the window
    QSettings settings("YourOrganization", "YourApplication");
    QStringList scheduleStringList = settings.value("schedule").toStringList();
    for (const QString& timeString : scheduleStringList) {
        schedule.append(QTime::fromString(timeString));
    }
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    m_directory = settings.value("lastDirectory").toString();

    // Load the history from QSettings
    directoryHistory = settings.value("directoryHistory").toStringList();

    if (!m_directory.isEmpty()) {
        setDirectory(m_directory);  // Set the directory
        QTimer::singleShot(100, this, [this]() { loadImageFromDirectory(m_directory); });
    }



}

void MainWindow::startSchedule() {
    // Start the schedule of timers here
    // You could use a QTimer for each item in the schedule, and connect the timeout signal to a slot that starts the next timer
    scheduleActive = true;
    if (!schedule.empty()) {
        currentScheduleIndex = 0;
        startNextTimerInSchedule();
    }
}

void MainWindow::startNextTimerInSchedule() {
    if (currentScheduleIndex < schedule.size()) {
        countdownTime = schedule[currentScheduleIndex];
        // Convert the QTime to milliseconds
        int interval = QTime(0, 0).msecsTo(countdownTime);
        QTimer* newTimer = new QTimer(this);
        timers.append(newTimer);  // Add the new timer to the list
        connect(newTimer, &QTimer::timeout, this, [this, newTimer]() {
            // Handle the timeout signal here
            // ...
            newTimer->deleteLater();  // Delete the timer when it's finished
        });
        newTimer->start(interval);  // Start the timer with the calculated interval
        isTimerRunning = true;
        currentScheduleIndex++;
    }
}

void MainWindow::editSchedule() {
    ScheduleDialog dialog(this, schedule);  // Pass the schedule to the dialog
    if (dialog.exec() == QDialog::Accepted) {
        schedule = dialog.getSchedule();  // Update the schedule when the dialog is accepted
    }
}






void MainWindow::setDirectory(const QString& directory)
{
    m_directory = directory;

    // Remove the directory if it's already in the history
    directoryHistory.removeAll(directory);

    // Prepend the directory to the history
    directoryHistory.prepend(directory);

    // If the history size exceeds 10, remove the last item
    if (directoryHistory.size() > 10) {
        directoryHistory.removeLast();
    }

    // Save the directory and history to QSettings
    QSettings settings("YourOrganization", "YourApplication");
    settings.setValue("lastDirectory", m_directory);
    settings.setValue("directoryHistory", directoryHistory);


    // Populate the list of files
    m_files.clear();
    QDirIterator it(directory, QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG" << "*.bmp" << "*.BMP" << "*.gif" << "*.webp", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        m_files << it.fileInfo();
    }

    // Shuffle the list of files
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_files.begin(), m_files.end(), g);

    m_currentIndex = 0;
}

QString MainWindow::getRandomImage(const QString& directory)
{
    // If all images have been selected, shuffle the list of files and start over
    if (m_currentIndex >= m_files.size()) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_files.begin(), m_files.end(), g);
        m_currentIndex = 0;
    }

    // Check if m_files is empty before trying to access an element
    if (m_files.isEmpty()) {
        return QString();
    }

    return m_files[m_currentIndex++].filePath();
}


void MainWindow::onGrayscaleButtonClicked()
{
    if (m_view->scene()->items().isEmpty()) {
        qDebug() << "Scene is empty";
        return;
    }

    // Toggle the visibility of the original and grayscale pixmaps
    bool isGrayscale = m_grayscalePixmapItem->isVisible();
    m_originalPixmapItem->setVisible(isGrayscale);
    m_grayscalePixmapItem->setVisible(!isGrayscale);

    if (m_grayscalePixmapItem->isVisible()) {
        qDebug() << "Grayscale image is visible";
    }
    else {
        qDebug() << "Grayscale image is not visible";
    }
}


void MainWindow::onPosterizeButtonClicked(int levels)
{
    if (m_view->scene()->items().isEmpty()) {
        return;
    }

    if (!m_isPosterized) {
        // Posterize the original image
        QImage originalImage = m_originalPixmapItem->pixmap().toImage();
        QImage posterizedOriginalImage = posterizeImage(originalImage, levels);
        m_originalPixmapItem->setPixmap(QPixmap::fromImage(posterizedOriginalImage));

        // Posterize the grayscale image
        QImage grayscaleImage = m_grayscalePixmapItem->pixmap().toImage();
        QImage posterizedGrayscaleImage = posterizeImage(grayscaleImage, levels);
        m_grayscalePixmapItem->setPixmap(QPixmap::fromImage(posterizedGrayscaleImage));

        m_isPosterized = true;
    }
    else {
        // Revert back to the original and grayscale images
        m_originalPixmapItem->setPixmap(m_originalPixmap);
        m_grayscalePixmapItem->setPixmap(m_grayscalePixmap);
        m_isPosterized = false;
    }
}


void MainWindow::onMedianFilterButtonClicked()
{
    if (m_originalPixmapItem) {
        if (!m_isMedianFiltered) {
            QImage qimg = m_originalPixmapItem->pixmap().toImage().convertToFormat(QImage::Format_RGB888);
            cv::Mat mat(qimg.height(), qimg.width(), CV_8UC3, (uchar*)qimg.bits(), qimg.bytesPerLine());

            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

            // Define the kernel size based on the image size
            int kernelSize;
            if (qimg.width() < 1000 && qimg.height() < 1000) {
                kernelSize = 1;
            }
            else if (qimg.width() < 2000 && qimg.height() < 2000) {
                kernelSize = 3;
            }
            else {
                kernelSize = 5;
            }

            cv::medianBlur(mat, mat, kernelSize);  // Apply the median filter
            cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);

            QImage result((const unsigned char*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(result);

            m_originalPixmapItem->setPixmap(pixmap);

            m_isMedianFiltered = true;
        }
        else {
            // Revert back to the original image
            m_originalPixmapItem->setPixmap(m_originalPixmap);
            m_isMedianFiltered = false;
        }
    }
}



void MainWindow::onDegradeButtonClicked()
{
    if (m_view->scene()->items().isEmpty()) {
        qDebug() << "Scene is empty";
        return;
    }

    if (!m_isBlurred) {
        // Create deep copies of the original and grayscale images
        QImage originalImage = m_originalPixmapItem->pixmap().toImage().copy();
        QImage grayscaleImage = m_grayscalePixmapItem->pixmap().toImage().copy();

        // Check if the images are larger than 1000x1000 pixels and reduce their size by half if they are
        if (originalImage.width() > 1500 || originalImage.height() > 1500) {
            originalImage = originalImage.scaled(originalImage.width() / 2, originalImage.height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if (grayscaleImage.width() > 1500 || grayscaleImage.height() > 1500) {
            grayscaleImage = grayscaleImage.scaled(grayscaleImage.width() / 2, grayscaleImage.height() / 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // Convert the copied images to cv::Mat
        cv::Mat originalMat = cv::Mat(originalImage.height(), originalImage.width(), CV_8UC4, originalImage.bits(), static_cast<size_t>(originalImage.bytesPerLine()));
        cv::Mat grayscaleMat = cv::Mat(grayscaleImage.height(), grayscaleImage.width(), CV_8UC4, grayscaleImage.bits(), static_cast<size_t>(grayscaleImage.bytesPerLine()));

        // Apply a Gaussian blur to the original and grayscale images
        cv::GaussianBlur(originalMat, originalMat, cv::Size(0, 0), 5);
        cv::GaussianBlur(grayscaleMat, grayscaleMat, cv::Size(0, 0), 5);

        // Convert the cv::Mat images back to QImage
        QImage blurredOriginalImage = QImage(originalMat.data, originalMat.cols, originalMat.rows, originalMat.step, QImage::Format_ARGB32);
        QImage blurredGrayscaleImage = QImage(grayscaleMat.data, grayscaleMat.cols, grayscaleMat.rows, grayscaleMat.step, QImage::Format_ARGB32);

        // Set the pixmaps of the QGraphicsPixmapItems to the blurred images
        m_originalPixmapItem->setPixmap(QPixmap::fromImage(blurredOriginalImage));
        m_grayscalePixmapItem->setPixmap(QPixmap::fromImage(blurredGrayscaleImage));

        m_isBlurred = true;
    }
    else {
        // Revert back to the original and grayscale images
        m_originalPixmapItem->setPixmap(m_originalPixmap);
        m_grayscalePixmapItem->setPixmap(m_grayscalePixmap);
        m_isBlurred = false;
    }
}

QImage MainWindow::posterizeImage(const QImage& image, int levels)
{
    QImage reducedImage = image;

    // Only reduce the image if its width or height is greater than 2000 pixels
    if (image.width() > 2000 || image.height() > 2000) {
        reducedImage = image.scaled(image.width() * 0.5, image.height() * 0.5, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    // Posterize the reduced image
    int levelSize = 256 / levels;
    QRgb* pixelData = reinterpret_cast<QRgb*>(reducedImage.bits());
    int pixelCount = reducedImage.width() * reducedImage.height();

    for (int i = 0; i < pixelCount; ++i) {
        QColor color(pixelData[i]);
        int h = color.hue();
        int s = color.saturation();
        int v = color.value();
        v = qMin(qRound((double)v / levelSize) * levelSize, 255);
        color.setHsv(h, s, v);
        pixelData[i] = color.rgb();
    }

    // Resize the posterized image back to its original size
    reducedImage = reducedImage.scaled(image.width(), image.height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);

    return reducedImage;
}

void MainWindow::onOpenButtonClicked() {
    QString directory = QFileDialog::getExistingDirectory();
    if (!directory.isEmpty()) {
        directoryHistory.append(directory);
        setDirectory(directory);
        loadImageFromDirectory(directory);
    }
}

void MainWindow::onHistoryButtonClicked() {
    QMenu menu;

    // Add each directory in the history to the menu
    for (const QString& directory : directoryHistory) {
        menu.addAction(directory, this, [this, directory]() {
            // Slot for handling when a directory is clicked in the menu
            setDirectory(directory);
            loadImageFromDirectory(directory);
            });
    }

    // Show the menu
    menu.exec(QCursor::pos());
}

void MainWindow::onNextButtonClicked()
{
    if (!m_directory.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirmation", "Are you sure you want to load the next image?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            loadImageFromDirectory(m_directory);
        }
    }
}

void MainWindow::onFlipButtonClicked()
{
    if (m_view->scene()->items().isEmpty()) {
        return;
    }

    // Flip the original pixmap
    QImage originalImage = m_originalPixmapItem->pixmap().toImage();
    originalImage = originalImage.mirrored(true, false);
    QPixmap originalPixmap = QPixmap::fromImage(originalImage);
    m_originalPixmapItem->setPixmap(originalPixmap);

    // Flip the grayscale pixmap
    QImage grayscaleImage = m_grayscalePixmapItem->pixmap().toImage();
    grayscaleImage = grayscaleImage.mirrored(true, false);
    QPixmap grayscalePixmap = QPixmap::fromImage(grayscaleImage);
    m_grayscalePixmapItem->setPixmap(grayscalePixmap);

    // Get the center of the view in scene coordinates
    QPointF viewCenterSceneCoords = m_view->mapToScene(m_view->viewport()->rect().center());

    // Convert the view center to item coordinates
    QPointF viewCenterItemCoords = m_originalPixmapItem->mapFromScene(viewCenterSceneCoords);

    // Calculate the new position of the view center in the flipped image
    QPointF newViewCenterItemCoords(originalPixmap.width() - viewCenterItemCoords.x(), viewCenterItemCoords.y());

    // Convert the new view center to scene coordinates
    QPointF newViewCenterSceneCoords = m_originalPixmapItem->mapToScene(newViewCenterItemCoords.toPoint());

    // Center the view on the new position
    m_view->centerOn(newViewCenterSceneCoords);
}

void MainWindow::loadImageFromDirectory(const QString& directory)
{
    qDebug() << "Loading image from directory: " << directory;

    QString imagePath = getRandomImage(directory); // Use getRandomImage to select a random image

    if (imagePath.isEmpty()) {
        QMessageBox::information(this, tr("No Image Found"), tr("No image found in directory"));
        return;
    }

    qDebug() << "Selected image file: " << imagePath;

    MagickWand* magickWand;
    MagickWandGenesis();
    magickWand = NewMagickWand();
    qDebug() << "Initialized MagickWand";

    if (MagickReadImage(magickWand, imagePath.toLocal8Bit().constData()) == MagickFalse) {
        qDebug() << "Failed to read image";
        QMessageBox::critical(this, tr("Image Load Error"), tr("Failed to load image: ") + imagePath);
        DestroyMagickWand(magickWand);
        MagickWandTerminus();
        return;
    }
    qDebug() << "Image read successfully";

    // Get the color profile from the image
    size_t length;
    unsigned char* profile = MagickGetImageProfile(magickWand, "ICC", &length);
    qDebug() << "Retrieved color profile from image";

    cv::Mat img;  // Define img here

    if (profile != NULL) {
        qDebug() << "Color profile exists";
        // Create a LittleCMS color profile
        cmsHPROFILE hInProfile = cmsOpenProfileFromMem(profile, length);
        qDebug() << "Created LittleCMS color profile";

        // Create a LittleCMS transform
        cmsHPROFILE hOutProfile = cmsCreate_sRGBProfile();
        cmsHTRANSFORM hTransform = cmsCreateTransform(hInProfile, TYPE_BGR_8, hOutProfile, TYPE_BGR_8, INTENT_PERCEPTUAL, 0);
        qDebug() << "Created LittleCMS transform";

        // Open the image
        img = cv::imread(imagePath.toStdString());
        if (img.empty()) {
            qDebug() << "Failed to open image";
            return;
        }
        qDebug() << "Opened image successfully";

        // Apply the color profile to the image data
        cmsDoTransform(hTransform, img.data, img.data, img.total());
        qDebug() << "Applied color profile to image data";

        // Clean up
        cmsDeleteTransform(hTransform);
        cmsCloseProfile(hInProfile);
        cmsCloseProfile(hOutProfile);
        qDebug() << "Cleaned up LittleCMS resources";
    }
    else {
        qDebug() << "Color profile does not exist";
        img = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
        if (img.empty()) {
            qDebug() << "Failed to open image";
            return;
        }
        qDebug() << "Opened image successfully";
    }

    // Apply the Lanczos resampling algorithm
    cv::Mat resampled;
    // Check the size of the image
    if (img.cols < 2000 && img.rows < 2000) {
        // If the image is smaller than 2000x2000, make it twice as big
        cv::resize(img, resampled, cv::Size(img.cols * 2, img.rows * 2), 0, 0, cv::INTER_LANCZOS4);
        qDebug() << "Applied Lanczos resampling algorithm and resized image";
    }
    else {
        // If the image is already 2000x2000 or larger, apply Lanczos but keep the same size
        cv::resize(img, resampled, cv::Size(img.cols, img.rows), 0, 0, cv::INTER_LANCZOS4);
        qDebug() << "Applied Lanczos resampling algorithm without resizing";
    }

    // Convert the resampled image to RGB
    cv::cvtColor(resampled, resampled, cv::COLOR_BGR2RGB);
    qDebug() << "Converted image to RGB";

    QImage qImage((uchar*)resampled.data, resampled.cols, resampled.rows, resampled.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qImage);
    m_originalPixmapItem = new QGraphicsPixmapItem(pixmap);

    // Store the original pixmap
    m_originalPixmap = pixmap;


    qDebug() << "Created original QPixmap and QGraphicsPixmapItem";

    m_view->scene()->addPixmap(pixmap);

    // Update the scene's bounding rectangle
    m_view->scene()->setSceneRect(pixmap.rect());

    // Reset the view's scroll position
    m_view->setSceneRect(m_view->scene()->sceneRect());

    // Adjust the view's scale to fit the new image
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);

    // Convert the QImage to grayscale using the luminosity method
    QImage grayscaleImage = qImage.copy();
    for (int y = 0; y < grayscaleImage.height(); y++) {
        for (int x = 0; x < grayscaleImage.width(); x++) {
            QColor pixelColor = grayscaleImage.pixelColor(x, y);
            int gray = qRound(0.299 * pixelColor.red() + 0.587 * pixelColor.green() + 0.114 * pixelColor.blue());
            grayscaleImage.setPixelColor(x, y, QColor(gray, gray, gray));
        }
    }

    QPixmap grayscalePixmap = QPixmap::fromImage(grayscaleImage);
    m_grayscalePixmapItem = new QGraphicsPixmapItem(grayscalePixmap);

    // Store the grayscale pixmap
    m_grayscalePixmap = grayscalePixmap;

    // Add the original pixmap to the scene of the ZoomableGraphicsView
    m_view->scene()->clear();
    m_view->scene()->addItem(m_originalPixmapItem);
    m_view->scene()->addItem(m_grayscalePixmapItem);
    m_grayscalePixmapItem->setVisible(false);  // Hide the grayscale pixmap by default
    qDebug() << "Added pixmaps to scene";

    // Set the scene rectangle to the bounding rectangle of the pixmap
    m_view->scene()->setSceneRect(m_originalPixmapItem->boundingRect());
    qDebug() << "Set scene rectangle";

    // Scale the view to ensure that the entire scene rectangle fits within the view's viewport
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);
    qDebug() << "Scaled view to fit scene rectangle";

    DestroyMagickWand(magickWand);
    MagickWandTerminus();
    qDebug() << "Cleaned up MagickWand";

    // Reset the grayscale filter when a new image is loaded
    m_isGrayscale = false;

    // Reset the posterize filter when a new image is loaded
    m_isPosterized = false;

    // At the end of the loadImageFromDirectory method
    startTimerButton->click();

    m_isBlurred = false;

    m_isMedianFiltered = false;


    // Recreate the lines
    QPen pen(Qt::red, 2); // Set color to red and width to 2
    m_view->setHorizontalLine(new QGraphicsLineItem(0, m_view->scene()->height() / 2, m_view->scene()->width(), m_view->scene()->height() / 2));

    m_view->horizontalLine()->setPen(pen);
    m_view->scene()->addItem(m_view->horizontalLine());

    m_view->setVerticalLine(new QGraphicsLineItem(m_view->scene()->width() / 2, 0, m_view->scene()->width() / 2, m_view->scene()->height()));

    m_view->verticalLine()->setPen(pen);
    m_view->scene()->addItem(m_view->verticalLine());

    // Your existing code to handle the rest of the function...
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    // Convert the schedule to a QStringList
    QStringList scheduleStringList;
    for (const QTime& time : schedule) {
        scheduleStringList.append(time.toString());
    }

    // Save the schedule to QSettings
    QSettings settings("YourOrganization", "YourApplication");
    settings.setValue("schedule", scheduleStringList);

    // Save the size and position of the window
    settings.setValue("mainWindowGeometry", saveGeometry());

    // Save the history to QSettings
    settings.setValue("directoryHistory", directoryHistory);

    QWidget::closeEvent(event);
}
