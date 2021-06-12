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

#include "ImageFileWidget.h"

#include <QVBoxLayout>
#include <QDir>
#include <QMdiSubWindow>
#include <QMessageBox>

#include "RGBFramebufferWidget.h"
#include "FramebufferWidget.h"

ImageFileWidget::ImageFileWidget(QWidget *parent)
    : QWidget(parent)
    , m_img(nullptr)
    , m_openedFolder(QDir::homePath())
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_splitter = new QSplitter(this);

    m_treeView = new QTreeView(m_splitter);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setExpandsOnDoubleClick(false);

    m_mdiArea = new QMdiArea(m_splitter);
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
    m_mdiArea->setBackground(QBrush(QColor(80, 80, 80)));

    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_mdiArea);

    layout->addWidget(m_splitter);

    setLayout(layout);

    // clang-format off
    connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
            this,       SLOT(onDoubleClicked(QModelIndex)));
    // clang-format on
}

ImageFileWidget::~ImageFileWidget()
{
    delete m_img;
}

void ImageFileWidget::open(const QString &filename)
{
    m_openedFolder = QFileInfo(filename).absolutePath();

    // Attempt opening the image

    OpenEXRImage *imageLoaded = nullptr;

    try {
        imageLoaded = new OpenEXRImage(filename, this);
    } catch (std::exception &e) {
        onLoadFailed(e.what());

        delete imageLoaded;

        return;
    }

    // No error so far, continue normal execution
    if (m_img) {
        m_mdiArea->closeAllSubWindows();
        m_treeView->setModel(nullptr);
        delete m_img;
        m_img = nullptr;
    }

    //    m_statusBarMessage->setText(filename);

    m_img = imageLoaded;
    m_treeView->setModel(m_img->getHeaderModel());
    m_treeView->expandAll();
    m_treeView->resizeColumnToContents(0);

    // Detect if there is a root RGB or YC layer group
    if (m_img->getHeaderModel()->getLayers().size() > 0) {
        RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
        RGBFramebufferModel * imageModel  = nullptr;

        QString title;

        if (m_img->getHeaderModel()->getLayers()[0]->hasRGBChilds()) {
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

        if (imageModel) {
            QObject::connect(
                        imageModel,
                        SIGNAL(loadFailed(QString)),
                        this,
                        SLOT(onLoadFailed(QString)));

            QObject::connect(
                        graphicView,
                        SIGNAL(openFileOnDropEvent(QString)),
                        this,
                        SLOT(open(QString)));

            graphicView->setModel(imageModel);
            imageModel->load(
                        m_img->getEXR(),
                        0,
                        m_img->getHeaderModel()->getLayers()[0]->hasAChild());

            QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
//            subWindow->resize(640, 480);
            subWindow->showMaximized();

        } else {
            delete graphicView;
        }
    }
}

void ImageFileWidget::setTabbed()
{
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
}

void ImageFileWidget::setCascade()
{
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->cascadeSubWindows();
}

void ImageFileWidget::setTiled()
{
    m_mdiArea->setViewMode(QMdiArea::SubWindowView);
    m_mdiArea->tileSubWindows();
}


QString ImageFileWidget::getTitle(int partId, const QString &layer) const
{
    return QString("Part: ") + QString::number(partId) + " "
           + QString("Layer: ") + layer;
}


void ImageFileWidget::openItem(HeaderItem *item)
{
    QString title;

    if (item->type() == "framebuffer") {
        title = getTitle(item->getPartID(), item->getItemName());
    } else if (item->type() == "RGB framebuffer") {
        title = getTitle(item->getPartID(), item->getItemName() + "RGB");
    } else if (item->type() == "YC framebuffer") {
        title = getTitle(item->getPartID(), item->getItemName() + "YC");
    } else if (item->type() == "Luminance framebuffer") {
        title = getTitle(item->getPartID(), item->getItemName() + "Y");
    } else {
        return;
    }

    // Check if the window already exists
    for (auto &w : m_mdiArea->subWindowList()) {
        if (w->windowTitle() == title) {
            w->setFocus();
            return;
        }
    }

    // If the window does not exist yet, create it
    FramebufferWidget *   graphicViewBW = nullptr;
    FramebufferModel *    imageModelBW  = nullptr;
    RGBFramebufferWidget *graphicView   = nullptr;
    RGBFramebufferModel * imageModel    = nullptr;

    QMdiSubWindow *subWindow = nullptr;

    if (item->type() == "framebuffer") {
        graphicViewBW = new FramebufferWidget(m_mdiArea);
        imageModelBW = new FramebufferModel(item->getItemName(), graphicViewBW);

        QObject::connect(
                    imageModelBW,
                    SIGNAL(loadFailed(QString)),
                    this,
                    SLOT(onLoadFailed(QString)));

        QObject::connect(
                    graphicViewBW,
                    SIGNAL(openFileOnDropEvent(QString)),
                    this,
                    SLOT(open(QString)));

        graphicViewBW->setModel(imageModelBW);
        imageModelBW->load(m_img->getEXR(), item->getPartID());

        subWindow = m_mdiArea->addSubWindow(graphicViewBW);
    } else if (item->type() == "RGB framebuffer") {
        graphicView = new RGBFramebufferWidget(m_mdiArea);
        imageModel  = new RGBFramebufferModel(
                    item->getItemName(),
                    RGBFramebufferModel::Layer_RGB,
                    graphicView);

        QObject::connect(
                    imageModel,
                    SIGNAL(loadFailed(QString)),
                    this,
                    SLOT(onLoadFailed(QString)));

        QObject::connect(
                    graphicView,
                    SIGNAL(openFileOnDropEvent(QString)),
                    this,
                    SLOT(open(QString)));

        graphicView->setModel(imageModel);
        imageModel->load(
                    m_img->getEXR(),
                    item->getPartID(),
                    m_img->getHeaderModel()->getLayers()[0]->hasAChild());

        subWindow = m_mdiArea->addSubWindow(graphicView);
    } else if (item->type() == "YC framebuffer") {
        graphicView = new RGBFramebufferWidget(m_mdiArea);
        imageModel  = new RGBFramebufferModel(
                    item->getItemName(),
                    RGBFramebufferModel::Layer_YC,
                    graphicView);

        QObject::connect(
                    imageModel,
                    SIGNAL(loadFailed(QString)),
                    this,
                    SLOT(onLoadFailed(QString)));

        QObject::connect(
                    graphicView,
                    SIGNAL(openFileOnDropEvent(QString)),
                    this,
                    SLOT(open(QString)));

        graphicView->setModel(imageModel);
        imageModel->load(
                    m_img->getEXR(),
                    item->getPartID(),
                    m_img->getHeaderModel()->getLayers()[0]->hasAChild());

        subWindow = m_mdiArea->addSubWindow(graphicView);
    } else if (item->type() == "Luminance framebuffer") {
        graphicView = new RGBFramebufferWidget(m_mdiArea);
        imageModel  = new RGBFramebufferModel(
                    item->getItemName(),
                    RGBFramebufferModel::Layer_Y,
                    graphicView);

        QObject::connect(
                    imageModel,
                    SIGNAL(loadFailed(QString)),
                    this,
                    SLOT(onLoadFailed(QString)));

        QObject::connect(
                    graphicView,
                    SIGNAL(openFileOnDropEvent(QString)),
                    this,
                    SLOT(open(QString)));

        graphicView->setModel(imageModel);
        imageModel->load(
                    m_img->getEXR(),
                    item->getPartID(),
                    m_img->getHeaderModel()->getLayers()[0]->hasAChild());

        subWindow = m_mdiArea->addSubWindow(graphicView);
    }

    if (subWindow) {
        subWindow->setWindowTitle(title);
        subWindow->resize(640, 480);
        subWindow->show();
    } else {
        // This shall never happen
        assert(0);
    }
}


void ImageFileWidget::onDoubleClicked(const QModelIndex &index)
{
    HeaderItem *item = static_cast<HeaderItem *>(index.internalPointer());
    openItem(item);
}


void ImageFileWidget::onLoadFailed(const QString &msg)
{
    std::cerr << "Loading error: " << msg.toStdString() << std::endl;

    QMessageBox msgBox;
    msgBox.setText("Error while loading the framebuffer.");
    msgBox.setInformativeText(
                "The loading process ended with the following error: " + msg);
    msgBox.exec();
}
