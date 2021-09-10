/**
 * Copyright (c) 2021 Alban Fichet <alban dot fichet at gmx dot fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *  * Neither the name of the organization(s) nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GraphicsView.h"
#include "GraphicsScene.h"

#include <QDragEnterEvent>
#include <QGraphicsPixmapItem>
#include <QGuiApplication>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QScrollBar>
#include <QUrl>

GraphicsView::GraphicsView(QWidget *parent)
  : QGraphicsView(parent)
  , _model(nullptr)
  , _imageItem(nullptr)
  , _zoomLevel(1.f)
  , _autoscale(true)
  , _showDataWindow(true)
  , _showDisplayWindow(true)
{
    GraphicsScene *scene = new GraphicsScene;
    setScene(scene);
    //  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
    setAcceptDrops(true);

    connect(
      scene,
      SIGNAL(openFileOnDropEvent(QString)),
      this,
      SLOT(open(QString)));
}

GraphicsView::~GraphicsView()
{
    //    delete _model;
}

void GraphicsView::setModel(const FramebufferModel *model)
{
    _model = model;

    // clang-format off
    connect(_model, SIGNAL(imageChanged()), this, SLOT(onImageChanged()));
    connect(_model, SIGNAL(imageLoaded()),  this, SLOT(onImageLoaded()));
    // clang-format on
}

void GraphicsView::onImageLoaded()
{
    _dataWindow    = _model->getDataWindow();
    _displayWindow = _model->getDisplayWindow();

    // Stretch or shrink width according to pixelAspectRatio
    const float aspect = _model->pixelAspectRatio();

    const int displayWindowW       = aspect * _displayWindow.width();
    const int displayWindowCenterX = _displayWindow.center().x();

    _displayWindow.setLeft(displayWindowCenterX - displayWindowW / 2.f);
    _displayWindow.setRight(displayWindowCenterX + displayWindowW / 2.f);

    const int dataWindowW       = aspect * _dataWindow.width();
    const int dataWindowCenterX = _dataWindow.center().x();

    _dataWindow.setLeft(dataWindowCenterX - dataWindowW / 2.f);
    _dataWindow.setRight(dataWindowCenterX + dataWindowW / 2.f);

    // Adapt display and data windows to the image display starting at 0, 0
    _displayWindow.translate(
      -_dataWindow.topLeft().x(),
      -_dataWindow.topLeft().y());

    _dataWindow.translate(
      -_dataWindow.topLeft().x(),
      -_dataWindow.topLeft().y());

    // Fit view to display window
    autoscale();
}

void GraphicsView::onImageChanged()
{
    if (_model == nullptr) return;

    if (_imageItem != nullptr) {
        scene()->removeItem(_imageItem);
        delete _imageItem;
        _imageItem = nullptr;
    }

    const QImage &loadedImage = _model->getLoadedImage();

    // We need to resize the image according to pixelAspectRatio
    // Small optim, no need to process the transform is aspect ratio = 1
    if (_model->pixelAspectRatio() != 1.f) {
        const QImage aspectCorrectedImage = loadedImage.scaled(
          loadedImage.width() * _model->pixelAspectRatio(),
          loadedImage.height(),
          Qt::IgnoreAspectRatio,
          Qt::SmoothTransformation);

        _imageItem
          = scene()->addPixmap(QPixmap::fromImage(aspectCorrectedImage));
    } else {
        _imageItem = scene()->addPixmap(QPixmap::fromImage(loadedImage));
    }
}

void GraphicsView::setZoomLevel(double zoom)
{
    if (_model == nullptr || !_model->isImageLoaded())
        return;   // || zoom == _zoomLevel) return;

//    if (_zoomLevel == zoom) return;

    _zoomLevel = std::max(0.01, zoom);
    resetTransform();
    scale(_zoomLevel, _zoomLevel);

    // We want autoscale when loading a new image
    _autoscale = false;

    emit zoomLevelChanged(zoom);
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

void GraphicsView::autoscale()
{
    scene()->setSceneRect(_displayWindow);
    fitInView(_displayWindow, Qt::KeepAspectRatio);

    _zoomLevel = std::min(viewportTransform().m11(), viewportTransform().m22());
    _zoomLevel = std::min(_zoomLevel, 1.);
    _zoomLevel = std::max(0.01, _zoomLevel);

    resetTransform();
    scale(_zoomLevel, _zoomLevel);

    emit zoomLevelChanged(_zoomLevel);

    // We want autoscale when loading a new image
    _autoscale = true;
}

void GraphicsView::open(const QString &filename)
{
    emit openFileOnDropEvent(filename);
}

void GraphicsView::showDisplayWindow(bool show)
{
    _showDisplayWindow = show;
    scene()->invalidate();
    // scene()->sceneRect(), QGraphicsScene::ForegroundLayer);
}

void GraphicsView::showDataWindow(bool show)
{
    _showDataWindow = show;
    scene()->invalidate();
    // scene()->sceneRect(), QGraphicsScene::ForegroundLayer);
}

// void GraphicsView::showDatawindowBoders(bool visible)
//{
//    if (_model == nullptr) return;

//    if (_datawindowItem != nullptr) {
//        scene()->removeItem(_datawindowItem);
//        delete _datawindowItem;
//        _datawindowItem = nullptr;
//    }

//    _datawindowItem = scene()->addRect(0, 0, m_)
//}

// void GraphicsView::showDisplaywindowBorders(bool visible)
//{

//}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) != 0U) {
        QGraphicsView::wheelEvent(event);
    } else {
        if (_model == nullptr || !_model->isImageLoaded()) return;

        const QPoint delta = event->angleDelta();

        if (delta.y() != 0) {
            if (delta.y() > 0) {
                zoomIn();
            } else {
                zoomOut();
            }
        }
    }
}

void GraphicsView::resizeEvent(QResizeEvent *e)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    if (_autoscale) {
        autoscale();
    } else {
        // Recenter the image
        resetTransform();
        scale(_zoomLevel, _zoomLevel);
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    if (
      (event->button() == Qt::MiddleButton)
      || (event->button() == Qt::LeftButton)) {
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
      ((event->buttons() & Qt::MiddleButton) != 0U)
      || ((event->buttons() & Qt::LeftButton) != 0U)) {
        QScrollBar *        hBar  = horizontalScrollBar();
        QScrollBar *        vBar  = verticalScrollBar();
        QPoint              delta = event->pos() - _startDrag;
        std::pair<int, int> bar_values;
        bar_values.first
          = hBar->value() + (isRightToLeft() ? delta.x() : -delta.x());
        bar_values.second = vBar->value() - delta.y();
        hBar->setValue(bar_values.first);
        vBar->setValue(bar_values.second);
        _startDrag = event->pos();
    } else {
        QPointF imgCoords = mapToScene(event->pos());
        emit queryPixelInfo(imgCoords.x(), imgCoords.y());
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *)
{
    if (_model == nullptr || !_model->isImageLoaded()) return;

    setCursor(Qt::ArrowCursor);
}

void GraphicsView::dropEvent(QDropEvent *ev)
{
    if (_model == nullptr) return;

    QList<QUrl> urls = ev->mimeData()->urls();

    if (!urls.empty()) {
        QString filename = urls[0].toString();
        QString startFileTypeString =
#ifdef _WIN32
          "file:///";
#else
          "file://";
#endif

        if (filename.startsWith(startFileTypeString)) {
            filename = filename.remove(0, startFileTypeString.length());

            emit openFileOnDropEvent(filename);
        }
    }
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}

void GraphicsView::drawBackground(QPainter *painter, const QRectF &)
{
    const int polySize = 16;

    QBrush a0(QColor(125, 125, 125));
    QBrush a1(QColor(100, 100, 100));

    painter->resetTransform();
    painter->setPen(Qt::NoPen);

    for (int i = 0; i < width() / polySize + 1; i++) {
        const int x = i * polySize;

        for (int j = 0; j < height() / polySize + 1; j++) {
            const int y = j * polySize;

            if ((i + j) % 2 == 0) {
                painter->setBrush(a0);
            } else {
                painter->setBrush(a1);
            }

            painter->drawRect(QRect(x, y, polySize, polySize));
        }
    }
}

void GraphicsView::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (_model) {
        if (_showDisplayWindow) {
            QPainterPath outerPath;
            QPainterPath innerPath;

            outerPath.addRect(rect);
            innerPath.addRect(_displayWindow);

            QPainterPath fillPath = outerPath.subtracted(innerPath);

            painter->fillPath(fillPath, QColor(0, 0, 0, 150));
        }

        painter->resetTransform();

        if (_showDataWindow) {
            QPolygonF dataW = mapFromScene(_dataWindow);

            painter->setPen(Qt::red);
            painter->drawPolygon(dataW);
        }

        if (_showDisplayWindow) {
            QPolygonF displayW = mapFromScene(_displayWindow);

            painter->setPen(Qt::black);
            painter->drawPolygon(displayW);
        }
    }
}

void GraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);

    // Problem with background drawing if not doing that...
    scene()->invalidate();
}
