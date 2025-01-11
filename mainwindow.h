#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QFileInfoList>
#include <QList>
#include <QCloseEvent>
#include <QMap>
#include <QDateTime>
#include <QSettings>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QCheckBox>
#include <QPushButton>
#include <qguiapplication.h>

class ZoomableGraphicsView;  // forward declaration
class ScheduleDialog;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();   // destructor

    // Enum for actions
    enum class Action {
        OpenDirectory,
        History,
        NextImage,
        Flip,
        Grayscale,
        StartSchedule,
        EditSchedule,
        RotateClockwise,
        RotateCounterclockwise,
        Posterize,
        Blur,
        MedianFilter,
        ToggleLines,
        SaveRuler,
        CopyDisplayedImage,
        ProcessClipboard,
        ConfirmDelete,
        // etc.
    };

    //settings
    QMap<QString, QString> m_hotkeyMappings;
    QMap<Action, QString> m_actionNameMap;
    QString m_customJSXPath;

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void performAction(Action action);

    void onSettingsButtonClicked();

    void refreshCustomJSXPath();

private slots:
    // UI-related slots
    void onOpenButtonClicked();
    void onHistoryButtonClicked();
    void onNextButtonClicked();
    void onFlipButtonClicked();
    void onGrayscaleButtonClicked();
    void onPosterizeButtonClicked(int levels);
    void onDegradeButtonClicked();
    void onMedianFilterButtonClicked();
    void confirmAndMoveFileToDeleteFolder();
    void startSchedule();
    void editSchedule();

private:
    // Internal helper methods
    void setDirectory(const QString& directory);
    void loadImageFromDirectory(const QString& directory);
    QString getRandomImage(const QString& directory);
    void processAndDisplayImage(const QString& filePath);
    void processClipboardImage();
    void processImage(const QImage& image);
    void saveImageToSharedFolder(const QImage& image, const QString& suffix);
    void deleteOldestFolderIfNeeded();
    QString m_tempDisplayedFilePath;
    // Scheduling
    void startNextTimerInSchedule();

    // UI members
    ZoomableGraphicsView* m_view;
    QGraphicsPixmapItem* m_originalPixmapItem;
    QGraphicsPixmapItem* m_grayscalePixmapItem;
    QPushButton* startTimerButton;

    // Pixmaps
    QPixmap m_originalPixmap;
    QPixmap m_grayscalePixmap;
    QString m_tempRulerFilePath;
    QString m_tempOriginalFilePath;
    QString m_tempGrayscaleFilePath;

    // Directory & file handling
    QString m_directory;
    QFileInfoList m_files;
    int m_currentIndex = 0;
    QStringList directoryHistory;
    QString m_currentImagePath;

    // Timers
    bool scheduleActive;
    QTime countdownTime;
    QList<QTime> schedule;
    int currentScheduleIndex;
    QList<QTimer*> timers;
    QTimer* m_countdownTimer;
    bool isTimerRunning;

    QPushButton* m_timeButton;
    QTime m_defaultCountdownTime;

    // Image states
    bool m_isGrayscale;
    bool m_isPosterized;
    bool m_isBlurred;
    bool m_isMedianFiltered;

    // Folder housekeeping
    QMap<QString, QDateTime> m_folderCreationTimes;
    QString m_tempFilePath;

    // Settings
    QSettings settings;  // used to store/restore QSettings

    // New member variable
    bool m_copyPasteEnabled;

    // Map each Action to a key code (Qt::Key_*)
    QMap<Action, int> m_actionKeyMap;

    // Reverse lookup: key code => Action
    QMap<int, Action> m_keyActionMap;

    // We'll store key->action. e.g. Qt::Key_F1 -> Action::ProcessClipboard
    QMap<int, Action> m_defaultKeyMap;

    // Also, some actions might open dynamic URLs. We'll keep a map of Action -> URL
    // for the ones that are just open-URL type:
    QMap<Action, QString> m_actionUrlMap;

    // New method to apply posterization
    void applyPosterization(int levels);
};

#endif // MAINWINDOW_H