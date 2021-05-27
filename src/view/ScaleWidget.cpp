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

#include "ScaleWidget.h"

#include <QPainter>
#include <QResizeEvent>

ScaleWidget::ScaleWidget(QWidget *parent)
  : QWidget(parent)
  , m_min(0.)
  , m_max(1.)
  , m_cmap(ColormapModule::create("grayscale"))
{}

QSize ScaleWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize ScaleWidget::sizeHint() const
{
    return QSize(100, 100);
}

void ScaleWidget::setMin(double value)
{
    m_min = value;
    update();
}

void ScaleWidget::setMax(double value)
{
    m_max = value;
    update();
}

void ScaleWidget::setColormap(ColormapModule::Map map)
{
    if (m_cmap) {
        delete m_cmap;
        m_cmap = nullptr;
    }

    m_cmap = ColormapModule::create(map);

    update();
}

void ScaleWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);

    const int barWidth    = 30;
    const int left_margin = 5;

    const int topBottom_margins = 10;

    const int start_y = topBottom_margins;
    const int end_y   = m_height - topBottom_margins;

    for (int y = start_y; y < end_y; y++) {
        float RGB[3];
        m_cmap->getRGBValue(y, end_y, start_y, RGB);

        painter.setPen(QColor(255 * RGB[0], 255 * RGB[1], 255 * RGB[2]));
        painter.drawLine(left_margin, y, barWidth + left_margin, y);
    }

    painter.setPen(QColor(125, 125, 125));
    painter.drawRect(QRect(left_margin, start_y, barWidth, end_y - start_y));

    const int end_bar_x        = left_margin + barWidth;
    const int text_left_margin = 10;
    const int start_text_x     = end_bar_x + text_left_margin;

    const int n_sec    = 5;
    const int fontSize = 12;

    QFont font = painter.font();
    font.setPixelSize(fontSize);
    painter.setFont(font);

    for (int i = 0; i < n_sec; i++) {
        float a = float(i) / float(n_sec - 1);
        float y = a * (end_y - start_y) + start_y;

        painter.setPen(QColor(125, 125, 125));
        painter.drawLine(left_margin, y, start_text_x - 5, y);

        float  value = (1.f - a) * (m_max - m_min) + m_min;
        QRectF textBox(
          start_text_x,
          y - fontSize / 3,
          m_width - start_text_x,
          fontSize);
        painter.setPen(Qt::white);
        painter.drawText(
          textBox,
          Qt::AlignLeft | Qt::AlignVCenter,
          QString::number(value));
    }
}

void ScaleWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    m_width  = e->size().width();
    m_height = e->size().height();
}
