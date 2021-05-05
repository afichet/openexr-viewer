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
#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMimeData>
#include <QSettings>

#include <QMdiSubWindow>

#include "RGBFramebufferWidget.h"
#include "FramebufferWidget.h"

#include <model/HeaderModel.h>
#include <model/OpenEXRLayerItem.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_splitter(new QSplitter(this))
    , m_img(nullptr)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    m_treeView = new QTreeView(m_splitter);
    m_mdiArea = new QMdiArea(m_splitter);
    m_mdiArea->setViewMode(QMdiArea::TabbedView);

    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_mdiArea);

    setCentralWidget(m_splitter);

    connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onDoubleClicked(QModelIndex)));

    readSettings();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete m_img;
}


void MainWindow::open(QString filename)
{
    if (m_img) {
        m_mdiArea->closeAllSubWindows();
        m_treeView->setModel(nullptr);
        delete m_img;
        m_img = nullptr;
    }

    m_img = new OpenEXRImage(filename, m_treeView);
    m_treeView->setModel(m_img->getHeaderModel());
    m_treeView->expandAll();
    m_treeView->resizeColumnToContents(0);

    // Detect if there is a root RGB or YC layer group
    if (m_img->getHeaderModel()->getLayers().size() > 0) {
        if(m_img->getHeaderModel()->getLayers()[0]->hasRGBChilds()) {
            QString title = getTitle(0, "RGB");

            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createRGBImageModel(0, m_img->getHeaderModel()->getLayers()[0]->getFullName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        } else if (m_img->getHeaderModel()->getLayers()[0]->hasYCChilds()) {
            QString title = getTitle(0, "YC");

            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createRGBImageModel(0, m_img->getHeaderModel()->getLayers()[0]->getFullName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        } else if (m_img->getHeaderModel()->getLayers()[0]->hasYChild()) {
            QString title = getTitle(0, "Y");

            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createRGBImageModel(0, m_img->getHeaderModel()->getLayers()[0]->getFullName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    }
}


void MainWindow::on_action_Open_triggered()
{
    const QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open OpenEXR Image"),
                QDir::homePath(),
                tr("Images (*.exr)")
                );

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

    if (!urls.empty())
    {
        QString filename = urls[0].toString();
        QString startFileTypeString =
            #ifdef _WIN32
                "file:///";
            #else
                "file://";
            #endif

        if (filename.startsWith(startFileTypeString))
        {
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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "afichet", "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry"      , saveGeometry());
    settings.setValue("state"         , saveState());
    settings.setValue("splitter"      , m_splitter->saveState());
    settings.endGroup();
}


void MainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "afichet", "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();
}

void MainWindow::openItem(OpenEXRHeaderItem *item)
{
    if (item->type() == "framebuffer") {
        QString title = getTitle(item->getPartID(), item->getName() + "RGB");

        // Check if the window already exists
        bool windowExists = false;
        for (auto& w: m_mdiArea->subWindowList()) {
            if (w->windowTitle() == title) {
                w->setFocus();
                windowExists = true;
                break;
            }
        }

        // If the window does not exist yet, create it
        if (!windowExists) {
            FramebufferWidget *graphicView = new FramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createImageModel(item->getPartID(), item->getName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    } else if (item->type() == "RGB framebuffer") {

    } else if (item->type() == "YC framebuffer") {
        QString title = getTitle(item->getPartID(), item->getName() + "YC");

        // Check if the window already exists
        bool windowExists = false;
        for (auto& w: m_mdiArea->subWindowList()) {
            if (w->windowTitle() == title) {
                w->setFocus();
                windowExists = true;
                break;
            }
        }

        // If the window does not exist yet, create it
        if (!windowExists) {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createYCImageModel(item->getPartID(), item->getName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    } else if (item->type() == "Luminance framebuffer") {
        QString title = getTitle(item->getPartID(), item->getName() + "Y");

        // Check if the window already exists
        bool windowExists = false;
        for (auto& w: m_mdiArea->subWindowList()) {
            if (w->windowTitle() == title) {
                w->setFocus();
                windowExists = true;
                break;
            }
        }

        // If the window does not exist yet, create it
        if (!windowExists) {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createYImageModel(item->getPartID(), item->getName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    }
}

QString MainWindow::getTitle(int partId, const QString &layer) const
{
    QString title =
            QString("Part: ") + QString::number(partId) + " " +
            QString("Layer: ") + layer;
}



void MainWindow::onDoubleClicked(const QModelIndex &index)
{
    OpenEXRHeaderItem* item = static_cast<OpenEXRHeaderItem*>(index.internalPointer());
    openItem(item);
}


void MainWindow::on_action_Tabbed_triggered()
{
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
}


void MainWindow::on_action_Cascade_triggered()
{
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->cascadeSubWindows();
}


void MainWindow::on_action_Tiled_triggered()
{
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->tileSubWindows();
}
