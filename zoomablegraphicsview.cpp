// zoomablegraphicsview.cpp

#include "zoomablegraphicsview.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QImage>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QProcess>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <algorithm>

// Constructor
ZoomableGraphicsView::ZoomableGraphicsView(QWidget* parent)
    : QGraphicsView(parent),
    m_horizontalLine(nullptr),
    m_verticalLine(nullptr),
    m_diagonalLine1(nullptr),
    m_diagonalLine2(nullptr),
    m_rhomboid(nullptr),
    m_additionalHorizontalLine1(nullptr),
    m_additionalHorizontalLine2(nullptr),
    m_additionalHorizontalLine3(nullptr),
    m_additionalVerticalLine1(nullptr),
    m_additionalVerticalLine2(nullptr),
    m_additionalVerticalLine3(nullptr),
    m_isPanning(false)
{
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::gray);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);

    // Enable built-in panning with the mouse
    setDragMode(QGraphicsView::ScrollHandDrag);

    // When zooming with the wheel, anchor under the mouse
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Create a default scene if none is passed in
    if (!scene()) {
        setScene(new QGraphicsScene(this));
    }
}

// Destructor
ZoomableGraphicsView::~ZoomableGraphicsView()
{
    // Qt automatically deletes child QGraphicsItems when the scene is deleted
}

// Create lines for overlay based on imageRect
void ZoomableGraphicsView::createAndAddLines(const QRectF& imageRect)
{
    if (!scene()) return;

    // Store the imageRect
    m_imageRect = imageRect;

    // Calculate thickness based on image size
    int thickness = std::max(1, static_cast<int>(
        std::min(imageRect.width(), imageRect.height()) / 500));
    QPen pen(Qt::red, thickness);

    // Main horizontal line
    m_horizontalLine = new QGraphicsLineItem(
        imageRect.left(), imageRect.center().y(),
        imageRect.right(), imageRect.center().y());
    m_horizontalLine->setPen(pen);
    scene()->addItem(m_horizontalLine);

    // Main vertical line
    m_verticalLine = new QGraphicsLineItem(
        imageRect.center().x(), imageRect.top(),
        imageRect.center().x(), imageRect.bottom());
    m_verticalLine->setPen(pen);
    scene()->addItem(m_verticalLine);

    // Diagonal lines
    m_diagonalLine1 = new QGraphicsLineItem(
        imageRect.left(), imageRect.top(),
        imageRect.right(), imageRect.bottom());
    m_diagonalLine2 = new QGraphicsLineItem(
        imageRect.right(), imageRect.top(),
        imageRect.left(), imageRect.bottom());
    m_diagonalLine1->setPen(pen);
    m_diagonalLine2->setPen(pen);
    scene()->addItem(m_diagonalLine1);
    scene()->addItem(m_diagonalLine2);

    // Additional horizontal lines
    m_additionalHorizontalLine1 = new QGraphicsLineItem(
        imageRect.left(), imageRect.top() + imageRect.height() / 4,
        imageRect.right(), imageRect.top() + imageRect.height() / 4);
    m_additionalHorizontalLine2 = new QGraphicsLineItem(
        imageRect.left(), imageRect.top() + 2 * imageRect.height() / 4,
        imageRect.right(), imageRect.top() + 2 * imageRect.height() / 4);
    m_additionalHorizontalLine3 = new QGraphicsLineItem(
        imageRect.left(), imageRect.top() + 3 * imageRect.height() / 4,
        imageRect.right(), imageRect.top() + 3 * imageRect.height() / 4);
    m_additionalHorizontalLine1->setPen(pen);
    m_additionalHorizontalLine2->setPen(pen);
    m_additionalHorizontalLine3->setPen(pen);
    scene()->addItem(m_additionalHorizontalLine1);
    scene()->addItem(m_additionalHorizontalLine2);
    scene()->addItem(m_additionalHorizontalLine3);

    // Additional vertical lines
    m_additionalVerticalLine1 = new QGraphicsLineItem(
        imageRect.left() + imageRect.width() / 4, imageRect.top(),
        imageRect.left() + imageRect.width() / 4, imageRect.bottom());
    m_additionalVerticalLine2 = new QGraphicsLineItem(
        imageRect.left() + 2 * imageRect.width() / 4, imageRect.top(),
        imageRect.left() + 2 * imageRect.width() / 4, imageRect.bottom());
    m_additionalVerticalLine3 = new QGraphicsLineItem(
        imageRect.left() + 3 * imageRect.width() / 4, imageRect.top(),
        imageRect.left() + 3 * imageRect.width() / 4, imageRect.bottom());
    m_additionalVerticalLine1->setPen(pen);
    m_additionalVerticalLine2->setPen(pen);
    m_additionalVerticalLine3->setPen(pen);
    scene()->addItem(m_additionalVerticalLine1);
    scene()->addItem(m_additionalVerticalLine2);
    scene()->addItem(m_additionalVerticalLine3);

    // Optional rhomboid
    QPolygonF polygon;
    polygon << QPointF(imageRect.center().x(), imageRect.top())
        << QPointF(imageRect.right(), imageRect.center().y())
        << QPointF(imageRect.center().x(), imageRect.bottom())
        << QPointF(imageRect.left(), imageRect.center().y());
    m_rhomboid = new QGraphicsPolygonItem(polygon);
    m_rhomboid->setPen(pen);
    scene()->addItem(m_rhomboid);
}

