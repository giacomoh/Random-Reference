#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGraphicsView>
#include <QTimeEdit>  // Add this line
#include <QSettings>  // Add this line

class ZoomableGraphicsView;  // Forward declaration

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    void startSchedule();
    void startNextTimerInSchedule();
    void editSchedule();
    QString getRandomImage(const QString& directory);
    int levels;  // Add this line
    QImage posterizeImage(const QImage& image, int levels);
    bool m_isBlurred = false;
    bool scheduleActive;  // Declare the scheduleActive member variable
    QGraphicsLineItem* m_horizontalLine;
    QGraphicsLineItem* m_verticalLine;
    void updateLines();

private slots:
    void onOpenButtonClicked();
    void onHistoryButtonClicked();
    void onNextButtonClicked();
    void onFlipButtonClicked();
    void onGrayscaleButtonClicked();  // Declare the onGrayscaleButtonClicked function
    void onPosterizeButtonClicked(int levels);
    void onDegradeButtonClicked();
    void onMedianFilterButtonClicked();


private:
    ZoomableGraphicsView* m_view;  // Declare the m_view member variable
    QGraphicsPixmapItem* m_originalPixmapItem;  // Declare the m_originalPixmapItem member variable
    QGraphicsPixmapItem* m_grayscalePixmapItem;  // Declare the m_grayscalePixmapItem member variable
    bool m_isGrayscale;  // Declare the m_isGrayscale member variable
    QFileInfoList m_files;  // Add this line
    int m_currentIndex = 0;  // Add this line
    void setDirectory(const QString& directory);  // Add this line
    QTime countdownTime;
    bool isTimerRunning;
    QTimer* timer;
    QPixmap m_originalPixmap;  // Store the original pixmap
    QPixmap m_grayscalePixmap;  // Store the grayscale pixmap
    bool m_isPosterized;  // Add this line
    QPushButton* startTimerButton;  // Add this line
    QStringList directoryHistory;
    bool m_isMedianFiltered;  // Add this line
    QList<QTime> schedule;
    int currentScheduleIndex;


private:
    void loadImageFromDirectory(const QString& directory);
    QList<QTimer*> timers;
    QSettings settings;

private:
    QString m_directory;

protected:
    void closeEvent(QCloseEvent* event) override;
};


#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTime>

class ScheduleDialog : public QDialog {
    Q_OBJECT

public:
    ScheduleDialog(QWidget* parent = nullptr, const QList<QTime>& schedule = QList<QTime>())
        : QDialog(parent), schedule(schedule) {
        QVBoxLayout* layout = new QVBoxLayout(this);

        // Create a QTimeEdit for time input
        QTimeEdit* timeEdit = new QTimeEdit(this);
        timeEdit->setDisplayFormat("hh:mm:ss");
        layout->addWidget(timeEdit);

        // Create a QListWidget to display the schedule
        listWidget = new QListWidget(this);  // Initialize the listWidget member variable
        layout->addWidget(listWidget);

        // Initialize the QListWidget with the schedule
        for (const QTime& time : schedule) {
            listWidget->addItem(time.toString("hh:mm:ss"));
        }

        // Create an "Add" button
        QPushButton* addButton = new QPushButton("Add", this);
        connect(addButton, &QPushButton::clicked, this, [=]() {
            QTime time = timeEdit->time();
            this->schedule.append(time);
            listWidget->addItem(time.toString("hh:mm:ss"));
            });

        layout->addWidget(addButton);

        QPushButton* removeButton = new QPushButton("Remove Timer", this);
        connect(removeButton, &QPushButton::clicked, this, &ScheduleDialog::removeTimer);
        layout->addWidget(removeButton);

        QPushButton* okButton = new QPushButton("OK", this);
        connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(okButton);

        setLayout(layout);
    }

    QList<QTime> getSchedule() const {
        QList<QTime> schedule;
        for (int i = 0; i < listWidget->count(); ++i) {
            schedule.append(QTime::fromString(listWidget->item(i)->text(), "hh:mm:ss"));
        }
        return schedule;
    }

private slots:
    void addTimer(QTime time) {
        // Add a new timer to the list widget
        listWidget->addItem(time.toString("hh:mm:ss"));

        // Log the addition of the timer
        qInfo() << "Added timer: " << QTime(0, 10).toString("hh:mm:ss");
        qInfo() << "Current schedule: " << getSchedule();
    }

    void removeTimer() {
        if (listWidget->currentRow() != -1) {
            // Remove the currently selected timer from the list widget
            QListWidgetItem* item = listWidget->takeItem(listWidget->currentRow());
            QString itemText = item->text();
            delete item;

            // Log the removal of the timer
            qInfo() << "Removed timer: " << itemText;
            qInfo() << "Current schedule: " << getSchedule();
        }
    }

private:
    QListWidget* listWidget;
    QList<QTime> schedule;
    QList<QTimer*> timers;
};

#endif // MAINWINDOW_H
