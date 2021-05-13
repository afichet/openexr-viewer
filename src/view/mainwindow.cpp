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
        RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
        RGBFramebufferModel* imageModel = nullptr;

        QString title;

        if(m_img->getHeaderModel()->getLayers()[0]->hasRGBChilds()) {
            title = getTitle(0, "RGB");

            imageModel = new RGBFramebufferModel(
                m_img->getHeaderModel()->getLayers()[0]->getFullName(), 
                RGBFramebufferModel::Layer_RGB,
                graphicView);
        } else if (m_img->getHeaderModel()->getLayers()[0]->hasYCChilds()) {
            title = getTitle(0, "YC");

            imageModel = new RGBFramebufferModel(
                m_img->getHeaderModel()->getLayers()[0]->getFullName(), 
                RGBFramebufferModel::Layer_YC,
                graphicView);
        } else if (m_img->getHeaderModel()->getLayers()[0]->hasYChild()) {
            title = getTitle(0, "Y");

            imageModel = new RGBFramebufferModel(
                m_img->getHeaderModel()->getLayers()[0]->getFullName(), 
                RGBFramebufferModel::Layer_Y,
                graphicView);
        }

        graphicView->setModel(imageModel);
        imageModel->load(m_img->getEXR(), 0, m_img->getHeaderModel()->getLayers()[0]->hasAChild());

        QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

        subWindow->setWindowTitle(title);
        subWindow->show();
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
    QString title;

    if (item->type() == "framebuffer") {
        title = getTitle(item->getPartID(), item->getName());
    } else if (item->type() == "RGB framebuffer") {
        title = getTitle(item->getPartID(), item->getName() + "RGB");
    } else if (item->type() == "YC framebuffer") {
        title = getTitle(item->getPartID(), item->getName() + "YC");
    } else if (item->type() == "Luminance framebuffer") {
        title = getTitle(item->getPartID(), item->getName() + "Y");
    } else {
        return;
    }

    // Check if the window already exists
    bool windowExists = false;

    for (auto& w: m_mdiArea->subWindowList()) {
        if (w->windowTitle() == title) {
            w->setFocus();
            windowExists = true;
            return;
        }
    }

    // If the window does not exist yet, create it
    if (!windowExists) {
        QMdiSubWindow* subWindow = nullptr;

        if (item->type() == "framebuffer") {
            FramebufferWidget *graphicView = new FramebufferWidget(m_mdiArea);
            FramebufferModel *imageModel = new FramebufferModel(
                item->getName(), 
                graphicView);
            
            graphicView->setModel(imageModel);
            imageModel->load(m_img->getEXR(), item->getPartID());

            subWindow = m_mdiArea->addSubWindow(graphicView);
        } else if (item->type() == "RGB framebuffer") {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            RGBFramebufferModel* imageModel = new RGBFramebufferModel(
                item->getName(), 
                RGBFramebufferModel::Layer_RGB,
                graphicView);

            graphicView->setModel(imageModel);
            imageModel->load(m_img->getEXR(), item->getPartID(), m_img->getHeaderModel()->getLayers()[0]->hasAChild());
            
            subWindow = m_mdiArea->addSubWindow(graphicView);
        } else if (item->type() == "YC framebuffer") {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            RGBFramebufferModel* imageModel = new RGBFramebufferModel(
                item->getName(), 
                RGBFramebufferModel::Layer_YC,
                graphicView);

            graphicView->setModel(imageModel);
            imageModel->load(m_img->getEXR(), item->getPartID(), m_img->getHeaderModel()->getLayers()[0]->hasAChild());
            
            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);
        } else if (item->type() == "Luminance framebuffer") {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            RGBFramebufferModel* imageModel = new RGBFramebufferModel(
                item->getName(), 
                RGBFramebufferModel::Layer_Y,
                graphicView);

            graphicView->setModel(imageModel);
            imageModel->load(m_img->getEXR(), item->getPartID(), m_img->getHeaderModel()->getLayers()[0]->hasAChild());
            
            subWindow = m_mdiArea->addSubWindow(graphicView);
        }

        subWindow->setWindowTitle(title);
        subWindow->show();
    }
}


QString MainWindow::getTitle(int partId, const QString &layer) const
{
    return
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