void ZoomableGraphicsView::setLinesVisibility(bool visible)
{
    if (m_horizontalLine) m_horizontalLine->setVisible(visible);
    if (m_verticalLine)   m_verticalLine->setVisible(visible);
    if (m_diagonalLine1)  m_diagonalLine1->setVisible(visible);
    if (m_diagonalLine2)  m_diagonalLine2->setVisible(visible);
    if (m_rhomboid)       m_rhomboid->setVisible(visible);

    if (m_additionalHorizontalLine1) m_additionalHorizontalLine1->setVisible(visible);
    if (m_additionalHorizontalLine2) m_additionalHorizontalLine2->setVisible(visible);
    if (m_additionalHorizontalLine3) m_additionalHorizontalLine3->setVisible(visible);
    if (m_additionalVerticalLine1)   m_additionalVerticalLine1->setVisible(visible);
    if (m_additionalVerticalLine2)   m_additionalVerticalLine2->setVisible(visible);
    if (m_additionalVerticalLine3)   m_additionalVerticalLine3->setVisible(visible);
}

bool ZoomableGraphicsView::areLinesVisible() const
{
    return (m_horizontalLine && m_horizontalLine->isVisible());
}

void ZoomableGraphicsView::saveRulerImage(const QString& tempFilePath)
{
    if (!scene()) {
        qDebug() << "No scene to save.";
        return;
    }

    // Use m_imageRect instead of sceneRect
    QRectF imageRect = m_imageRect;
    QSize imageSize = imageRect.size().toSize();

    // Debug: Log imageRect and imageSize
    qDebug() << "Image Rect:" << imageRect;
    qDebug() << "Image Size:" << imageSize;

    if (imageSize.width() <= 0 || imageSize.height() <= 0) {
        qDebug() << "Image has invalid size:" << imageSize;
        return;
    }

    QImage rulerImage(imageSize, QImage::Format_ARGB32);
    if (rulerImage.isNull()) {
        qDebug() << "Failed to create QImage with size:" << imageSize;
        return;
    }

    rulerImage.fill(Qt::transparent);

    QPainter painter(&rulerImage);
    if (!painter.isActive()) {
        qDebug() << "QPainter failed to begin.";
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    int thickness = std::max(1, static_cast<int>(
        std::min(imageRect.width(), imageRect.height()) / 500));
    QPen pen(Qt::red, thickness);
    painter.setPen(pen);

    // Draw lines if visible
    if (m_horizontalLine && m_horizontalLine->isVisible())
        painter.drawLine(m_horizontalLine->line());
    if (m_verticalLine && m_verticalLine->isVisible())
        painter.drawLine(m_verticalLine->line());
    if (m_diagonalLine1 && m_diagonalLine1->isVisible())
        painter.drawLine(m_diagonalLine1->line());
    if (m_diagonalLine2 && m_diagonalLine2->isVisible())
        painter.drawLine(m_diagonalLine2->line());
    if (m_rhomboid && m_rhomboid->isVisible())
        painter.drawPolygon(m_rhomboid->polygon());

    // Additional lines
    if (m_additionalHorizontalLine1 && m_additionalHorizontalLine1->isVisible())
        painter.drawLine(m_additionalHorizontalLine1->line());
    if (m_additionalHorizontalLine2 && m_additionalHorizontalLine2->isVisible())
        painter.drawLine(m_additionalHorizontalLine2->line());
    if (m_additionalHorizontalLine3 && m_additionalHorizontalLine3->isVisible())
        painter.drawLine(m_additionalHorizontalLine3->line());
    if (m_additionalVerticalLine1 && m_additionalVerticalLine1->isVisible())
        painter.drawLine(m_additionalVerticalLine1->line());
    if (m_additionalVerticalLine2 && m_additionalVerticalLine2->isVisible())
        painter.drawLine(m_additionalVerticalLine2->line());
    if (m_additionalVerticalLine3 && m_additionalVerticalLine3->isVisible())
        painter.drawLine(m_additionalVerticalLine3->line());

    painter.end();

    if (!tempFilePath.isEmpty()) {
        // Ensure the directory exists
        QFileInfo fileInfo(tempFilePath);
        QDir dir = fileInfo.dir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qDebug() << "Failed to create directory:" << dir.absolutePath();
                return;
            }
        }

        if (!rulerImage.save(tempFilePath)) {
            qDebug() << "Failed to save ruler image to" << tempFilePath;
        }
        else {
            qDebug() << "Saved ruler lines to" << tempFilePath;
        }
    }
}

