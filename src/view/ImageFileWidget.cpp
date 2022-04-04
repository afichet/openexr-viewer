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
#include <QString>

#include <model/OpenEXRImage.h>

#include "RGBFramebufferWidget.h"
#include "YFramebufferWidget.h"

ImageFileWidget::ImageFileWidget(const QString& filename, QWidget* parent)
  : QWidget(parent)
  , m_img(nullptr)
  , m_openedFolder(QDir::homePath())
  , m_isStream(false)
{
    setupLayout();

    // clang-format off
    connect(m_attributesTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this                , SLOT(onAttributeDoubleClicked(QModelIndex)));

    connect(m_layersTreeView    , SIGNAL(doubleClicked(QModelIndex)),
            this                , SLOT(onLayerDoubleClicked(QModelIndex)));
    // clang-format on


    // Open the file
    open(filename);
}


ImageFileWidget::ImageFileWidget(std::istream& stream, QWidget* parent)
  : QWidget(parent)
  , m_img(nullptr)
  , m_openedFolder(QDir::homePath())
  , m_isStream(true)
{
    setupLayout();

    // clang-format off
    connect(m_attributesTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this                , SLOT(onAttributeDoubleClicked(QModelIndex)));

    connect(m_layersTreeView    , SIGNAL(doubleClicked(QModelIndex)),
            this                , SLOT(onLayerDoubleClicked(QModelIndex)));
    // clang-format on


    // Open the file

    open(stream);
}


ImageFileWidget::~ImageFileWidget()
{
    delete m_img;
}



void ImageFileWidget::refresh()
{
    // TODO:
    // Better refresh handling: keep all window open, close those with no valid
    // layer...
    if (!m_isStream) {
        open(m_openedFilename);
    }
}



void ImageFileWidget::setTabbed()
{
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
    m_mdiArea->setTabsMovable(true);
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


void ImageFileWidget::setupLayout()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_splitterImageView  = new QSplitter(this);
    m_splitterProperties = new QSplitter(Qt::Vertical, m_splitterImageView);

    m_attributesTreeView = new QTreeView(m_splitterProperties);
    m_attributesTreeView->setAlternatingRowColors(true);
    m_attributesTreeView->setExpandsOnDoubleClick(false);
    m_attributesTreeView->setIndentation(32);

    m_layersTreeView = new QTreeView(m_splitterProperties);
    m_layersTreeView->setUniformRowHeights(true);
    m_layersTreeView->setAlternatingRowColors(true);
    m_layersTreeView->setExpandsOnDoubleClick(false);
    m_layersTreeView->setIndentation(32);

    m_mdiArea = new QMdiArea(m_splitterImageView);
    m_mdiArea->setViewMode(QMdiArea::TabbedView);
    m_mdiArea->setTabsMovable(true);
    m_mdiArea->setTabsClosable(true);
    m_mdiArea->setDocumentMode(true);
    m_mdiArea->setBackground(QBrush(QColor(80, 80, 80)));

    m_splitterProperties->addWidget(m_attributesTreeView);
    m_splitterProperties->addWidget(m_layersTreeView);

    m_splitterImageView->addWidget(m_splitterProperties);
    m_splitterImageView->addWidget(m_mdiArea);

    layout->addWidget(m_splitterImageView);

    setLayout(layout);
}


QString ImageFileWidget::getTitle(const LayerItem* item)
{
    QString layerName;

    switch (item->getType()) {
        // Color layer groups
        case LayerItem::RGB:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName())
                + "RGB";
            break;

        case LayerItem::RGBA:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName())
                + "RGBA";
            break;

        case LayerItem::YC:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName())
                + "YC";
            break;

        case LayerItem::YCA:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName())
                + "YCA";
            break;

        case LayerItem::YA:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName())
                + "YA";
            break;

        // Individual layers
        case LayerItem::R:
        case LayerItem::G:
        case LayerItem::B:
        case LayerItem::A:
        case LayerItem::Y:
        case LayerItem::RY:
        case LayerItem::BY:
        case LayerItem::GENERAL:
            layerName
              = "Layer: " + QString::fromStdString(item->getOriginalFullName());
            break;

        case LayerItem::GROUP:
        case LayerItem::PART:
            layerName = "";
            break;

        // This shall never happen
        case LayerItem::N_LAYERTYPES:
            assert(0);
            break;
    }


    // check if there is a part name

    QString partName;

    if (item->getPart() >= 0) {
        partName += tr("Part:") + " " + QString::number(item->getPart());

        if (item->hasPartName()) {
            partName
              += " (" + QString::fromStdString(item->getPartName()) + ")";
        }
    } else {
        // Single part file
        if (item->hasPartName()) {
            partName += QString::fromStdString(item->getPartName());
        }
    }

    return partName + " " + layerName;
}


