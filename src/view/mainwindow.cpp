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

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <cassert>

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QMimeData>
#include <QSettings>

#include <model/attribute/HeaderModel.h>
#include <model/attribute/LayerItem.h>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_openFileTabs(new QTabWidget(this))

  , m_statusBarMessage(new QLabel(this))
{
    ui->setupUi(this);
    setAcceptDrops(true);

    setCentralWidget(m_openFileTabs);

    statusBar()->addPermanentWidget(m_statusBarMessage);

    readSettings();

    // clang-format off
    connect(m_openFileTabs, SIGNAL(currentChanged(int)),
            this,           SLOT(onCurrentChanged(int)));
    // clang-format on
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open(const QString &filename)
{
    QString filename_no_path = QFileInfo(filename).baseName();

    ImageFileWidget *fileWidget = new ImageFileWidget(m_openFileTabs);
    fileWidget->open(filename);
    fileWidget->setSplitterState(m_splitterState);

    m_openFileTabs->addTab(fileWidget, filename_no_path);
    m_openFileTabs->setCurrentWidget(fileWidget);
}

void MainWindow::on_action_Open_triggered()
{
    const QString filename = QFileDialog::getOpenFileName(
      this,
      tr("Open OpenEXR Image"),
      m_currentOpenedFolder,
      tr("Images (*.exr)"));

    if (filename.size() != 0) {
        open(filename);
    }
}

void MainWindow::on_action_Quit_triggered()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *ev)
{
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
            open(filename);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}

void MainWindow::writeSettings()
{
    QSettings settings(
      QSettings::IniFormat,
      QSettings::UserScope,
      "afichet",
      "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("splitter", m_splitterState);
    settings.setValue("openedFolder", m_currentOpenedFolder);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings(
      QSettings::IniFormat,
      QSettings::UserScope,
      "afichet",
      "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_splitterState = settings.value("splitter").toByteArray();

    if (settings.contains("openedFolder")) {
        m_currentOpenedFolder = settings.value("openedFolder").toString();
    } else {
        m_currentOpenedFolder = QDir::homePath();
    }

    settings.endGroup();
}





void MainWindow::on_action_Tabbed_triggered()
{
    ImageFileWidget* widget = (ImageFileWidget*)m_openFileTabs->currentWidget();

    if (widget) {
        widget->setTabbed();
    }
}

void MainWindow::on_action_Cascade_triggered()
{
    ImageFileWidget* widget = (ImageFileWidget*)m_openFileTabs->currentWidget();

    if (widget) {
        widget->setCascade();
    }
}

void MainWindow::on_action_Tiled_triggered()
{
    ImageFileWidget* widget = (ImageFileWidget*)m_openFileTabs->currentWidget();

    if (widget) {
        widget->setTiled();
    }
}

void MainWindow::on_action_Refresh_triggered()
{
//    open(m_img->getFilename());

    // TODO: Shall refresh and reopen all windows
}

void MainWindow::onCurrentChanged(int index)
{
    if (index == -1) return;

    ImageFileWidget* widget = (ImageFileWidget*)m_openFileTabs->currentWidget();
    m_currentOpenedFolder = widget->getOpenedFolder();
    m_splitterState = widget->getSplitterState();

    m_statusBarMessage->setText(widget->getOpenedFilename());

}