// Zooming with the wheel
void ZoomableGraphicsView::wheelEvent(QWheelEvent* event)
{
    double scaleFactor = 1.15;
    if (event->modifiers() & Qt::ControlModifier) {
        scaleFactor = 1.02;  // Smaller zoom if holding Ctrl
    }
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    }
    else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    updateLineThickness();
}

// Keep lines at a comfortable thickness even after zoom
void ZoomableGraphicsView::updateLineThickness()
{
    if (!scene()) return;
    QRectF imageRect = m_imageRect;
    int thickness = std::max(1, static_cast<int>(
        std::min(imageRect.width(), imageRect.height()) / 500));
    QPen pen(Qt::red, thickness);

    // Main lines
    if (m_horizontalLine) m_horizontalLine->setPen(pen);
    if (m_verticalLine)   m_verticalLine->setPen(pen);
    if (m_diagonalLine1)  m_diagonalLine1->setPen(pen);
    if (m_diagonalLine2)  m_diagonalLine2->setPen(pen);
    if (m_rhomboid)       m_rhomboid->setPen(pen);

    // Additional lines
    if (m_additionalHorizontalLine1) m_additionalHorizontalLine1->setPen(pen);
    if (m_additionalHorizontalLine2) m_additionalHorizontalLine2->setPen(pen);
    if (m_additionalHorizontalLine3) m_additionalHorizontalLine3->setPen(pen);
    if (m_additionalVerticalLine1)   m_additionalVerticalLine1->setPen(pen);
    if (m_additionalVerticalLine2)   m_additionalVerticalLine2->setPen(pen);
    if (m_additionalVerticalLine3)   m_additionalVerticalLine3->setPen(pen);
}

// For panning with the mouse
void ZoomableGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void ZoomableGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        // Calculate the movement delta
        QPointF delta = mapToScene(m_lastPanPoint) - mapToScene(event->pos());
        m_lastPanPoint = event->pos();

        // Translate the view by the delta
        translate(delta.x(), delta.y());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ZoomableGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// Reset panning & optionally recenter
void ZoomableGraphicsView::resetPan()
{
    // Remove or comment out the resetTransform() call
    // resetTransform();

    // Optionally, ensure the view is centered
    // centerOn(scene()->sceneRect().center());
}
