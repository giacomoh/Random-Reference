#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGraphicsView>
#include <QTimeEdit>  // Add this line

class ZoomableGraphicsView;  // Forward declaration

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    QString getRandomImage(const QString& directory);
    int levels;  // Add this line
    QImage posterizeImage(const QImage& image, int levels);

private slots:
    void onOpenButtonClicked();
    void onNextButtonClicked();
    void onFlipButtonClicked();
    void onGrayscaleButtonClicked();  // Declare the onGrayscaleButtonClicked function
    void onPosterizeButtonClicked(int levels);

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

private:
    void loadImageFromDirectory(const QString& directory);

private:
    QString m_directory;

protected:
    void closeEvent(QCloseEvent* event) override;
};

#endif // MAINWINDOW_H
