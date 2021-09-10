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

#pragma once

#include <QGraphicsView>

#include <model/framebuffer/FramebufferModel.h>

#include <iostream>

class GraphicsView: public QGraphicsView
{
    Q_OBJECT
  public:
    GraphicsView(QWidget *parent = nullptr);
    virtual ~GraphicsView();

  public slots:
    void setModel(const FramebufferModel *model);

    void onImageLoaded();
    void onImageChanged();

    void setZoomLevel(double zoom);
    void zoomIn();
    void zoomOut();

    void autoscale();

    void open(const QString &filename);

    void showDisplayWindow(bool show);
    void showDataWindow(bool show);

  signals:
    void zoomLevelChanged(double zoom);
    void openFileOnDropEvent(const QString &filename);
    void queryPixelInfo(int x, int y);

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;

    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void drawForeground(QPainter *painter, const QRectF &rect) override;

    virtual void scrollContentsBy(int dx, int dy) override;

  private:
    const FramebufferModel *_model;
    QGraphicsPixmapItem *   _imageItem;
    //    QGraphicsRectItem *_datawindowItem;
    //    QGraphicsRectItem *_displaywindowItem;

    QPoint _startDrag;

    double _zoomLevel;
    bool   _autoscale;

    QRectF _dataWindow;
    QRectF _displayWindow;

    bool _showDataWindow;
    bool _showDisplayWindow;
};
