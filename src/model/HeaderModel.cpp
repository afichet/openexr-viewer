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

#include "HeaderModel.h"

#include <cassert>

#include <QFileInfo>

#include <OpenEXR/ImfBoxAttribute.h>
#include <OpenEXR/ImfChannelListAttribute.h>
#include <OpenEXR/ImfChromaticitiesAttribute.h>
#include <OpenEXR/ImfCompressionAttribute.h>
#include <OpenEXR/ImfDeepImageStateAttribute.h>
#include <OpenEXR/ImfDoubleAttribute.h>
#include <OpenEXR/ImfEnvmapAttribute.h>
#include <OpenEXR/ImfFloatAttribute.h>
#include <OpenEXR/ImfFloatVectorAttribute.h>
#include <OpenEXR/ImfIDManifestAttribute.h>
#include <OpenEXR/ImfIntAttribute.h>
#include <OpenEXR/ImfKeyCodeAttribute.h>
#include <OpenEXR/ImfLineOrderAttribute.h>
#include <OpenEXR/ImfMatrixAttribute.h>
#include <OpenEXR/ImfOpaqueAttribute.h>
#include <OpenEXR/ImfPreviewImageAttribute.h>
#include <OpenEXR/ImfRationalAttribute.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <OpenEXR/ImfStringVectorAttribute.h>
#include <OpenEXR/ImfTileDescriptionAttribute.h>
#include <OpenEXR/ImfTimeCodeAttribute.h>
#include <OpenEXR/ImfVecAttribute.h>

HeaderModel::HeaderModel(int n_parts, QObject *parent)
  : QAbstractItemModel(parent)
  , m_rootItem(new HeaderItem(nullptr, {tr("Name"), tr("Value"), tr("Type")}))
{
    m_headerItems.resize(n_parts);
    m_partRootLayer.resize(n_parts);
}

HeaderModel::~HeaderModel()
{
    delete m_rootItem;

    for (LayerItem *it : m_partRootLayer) {
        delete it;
    }
}

void HeaderModel::addFile(
  const Imf::MultiPartInputFile &file, const QString &filename)
{
    QString rootValue = QString::number(file.parts()) + " part";
    if (file.parts() > 1) {
        rootValue += "s";
    }

    HeaderItem *fileRoot
      = new HeaderItem(m_rootItem, {QFileInfo(filename).fileName(), rootValue, "file"});

    if (file.parts() > 1) {
        for (int i = 0; i < file.parts(); i++) {
            const Imf::Header &exrHeader = file.header(i);

            std::string partName = "Untitled part";

            if (exrHeader.hasName()) {
                partName = exrHeader.name();
            }

            QString partValue = "[" + QString::number(i) + "]";
            HeaderItem *partRoot
              = new HeaderItem(fileRoot, {partName.c_str(), partValue, "part"});

            addHeader(exrHeader, partRoot, QString::fromStdString(partName), i);
        }

    } else if (file.parts() == 1) {
        const Imf::Header &exrHeader = file.header(0);

        std::string partName = "Untitled part";

        if (exrHeader.hasName()) {
            partName = exrHeader.name();
        }

        addHeader(exrHeader, fileRoot, QString::fromStdString(partName), 0);
    } else {
        delete fileRoot;
    }
}


QVariant HeaderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    HeaderItem *item = static_cast<HeaderItem *>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags HeaderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QVariant HeaderModel::headerData(
  int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_rootItem->data(section);
    }

    return QVariant();
}

QModelIndex
HeaderModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    HeaderItem *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<HeaderItem *>(parent.internalPointer());
    }

    HeaderItem *childItem = parentItem->child(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex HeaderModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    HeaderItem *childItem  = static_cast<HeaderItem *>(index.internalPointer());
    HeaderItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int HeaderModel::rowCount(const QModelIndex &parent) const
{
    HeaderItem *parentItem;

    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<HeaderItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int HeaderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<HeaderItem *>(parent.internalPointer())
          ->columnCount();
    }

    return m_rootItem->columnCount();
}


void HeaderModel::addHeader(
  const Imf::Header &header,
  HeaderItem *       root,
  const QString      partName,
  int                partID)
{
    assert(partID < (int)m_headerItems.size());
    assert(partID < (int)m_partRootLayer.size());

    m_partRootLayer[partID] = nullptr;

    size_t n_headerFields = 0;

    for (Imf::Header::ConstIterator it = header.begin(); it != header.end();
         it++) {
        addItem(it.name(), it.attribute(), root, partName, partID);
        ++n_headerFields;
    }
}



