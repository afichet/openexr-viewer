#pragma once

#include <QGraphicsView>

#include <model/ImageModel.h>

class GraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget *parent = nullptr);
    virtual ~GraphicsView();

public slots:
    void setModel(const ImageModel *model);

    void onImageLoaded(int width, int height);
    void onImageChanged();
//    void onMacbethChartChanged();
//    void setShowMacbeth(bool show);
//    void setShowPatchNumbers(bool show);
    void setZoomLevel(float zoom);
    void zoomIn();
    void zoomOut();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;

private:
    const ImageModel *         _model;
    QGraphicsPixmapItem *_imageItem;

    QVector<QGraphicsItem *> _chartItems;

//    bool                  _inSelection;
    int                   _selectedIdx;
    QGraphicsEllipseItem *_selection;

    QPoint _startDrag;

    bool _showMacbeth;
    bool _showPatchNumbers;

    float _zoomLevel;
    bool  _autoscale;
};

