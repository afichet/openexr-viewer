//
// Copyright (c) 2021 Alban Fichet <alban.fichet at gmx.fr>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//  * Neither the name of %ORGANIZATION% nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#pragma once

#include <QGraphicsView>

#include <model/ImageModel.h>

#include <iostream>

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

    virtual void drawBackground(QPainter *painter, const QRectF &rect) override
    {
        int polySize = 16;

        QBrush a0(QColor(200, 200, 200));
        QBrush a1(QColor(255, 255, 255));

        painter->setPen(Qt::NoPen);

        for (int i = 0; i < width()/polySize; i++) {
            const int x = i * polySize;

            for (int j = 0; j < height()/polySize; j++) {
                const int y = j * polySize;

                QPolygonF rect = mapToScene(QRect(x, y, polySize, polySize));

                if ((i+j)%2 == 0) {
                    painter->setBrush(a0);
                } else {
                    painter->setBrush(a1);
                }

                painter->drawPolygon(rect);
            }
        }
    }

private:
    const ImageModel *_model;
    QGraphicsPixmapItem *_imageItem;

    QPoint _startDrag;

    float _zoomLevel;
    bool  _autoscale;
};