void ImageFileWidget::openAttribute(const HeaderItem* item)
{
    if (item->getLayerItem() != nullptr) {
        openLayer(item->getLayerItem());
    }
}


void ImageFileWidget::openLayer(const LayerItem* item)
{
    QString title = getTitle(item);

    // Check if the window already exists
    for (auto& w : m_mdiArea->subWindowList()) {
        if (w->windowTitle() == title) {
            w->setFocus();
            return;
        }
    }

    // If the window does not exist yet, create it
    YFramebufferWidget*   graphicViewBW = nullptr;
    YFramebufferModel*    imageModelBW  = nullptr;
    RGBFramebufferWidget* graphicView   = nullptr;
    RGBFramebufferModel*  imageModel    = nullptr;

    QMdiSubWindow* subWindow = nullptr;

    switch (item->getType()) {
        case LayerItem::RGB:
        case LayerItem::RGBA:
            graphicView = new RGBFramebufferWidget(m_mdiArea);
            imageModel  = new RGBFramebufferModel(
              item->getOriginalFullName(),
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
              SLOT(onOpenFileDropEvent(QString)));

            graphicView->setModel(imageModel);

            imageModel->load(
              m_img->getEXR(),
              item->getPart(),
              item->getType() == LayerItem::RGBA);

            subWindow = m_mdiArea->addSubWindow(graphicView);

            break;

        case LayerItem::YCA:
        case LayerItem::YC:
            graphicView = new RGBFramebufferWidget(m_mdiArea);
            imageModel  = new RGBFramebufferModel(
              item->getOriginalFullName(),
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
              SLOT(onOpenFileDropEvent(QString)));

            graphicView->setModel(imageModel);

            imageModel->load(
              m_img->getEXR(),
              item->getPart(),
              item->getType() == LayerItem::YCA);

            subWindow = m_mdiArea->addSubWindow(graphicView);
            break;

        case LayerItem::R:
        case LayerItem::G:
        case LayerItem::B:
        case LayerItem::Y:
        case LayerItem::YA:
            graphicView = new RGBFramebufferWidget(m_mdiArea);
            imageModel  = new RGBFramebufferModel(
              item->getOriginalFullName(),
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
              SLOT(onOpenFileDropEvent(QString)));

            graphicView->setModel(imageModel);

            imageModel->load(
              m_img->getEXR(),
              item->getPart(),
              item->getType() == LayerItem::YA);

            subWindow = m_mdiArea->addSubWindow(graphicView);
            break;


        case LayerItem::A:
        case LayerItem::RY:
        case LayerItem::BY:
        case LayerItem::GENERAL:
            graphicViewBW = new YFramebufferWidget(m_mdiArea);
            imageModelBW  = new YFramebufferModel(
              item->getOriginalFullName(),
              graphicViewBW);

            QObject::connect(
              imageModelBW,
              SIGNAL(loadFailed(QString)),
              this,
              SLOT(onLoadFailed(QString)));

            QObject::connect(
              graphicViewBW,
              SIGNAL(openFileOnDropEvent(QString)),
              this,
              SLOT(onOpenFileDropEvent(QString)));

            graphicViewBW->setModel(imageModelBW);

            imageModelBW->load(m_img->getEXR(), item->getPart());

            subWindow = m_mdiArea->addSubWindow(graphicViewBW);
            break;

        case LayerItem::PART:
        case LayerItem::GROUP:
        case LayerItem::N_LAYERTYPES:
            break;
    }

    if (subWindow) {
        subWindow->setWindowTitle(title);

        switch (m_mdiArea->viewMode()) {
            case QMdiArea::TabbedView:
                subWindow->showMaximized();
                break;

            case QMdiArea::SubWindowView:
                subWindow->resize(800, 600);
                subWindow->show();
                break;
        }
    }
}


