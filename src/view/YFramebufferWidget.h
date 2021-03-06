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

#include <QWidget>

#include <model/framebuffer/YFramebufferModel.h>

namespace Ui
{
    class YFramebufferWidget;
}

class YFramebufferWidget: public QWidget
{
    Q_OBJECT

  public:
    explicit YFramebufferWidget(QWidget* parent = nullptr);
    ~YFramebufferWidget();

    void setModel(YFramebufferModel* model);

  signals:
    void openFileOnDropEvent(const QString& filename);

  private slots:
    void onQueryPixelInfo(int x, int y);

    void on_sbMinValue_valueChanged(double arg1);

    void on_sbMaxValue_valueChanged(double arg1);

    void on_buttonAuto_clicked();

    void onOpenFileOnDropEvent(const QString& filename);

    void on_cbColormap_currentIndexChanged(int index);

    void on_cbShowDataWindow_stateChanged(int arg1);

    void on_cbShowDisplayWindow_stateChanged(int arg1);

    void on_cbScale_stateChanged(int arg1);

  private:
    Ui::YFramebufferWidget* ui;
    YFramebufferModel*      m_model;
};