HeaderItem *HeaderModel::addItem(
  const char *          name,
  const Imf::Attribute &attribute,
  HeaderItem *          parent,
  QString               partName,
  int                   part_number)
{
    HeaderItem *attrItem = new HeaderItem(parent);

    const char *type = attribute.typeName();

    std::stringstream ss;

    // Box
    if (strcmp(type, Imf::Box2iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::Box2iAttribute::cast(attribute);
        ss << attr.value().min << " " << attr.value().size();
    } else if (strcmp(type, Imf::Box2fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::Box2fAttribute::cast(attribute);
        ss << attr.value().min << " " << attr.value().size();
    }
    // Channel List
    else if (strcmp(type, Imf::ChannelListAttribute::staticTypeName()) == 0) {
        auto   attr         = Imf::ChannelListAttribute::cast(attribute);
        size_t channelCount = 0;

        // Sanity check
        if (m_partRootLayer[part_number]) {
            delete m_partRootLayer[part_number];
            m_partRootLayer[part_number] = nullptr;
        }

        m_partRootLayer[part_number] = new LayerItem;

        for (Imf::ChannelList::ConstIterator chIt = attr.value().begin();
             chIt != attr.value().end();
             chIt++) {
            m_partRootLayer[part_number]->addLeaf(chIt.name(), &chIt.channel());
            ++channelCount;
        }

        ss << channelCount;

        m_partRootLayer[part_number]->constructItemHierarchy(
          attrItem,
          partName,
          part_number);
    }
    // Chromaticities
    else if (
      strcmp(type, Imf::ChromaticitiesAttribute::staticTypeName()) == 0) {
        auto attr = Imf::ChromaticitiesAttribute::cast(attribute);
        ss << attr.value().red << " " << attr.value().green << " "
           << attr.value().blue << " " << attr.value().white;
    }
    // Compression
    else if (strcmp(type, Imf::CompressionAttribute::staticTypeName()) == 0) {
        auto attr = Imf::CompressionAttribute::cast(attribute);
        switch (attr.value()) {
            case Imf::Compression::NO_COMPRESSION:
                ss << "no compression";
                break;
            case Imf::Compression::RLE_COMPRESSION:
                ss << "run length encoding";
                break;
            case Imf::Compression::ZIPS_COMPRESSION:
                ss << "zlib compression, one scan line at a time";
                break;
            case Imf::Compression::ZIP_COMPRESSION:
                ss << "zlib compression, in blocks of 16 scan lines";
                break;
            case Imf::Compression::PIZ_COMPRESSION:
                ss << "piz-based wavelet compression";
                break;
            case Imf::Compression::PXR24_COMPRESSION:
                ss << "lossy 24-bit float compression";
                break;
            case Imf::Compression::B44_COMPRESSION:
                ss << "lossy 4-by-4 pixel block compression";
                break;
            case Imf::Compression::B44A_COMPRESSION:
                ss << "lossy 4-by-4 pixel block compression";
                break;
            case Imf::Compression::DWAA_COMPRESSION:
                ss << "lossy DCT based compression, in blocks of 32 scanlines";
                break;
            case Imf::Compression::DWAB_COMPRESSION:
                ss << "lossy DCT based compression, in blocks of 256 scanlines";
                break;
            default:
                ss << "unknown compression type: " << attr.value();
                break;
        }
    }
    // Deep image
    else if (
      strcmp(type, Imf::DeepImageStateAttribute::staticTypeName()) == 0) {
        auto attr = Imf::DeepImageStateAttribute::cast(attribute);
        switch (attr.value()) {
            case Imf::DeepImageState::DIS_MESSY:
                ss << "messy";
                break;
            case Imf::DeepImageState::DIS_SORTED:
                ss << "sorted";
                break;
            case Imf::DeepImageState::DIS_NON_OVERLAPPING:
                ss << "non overlapping";
                break;
            case Imf::DeepImageState::DIS_TIDY:
                ss << "tidy";
                break;
            default:
                ss << "unknown deepimage state: " << attr.value();
                break;
        }
    }
    // Double
    else if (strcmp(type, Imf::DoubleAttribute::staticTypeName()) == 0) {
        auto attr = Imf::DoubleAttribute::cast(attribute);
        ss << attr.value();
    }
    // Envmap
    else if (strcmp(type, Imf::EnvmapAttribute::staticTypeName()) == 0) {
        auto attr = Imf::EnvmapAttribute::cast(attribute);
        switch (attr.value()) {
            case Imf::Envmap::ENVMAP_LATLONG:
                ss << "Latitude-longitude environment map";
                break;
            case Imf::Envmap::ENVMAP_CUBE:
                ss << "Cube map";
                break;
            default:
                ss << "unknown envmap parametrization: " << attr.value();
                break;
        }
    }
    // Float
    else if (strcmp(type, Imf::FloatAttribute::staticTypeName()) == 0) {
        auto attr = Imf::FloatAttribute::cast(attribute);
        ss << attr.value();
    }
    // Float vector
    else if (strcmp(type, Imf::FloatVectorAttribute::staticTypeName()) == 0) {
        auto   attr       = Imf::FloatVectorAttribute::cast(attribute);
        size_t floatCount = 0;

        for (std::vector<float>::iterator fIt = attr.value().begin();
             fIt != attr.value().end();
             fIt++) {
            new HeaderItem(
              attrItem,
              {*fIt, "", "float"},
              partName,
              part_number,
              name);
            ++floatCount;
        }

        ss << floatCount;
    }
    // IDManifest
    else if (strcmp(type, Imf::IDManifestAttribute::staticTypeName()) == 0) {
        auto attr = Imf::IDManifestAttribute::cast(attribute);
    }
    // Int
    else if (strcmp(type, Imf::IntAttribute::staticTypeName()) == 0) {
        auto attr = Imf::IntAttribute::cast(attribute);
        ss << attr.value();
    }
    // Key code
    else if (strcmp(type, Imf::KeyCodeAttribute::staticTypeName()) == 0) {
        auto attr = Imf::KeyCodeAttribute::cast(attribute);
        ss << std::endl
           << "\tCount: " << attr.value().count() << std::endl
           << "\tFilm MFC code: " << attr.value().filmMfcCode() << std::endl
           << "\tFilm type: " << attr.value().filmType() << std::endl
           << "\tPerf offset: " << attr.value().perfOffset() << std::endl
           << "\tPerfs per count: " << attr.value().perfsPerCount() << std::endl
           << "\tPerfs per frame: " << attr.value().perfsPerFrame() << std::endl
           << "\tPrefix: " << attr.value().prefix();
    }
    // Line order
    else if (strcmp(type, Imf::LineOrderAttribute::staticTypeName()) == 0) {
        auto attr = Imf::LineOrderAttribute::cast(attribute);
        switch (attr.value()) {
            case Imf::LineOrder::INCREASING_Y:
                ss << "Increasing Y: first scan line has lowest y coordinate";
                break;
            case Imf::LineOrder::DECREASING_Y:
                ss << "Decreasing Y: first scan line has highest y coordinate";
                break;
            case Imf::LineOrder::RANDOM_Y:
                ss << "Random Y: tiles are written in random order";
                break;   // Only for tiled
            default:
                ss << "unknown line order: " << attr.value();
                break;
        }
    }
    // Matrix
    else if (strcmp(type, Imf::M33fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M33fAttribute::cast(attribute);
    } else if (strcmp(type, Imf::M33dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M33dAttribute::cast(attribute);
    } else if (strcmp(type, Imf::M44fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M44fAttribute::cast(attribute);
    } else if (strcmp(type, Imf::M44dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M44dAttribute::cast(attribute);
    }
    // Opaque -> When unknown
    //        else if (strcmp(type, Imf::OpaqueAttribute::staticTypeName()) == 0)
    //        {
    //            auto attr = Imf::OpaqueAttribute::cast(attribute);
    //        }
    // Preview image
    else if (strcmp(type, Imf::PreviewImageAttribute::staticTypeName()) == 0) {
        auto attr = Imf::PreviewImageAttribute::cast(attribute);
    }
    // Rational
    else if (strcmp(type, Imf::RationalAttribute::staticTypeName()) == 0) {
        auto attr = Imf::PreviewImageAttribute::cast(attribute);
    }
    // String
    else if (strcmp(type, Imf::StringAttribute::staticTypeName()) == 0) {
        auto attr = Imf::StringAttribute::cast(attribute);
        ss << attr.value();
    }
    // String vector
    else if (strcmp(type, Imf::StringVectorAttribute::staticTypeName()) == 0) {
        auto   attr        = Imf::StringVectorAttribute::cast(attribute);
        size_t stringCount = 0;

        for (std::vector<std::string>::iterator sIt = attr.value().begin();
             sIt != attr.value().end();
             sIt++) {
            // Create a child
            new HeaderItem(
              attrItem,
              {sIt->c_str(), "", "string"},
              partName,
              part_number,
              name);
            ++stringCount;
        }

        ss << stringCount;
    }
    // Tile description
    else if (
      strcmp(type, Imf::TileDescriptionAttribute::staticTypeName()) == 0) {
        auto attr = Imf::TileDescriptionAttribute::cast(attribute);
    }
    // Timecode
    else if (strcmp(type, Imf::TimeCodeAttribute::staticTypeName()) == 0) {
        auto attr = Imf::TimeCodeAttribute::cast(attribute);
        ss << attr.value().hours() << ":" << attr.value().minutes() << ":"
           << attr.value().seconds() << " f" << attr.value().frame();
    }
    // Vector
    else if (strcmp(type, Imf::V2iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2iAttribute::cast(attribute);
        ss << attr.value();
    } else if (strcmp(type, Imf::V2fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2fAttribute::cast(attribute);
        ss << attr.value();
    } else if (strcmp(type, Imf::V2dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2dAttribute::cast(attribute);
        ss << attr.value();
    } else if (strcmp(type, Imf::V3iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3iAttribute::cast(attribute);
        ss << attr.value();
    } else if (strcmp(type, Imf::V3fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3fAttribute::cast(attribute);
        ss << attr.value();
    } else if (strcmp(type, Imf::V3dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3dAttribute::cast(attribute);
        ss << attr.value();
    } else {
        ss << "Unsupported";
    }

    QVector<QVariant> itemData = {name, QString(ss.str().c_str()), type};
    attrItem->setData(itemData);
    attrItem->setItemName(name);
    attrItem->setPartName(partName);
    attrItem->setPartID(part_number);

    return attrItem;
}
