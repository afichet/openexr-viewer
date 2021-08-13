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

#include <QTabWidget>
#include <QLabel>
#include <QMainWindow>
#include <QVector>
#include <QCloseEvent>
#include <QDropEvent>
#include <QByteArray>

#include <model/attribute/HeaderItem.h>
#include <model/framebuffer/RGBFramebufferModel.h>

#include "ImageFileWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow: public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void open(const QString &filename);

  private slots:
    void on_action_Open_triggered();
    void on_action_Quit_triggered();

  protected:
    void closeEvent(QCloseEvent *event) override;
    //    void showEvent(QShowEvent* event) override;

  private:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;

    void writeSettings();
    void readSettings();



  private slots:
    void on_action_Tabbed_triggered();

    void on_action_Cascade_triggered();

    void on_action_Tiled_triggered();

    void on_action_Refresh_triggered();

    void onCurrentChanged(int index);

  private:
    Ui::MainWindow *ui;

    QTabWidget *m_openFileTabs;

    QLabel *m_statusBarMessage;

    QVector<ImageFileWidget*> m_imageFileWidgets;

    QString m_currentOpenedFolder;

    QByteArray m_splitterState;
};
