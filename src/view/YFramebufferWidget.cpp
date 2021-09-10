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

#include "YFramebufferWidget.h"
#include "ui_YFramebufferWidget.h"

#include <util/ColormapModule.h>

YFramebufferWidget::YFramebufferWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::YFramebufferWidget)
  , m_model(nullptr)
{
    ui->setupUi(this);

    // clang-format off
    connect(
        ui->graphicsView, SIGNAL(openFileOnDropEvent(QString)),
        this,             SLOT(onOpenFileOnDropEvent(QString)));

    connect(
        ui->graphicsView, SIGNAL(queryPixelInfo(int, int)),
        this,             SLOT(onQueryPixelInfo(int, int)));
    // clang-format on

    for (int i = 0; i < ColormapModule::N_MAPS; i++) {
        ui->cbColormap->addItem(QString::fromStdString(
          ColormapModule::toString((ColormapModule::Map)i)));
    }
}


YFramebufferWidget::~YFramebufferWidget()
{
    delete ui;
    if (m_model) delete m_model;
}


void YFramebufferWidget::setModel(YFramebufferModel *model)
{
    m_model = model;
    ui->graphicsView->setModel(model);
}


void YFramebufferWidget::onQueryPixelInfo(int x, int y)
{
    ui->selectInfoLabel->setText(
      QString::fromStdString(m_model->getColorInfo(x, y)));
}


void YFramebufferWidget::on_sbMinValue_valueChanged(double arg1)
{
    ui->sbMaxValue->setMinimum(arg1);
    ui->scaleWidget->setMin(arg1);
    if (m_model) m_model->setMinValue(arg1);
}


void YFramebufferWidget::on_sbMaxValue_valueChanged(double arg1)
{
    ui->sbMinValue->setMaximum(arg1);
    ui->scaleWidget->setMax(arg1);
    if (m_model) m_model->setMaxValue(arg1);
}


void YFramebufferWidget::on_buttonAuto_clicked()
{
    if (m_model) {
        ui->sbMinValue->setValue(m_model->getDatasetMin());
        ui->sbMaxValue->setValue(m_model->getDatasetMax());
    }
}


void YFramebufferWidget::onOpenFileOnDropEvent(const QString &filename)
{
    emit openFileOnDropEvent(filename);
}


void YFramebufferWidget::on_cbColormap_currentIndexChanged(int index)
{
    ColormapModule::Map cmap = (ColormapModule::Map)index;

    ui->scaleWidget->setColormap(cmap);

    if (m_model) m_model->setColormap(cmap);
}


void YFramebufferWidget::on_cbShowDataWindow_stateChanged(int arg1)
{
    ui->graphicsView->showDataWindow(arg1 == Qt::Checked);
}


void YFramebufferWidget::on_cbShowDisplayWindow_stateChanged(int arg1)
{
    ui->graphicsView->showDisplayWindow(arg1 == Qt::Checked);
}


void YFramebufferWidget::on_cbScale_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        ui->scaleWidget->show();
    } else {
        ui->scaleWidget->hide();
    }
}
