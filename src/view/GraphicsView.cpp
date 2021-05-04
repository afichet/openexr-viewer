#include "GraphicsView.h"
#include "GraphicsScene.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPainter>
#include <QMouseEvent>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QGuiApplication>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , _model(nullptr)
    , _imageItem(nullptr)
//    , _inSelection(false)
    , _selection(nullptr)
    , _showMacbeth(true)
    , _showPatchNumbers(false)
    , _zoomLevel(1.f)
    , _autoscale(true)
{
    setScene(new GraphicsScene);
    //  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setAcceptDrops(true);
}


GraphicsView::~GraphicsView() {
//    delete _model;
}


void GraphicsView::setModel(const ImageModel *model)
{
    _model = model;

    connect(_model, SIGNAL(imageChanged()), this, SLOT(onImageChanged()));
    connect(_model, SIGNAL(imageLoaded(int,int)), this, SLOT(onImageLoaded(int,int)));
}


void GraphicsView::onImageLoaded(int width, int height)
{
    _zoomLevel = 1.f;
    fitInView(0, 0, width, height, Qt::KeepAspectRatio);
    _zoomLevel = std::min(viewportTransform().m11(), viewportTransform().m22());
    _autoscale = true;
}


void GraphicsView::onImageChanged()
{
    if (_model == nullptr) return;

    if (_imageItem != nullptr)
    {
        scene()->removeItem(_imageItem);
        delete _imageItem;
        _imageItem = nullptr;
    }

    const QImage &image = _model->getLoadedImage();
    _imageItem          = scene()->addPixmap(QPixmap::fromImage(image));
}


void GraphicsView::setZoomLevel(float zoom)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;
    _autoscale = false;
    _zoomLevel = std::max(0.01f, zoom);
    resetTransform();
    scale(_zoomLevel, _zoomLevel);
}


void GraphicsView::zoomIn()
{
    if (_model == nullptr || !_model->isImageLoaded()) return;
    setZoomLevel(_zoomLevel * 1.1);
}


void GraphicsView::zoomOut()
{
    if (_model == nullptr || !_model->isImageLoaded()) return;
    setZoomLevel(_zoomLevel / 1.1);
}


void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) != 0U)
    {
        QGraphicsView::wheelEvent(event);
    }
    else
    {
        if (_model == nullptr || !_model->isImageLoaded()) return;
        const QPoint delta = event->angleDelta();

        if (delta.y() != 0)
        {
            if (delta.y() > 0)
            {
                zoomIn();
            }
            else
            {
                zoomOut();
            }
        }
    }
}


void GraphicsView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    if (_model == nullptr || !_model->isImageLoaded()) return;

    if (_autoscale)
        fitInView(0, 0, _model->getLoadedImage().width(), _model->getLoadedImage().height(), Qt::KeepAspectRatio);
}


void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    if (
             (event->button() == Qt::MidButton)
             || (event->button() == Qt::LeftButton && QGuiApplication::keyboardModifiers() == Qt::ControlModifier))
    {
        QGraphicsView::mousePressEvent(event);
        setCursor(Qt::ClosedHandCursor);
        _startDrag = event->pos();
        return;
    }
}


void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    if (
             ((event->buttons() & Qt::MidButton) != 0U)
             || (((event->buttons() & Qt::LeftButton) != 0U) && QGuiApplication::keyboardModifiers() == Qt::ControlModifier))
    {
        QScrollBar *        hBar  = horizontalScrollBar();
        QScrollBar *        vBar  = verticalScrollBar();
        QPoint              delta = event->pos() - _startDrag;
        std::pair<int, int> bar_values;
        bar_values.first  = hBar->value() + (isRightToLeft() ? delta.x() : -delta.x());
        bar_values.second = vBar->value() - delta.y();
        hBar->setValue(bar_values.first);
        vBar->setValue(bar_values.second);
        _startDrag = event->pos();
    }
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    setCursor(Qt::ArrowCursor);
}


void GraphicsView::dropEvent(QDropEvent *ev)
{
    if (_model == nullptr) return;

    QList<QUrl> urls = ev->mimeData()->urls();

    if (!urls.empty())
    {
        QString fileName = urls[0].toString();
        QString startFileTypeString =
        #ifdef _WIN32
                "file:///";
#else
                "file://";
#endif

        if (fileName.startsWith(startFileTypeString))
        {
            fileName = fileName.remove(0, startFileTypeString.length());
//            _model->openFile(fileName);
            // TODO
        }
    }
}


void GraphicsView::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}