void ImageFileWidget::open(const QString& filename)
{
    assert(!m_isStream);

    // Open the file
    m_openedFilename = filename;
    m_openedFolder   = QFileInfo(m_openedFilename).absolutePath();

    // Attempt opening the image
    OpenEXRImage* imageLoaded = nullptr;

    try {
        imageLoaded = new OpenEXRImage(m_openedFilename, this);
    } catch (std::exception& e) {
        onLoadFailed(e.what());

        delete imageLoaded;

        return;
    }

    // No error so far, continue normal execution
    if (m_img) {
        m_mdiArea->closeAllSubWindows();
        m_attributesTreeView->setModel(nullptr);
        m_layersTreeView->setModel(nullptr);
        delete m_img;
        m_img = nullptr;
    }

    //    m_statusBarMessage->setText(filename);

    m_img = imageLoaded;

    afterOpen();
}


void ImageFileWidget::open(std::istream& stream)
{
    assert(m_isStream);

    // Attempt opening the image
    OpenEXRImage* imageLoaded = nullptr;

    try {
        imageLoaded = new OpenEXRImage(stream, this);
    } catch (std::exception& e) {
        onLoadFailed(e.what());

        delete imageLoaded;

        return;
    }

    // No error so far, continue normal execution
    if (m_img) {
        m_mdiArea->closeAllSubWindows();
        m_attributesTreeView->setModel(nullptr);
        m_layersTreeView->setModel(nullptr);
        delete m_img;
        m_img = nullptr;
    }

    //    m_statusBarMessage->setText(filename);

    m_img = imageLoaded;

    afterOpen();
}


void ImageFileWidget::afterOpen()
{
    m_attributesTreeView->setModel(m_img->getHeaderModel());
    m_attributesTreeView->expandAll();
    m_attributesTreeView->resizeColumnToContents(0);

    m_layersTreeView->setModel(m_img->getLayerModel());
    m_layersTreeView->expandAll();
    m_layersTreeView->resizeColumnToContents(0);

    openDefaultLayer();
}


void ImageFileWidget::openDefaultLayer()
{
    // Detect if there is a root RGB or YC layer group
    LayerItem const* r = m_img->getLayerModel()->getRoot();

    if (r != nullptr) {
        const LayerItem* child = nullptr;

        child = r->child(LayerItem::RGBA);
        if (child) {
            openLayer(child);
            return;
        }

        child = r->child(LayerItem::RGB);
        if (child) {
            openLayer(child);
            return;
        }

        child = r->child(LayerItem::YCA);
        if (child) {
            openLayer(child);
            return;
        }

        child = r->child(LayerItem::YC);
        if (child) {
            openLayer(child);
            return;
        }

        child = r->child(LayerItem::YA);
        if (child) {
            openLayer(child);
            return;
        }

        child = r->child(LayerItem::Y);
        if (child) {
            openLayer(child);
            return;
        }

        // When all children are parts, try to find a part with a displayable layer
        // TODO: factorize the code
        for (LayerItem* rr : r->children()) {
            child = rr->child(LayerItem::RGBA);
            if (child) {
                openLayer(child);
                return;
            }

            child = rr->child(LayerItem::RGB);
            if (child) {
                openLayer(child);
                return;
            }

            child = rr->child(LayerItem::YCA);
            if (child) {
                openLayer(child);
                return;
            }

            child = rr->child(LayerItem::YC);
            if (child) {
                openLayer(child);
                return;
            }

            child = rr->child(LayerItem::YA);
            if (child) {
                openLayer(child);
                return;
            }

            child = rr->child(LayerItem::Y);
            if (child) {
                openLayer(child);
                return;
            }
        }
    }
}


void ImageFileWidget::onAttributeDoubleClicked(const QModelIndex& index)
{
    HeaderItem* item = static_cast<HeaderItem*>(index.internalPointer());
    openAttribute(item);
}


void ImageFileWidget::onLayerDoubleClicked(const QModelIndex& index)
{
    LayerItem* item = static_cast<LayerItem*>(index.internalPointer());
    openLayer(item);
}


void ImageFileWidget::onLoadFailed(const QString& msg)
{
    std::cerr << "Loading error: " << msg.toStdString() << std::endl;

    QMessageBox msgBox;
    msgBox.setText(tr("Error while loading the framebuffer."));
    msgBox.setInformativeText(
      tr("The loading process ended with the following error:") + " " + msg);
    msgBox.exec();
}


void ImageFileWidget::onOpenFileDropEvent(const QString& filename)
{
    emit openFileOnDropEvent(filename);
}
