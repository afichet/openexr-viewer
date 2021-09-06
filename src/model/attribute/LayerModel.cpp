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

#include "LayerModel.h"

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfChannelList.h>

#include <QImage>
#include <QIcon>

LayerModel::LayerModel(
        Imf::MultiPartInputFile &file,
        const QString &filename,
        QObject *parent)
  : QAbstractItemModel(parent)
  , m_rootItem(new LayerItem(file))
  , m_fileHandle(file)
{
    const int nParts = file.parts();

    // To avoid having an extra item, we only add a root part for multipart files
    if (nParts > 1) {
        for (int part = 0; part < nParts; part++) {
            const Imf::Header &exrHeader = file.header(part);

            std::string partName = "Untitled part";

            if (exrHeader.hasName()) {
                partName = exrHeader.name();
            }

            LayerItem * leaf = m_rootItem->addLeaf(m_fileHandle, partName, nullptr, part);

            // Now list layers and add those to the part group
            const Imf::ChannelList &exrChannels = exrHeader.channels();

            for (Imf::ChannelList::ConstIterator it = exrChannels.begin(); it != exrChannels.end(); it++) {
                leaf->addLeaf(m_fileHandle, it.name(), &it.channel(), part);
            }
        }
    } else {
        const Imf::Header &exrHeader = file.header(0);

        // Now list layers and add those to the file group
        const Imf::ChannelList &exrChannels = exrHeader.channels();

        for (Imf::ChannelList::ConstIterator it = exrChannels.begin(); it != exrChannels.end(); it++) {
            m_rootItem->addLeaf(m_fileHandle, it.name(), &it.channel());
        }
    }

    m_rootItem->groupLayers();
//    m_rootItem->createThumbnails();
//    LayerItem::groupLayers(m_rootItem);
}


LayerModel::~LayerModel()
{
    delete m_rootItem;
}


QVariant LayerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    LayerItem *item = static_cast<LayerItem *>(index.internalPointer());

    // clang-format off
    switch (role) {
        case Qt::DecorationRole:
            switch(index.column()) {
                case LAYER:
                    switch(item->getType()) {
                        case LayerItem::R:
                        case LayerItem::G:
                        case LayerItem::B:
                        case LayerItem::A:
                        case LayerItem::Y:
                        case LayerItem::RY:
                        case LayerItem::BY:
                        case LayerItem::GENERAL:
                            return QIcon(":/svg/038-image.svg");

                        case LayerItem::RGB:
                        case LayerItem::RGBA:
                        case LayerItem::YA:
                        case LayerItem::YC:
                        case LayerItem::YCA:
                            return QIcon(":/svg/090-archive-2.svg");

                        case LayerItem::GROUP:
                        case LayerItem::PART:
                            return QIcon(":/svg/100-folder-27.svg");

                        // This shall never happen but avoid warning message from compiler
                        case LayerItem::N_LAYERTYPES:
                            return QVariant();
                    }

                    //item->getPreview();

                default:
                    return QVariant();
            }
            break;

        case Qt::DisplayRole:
            switch(index.column()) {
                case LAYER:
                    return QString::fromStdString(item->getLeafName());

                case TYPE:
                    switch(item->getType()) {
                        case LayerItem::R:       return tr("Red");
                        case LayerItem::G:       return tr("Green");
                        case LayerItem::B:       return tr("Blue");
                        case LayerItem::A:       return tr("Alpha");
                        case LayerItem::Y:       return tr("Luminance");
                        case LayerItem::RY:      return tr("Chroma R");
                        case LayerItem::BY:      return tr("Chroma B");
                        case LayerItem::RGB:     return tr("RGB");
                        case LayerItem::RGBA:    return tr("RGBA");
                        case LayerItem::YA:      return tr("Luminance Alpha");
                        case LayerItem::YC:      return tr("Luminance Chroma");
                        case LayerItem::YCA:     return tr("Luminance Chroma Alpha");
                        case LayerItem::GROUP:   return tr("Group");
                        case LayerItem::PART:    return tr("Part") + " " + QString::number(item->getPart());
                        case LayerItem::GENERAL: return tr("Framebuffer");
                        default: return QVariant();
                    }

                default:
                    return QVariant();
            }
            break;

        case Qt::ToolTipRole:
            {
                QString tooltip = "";
                tooltip += "<b>Part ID:</b> " + QString::number(item->getPart());

                // When it is a layer group, we do not want to display an empty layer name
                if (item->getType() != LayerItem::GROUP && item->getType() != LayerItem::PART) {
                    tooltip += "<br/>";
                    tooltip += "<b>Layer name:</b> " + QString::fromStdString(item->getOriginalFullName());
                }
                return tooltip;
            }
        default:
            return QVariant();
    }
    // clang-format on

    return QVariant();
}


Qt::ItemFlags LayerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}


QVariant LayerModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case LAYER:
            return "Layer";
        case TYPE:
            return "Type";
        default:
            return QVariant();
        }
    }

    return QVariant();
}


QModelIndex
LayerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    LayerItem *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<LayerItem *>(parent.internalPointer());
    }

    LayerItem *childItem = parentItem->child(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}


QModelIndex LayerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    LayerItem *childItem  = static_cast<LayerItem *>(index.internalPointer());
    LayerItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    int row = 0;//parentItem->row();
    return createIndex(row, 0, parentItem);
}


int LayerModel::rowCount(const QModelIndex &parent) const
{
    LayerItem *parentItem;

    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<LayerItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}


int LayerModel::columnCount(const QModelIndex &) const
{
    return N_LAYER_INFO;
}


