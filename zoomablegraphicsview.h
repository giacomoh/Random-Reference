// zoomablegraphicsview.h

#ifndef ZOOMABLEGRAPHICSVIEW_H
#define ZOOMABLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPoint>
#include <QPen>

class QGraphicsLineItem;
class QGraphicsPolygonItem;

class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget* parent = nullptr);
    ~ZoomableGraphicsView() override;

    // Overloaded function to accept imageRect
    void createAndAddLines(const QRectF& imageRect);
    void setLinesVisibility(bool visible);
    bool areLinesVisible() const;

    // Save just the lines as a PNG
    void saveRulerImage(const QString& tempFilePath);

    // Reset panning and optionally re-center/fit the scene
    void resetPan();

protected:
    // Overridden event handlers
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // Called after zooming to keep line thickness scaled
    void updateLineThickness();

    // Lines
    QGraphicsLineItem* m_horizontalLine;
    QGraphicsLineItem* m_verticalLine;
    QGraphicsLineItem* m_diagonalLine1;
    QGraphicsLineItem* m_diagonalLine2;
    QGraphicsPolygonItem* m_rhomboid;

    QGraphicsLineItem* m_additionalHorizontalLine1;
    QGraphicsLineItem* m_additionalHorizontalLine2;
    QGraphicsLineItem* m_additionalHorizontalLine3;
    QGraphicsLineItem* m_additionalVerticalLine1;
    QGraphicsLineItem* m_additionalVerticalLine2;
    QGraphicsLineItem* m_additionalVerticalLine3;

    // Variables for manual panning
    bool m_isPanning;
    QPoint m_lastPanPoint;

    // Member variable to store imageRect
    QRectF m_imageRect;
};

#endif // ZOOMABLEGRAPHICSVIEW_H
