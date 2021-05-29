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

    HeaderItem *fileRoot = new HeaderItem(
      m_rootItem,
      {QFileInfo(filename).fileName(), rootValue, "file"});

    const int nParts = file.parts();

    if (nParts > 1) {
        for (int i = 0; i < nParts; i++) {
            const Imf::Header &exrHeader = file.header(i);

            std::string partName = "Untitled part";

            if (exrHeader.hasName()) {
                partName = exrHeader.name();
            }

            QString     partValue = "[" + QString::number(i) + "]";
            HeaderItem *partRoot
              = new HeaderItem(fileRoot, {partName.c_str(), partValue, "part"});

            addHeader(exrHeader, partRoot, QString::fromStdString(partName), i);
        }
    } else if (nParts == 1) {
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

#define CALL_FOR_CLASS(_name, _attribute, _parent, _partName, _partID, _class) \
    if (strcmp(_attribute.typeName(), Imf::_class::staticTypeName()) == 0) {   \
        auto typedAttr = Imf::_class::cast(_attribute);                        \
        return addItem(_name, typedAttr, _parent, _partName, _partID);         \
    }

HeaderItem *HeaderModel::addItem(
  const char *          name,
  const Imf::Attribute &attribute,
  HeaderItem *          parent,
  QString               partName,
  int                   part_number)
{
    // Try to see if we have a function to handle such attribute type
    // clang-format off
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, Box2iAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, Box2fAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, ChannelListAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, ChromaticitiesAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, CompressionAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, DoubleAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, EnvmapAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, FloatAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, FloatVectorAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, IDManifestAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, IntAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, KeyCodeAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, LineOrderAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, M33fAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, M33dAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, M44fAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, M44dAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, PreviewImageAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, RationalAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, StringAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, StringVectorAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, TileDescriptionAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, TimeCodeAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V2iAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V2fAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V2dAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V3iAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V3fAttribute);
    CALL_FOR_CLASS(name, attribute, parent, partName, part_number, V3dAttribute);
    // Opaque -> unknown
    // CALL_FOR_CLASS(name, attribute, parent, partName, part_number, OpaqueAttribute);
    // clang-format on

    // We've tried everything we knew so far... this is an unknown attribute
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, "Unsupported", attribute.typeName()},
      partName,
      part_number,
      name);


    return attrItem;
}


// Box2i
HeaderItem *HeaderModel::addItem(
  const char *               name,
  const Imf::Box2iAttribute &attr,
  HeaderItem *               parent,
  QString                    partName,
  int                        part_number)
{
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, "", Imf::Box2iAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    std::stringstream sMin;
    sMin << attr.value().min;

    new HeaderItem(
      attrItem,
      {"min", sMin.str().c_str(), "vec2i"},
      partName,
      part_number,
      name);

    std::stringstream sMax;
    sMax << attr.value().max;

    new HeaderItem(
      attrItem,
      {"max", sMax.str().c_str(), "vec2i"},
      partName,
      part_number,
      name);

    std::stringstream sWidth;
    sWidth << attr.value().size().x;

    new HeaderItem(
      attrItem,
      {"width", sWidth.str().c_str(), "int"},
      partName,
      part_number,
      name);

    std::stringstream sHeight;
    sHeight << attr.value().size().y;

    new HeaderItem(
      attrItem,
      {"height", sHeight.str().c_str(), "int"},
      partName,
      part_number,
      name);

    return attrItem;
}

// Box2f
HeaderItem *HeaderModel::addItem(
  const char *               name,
  const Imf::Box2fAttribute &attr,
  HeaderItem *               parent,
  QString                    partName,
  int                        part_number)
{
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, "", Imf::Box2fAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    std::stringstream sMin;
    sMin << attr.value().min;

    new HeaderItem(
      attrItem,
      {"min", sMin.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    std::stringstream sMax;
    sMax << attr.value().max;

    new HeaderItem(
      attrItem,
      {"max", sMax.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    std::stringstream sWidth;
    sWidth << attr.value().size().x;

    new HeaderItem(
      attrItem,
      {"width", sWidth.str().c_str(), "float"},
      partName,
      part_number,
      name);

    std::stringstream sHeight;
    sHeight << attr.value().size().y;

    new HeaderItem(
      attrItem,
      {"height", sHeight.str().c_str(), "float"},
      partName,
      part_number,
      name);


    return attrItem;
}

// Channel list
HeaderItem *HeaderModel::addItem(
  const char *                     name,
  const Imf::ChannelListAttribute &attr,
  HeaderItem *                     parent,
  QString                          partName,
  int                              part_number)
{
    HeaderItem *attrItem = new HeaderItem(parent);

    std::stringstream ss;

    // Channel List
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


    QVector<QVariant> itemData = {
      name,
      QString(ss.str().c_str()),
      Imf::Box2fAttribute::staticTypeName()};
    attrItem->setData(itemData);
    attrItem->setItemName(name);
    attrItem->setPartName(partName);
    attrItem->setPartID(part_number);

    return attrItem;
}

// Channel List

// Chromaticities
HeaderItem *HeaderModel::addItem(
  const char *                        name,
  const Imf::ChromaticitiesAttribute &attr,
  HeaderItem *                        parent,
  QString                             partName,
  int                                 part_number)
{
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, "", Imf::ChromaticitiesAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    std::stringstream sRed;
    sRed << attr.value().red;

    new HeaderItem(
      attrItem,
      {"red", sRed.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    std::stringstream sGreen;
    sGreen << attr.value().green;

    new HeaderItem(
      attrItem,
      {"green", sGreen.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    std::stringstream sBlue;
    sBlue << attr.value().blue;

    new HeaderItem(
      attrItem,
      {"blue", sBlue.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    std::stringstream sWhite;
    sWhite << attr.value().white;

    new HeaderItem(
      attrItem,
      {"white", sWhite.str().c_str(), "vec2f"},
      partName,
      part_number,
      name);

    return attrItem;
}

// Compression
HeaderItem *HeaderModel::addItem(
  const char *                     name,
  const Imf::CompressionAttribute &attr,
  HeaderItem *                     parent,
  QString                          partName,
  int                              part_number)
{
    std::stringstream ss;
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
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::CompressionAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Deep image state
HeaderItem *HeaderModel::addItem(
  const char *                        name,
  const Imf::DeepImageStateAttribute &attr,
  HeaderItem *                        parent,
  QString                             partName,
  int                                 part_number)
{
    std::stringstream ss;
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

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::DeepImageStateAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Double
HeaderItem *HeaderModel::addItem(
  const char *                name,
  const Imf::DoubleAttribute &attr,
  HeaderItem *                parent,
  QString                     partName,
  int                         part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::DoubleAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Envmap
HeaderItem *HeaderModel::addItem(
  const char *                name,
  const Imf::EnvmapAttribute &attr,
  HeaderItem *                parent,
  QString                     partName,
  int                         part_number)
{
    std::stringstream ss;
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

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::EnvmapAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Float
HeaderItem *HeaderModel::addItem(
  const char *               name,
  const Imf::FloatAttribute &attr,
  HeaderItem *               parent,
  QString                    partName,
  int                        part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::FloatAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Float vector
HeaderItem *HeaderModel::addItem(
  const char *                     name,
  const Imf::FloatVectorAttribute &attr,
  HeaderItem *                     parent,
  QString                          partName,
  int                              part_number)
{
    HeaderItem *attrItem = new HeaderItem(parent);

    size_t            floatCount = 0;
    std::stringstream ss;

    for (auto fIt = attr.value().cbegin(); fIt != attr.value().cend(); fIt++) {
        new HeaderItem(
          attrItem,
          {*fIt, "", "float"},
          partName,
          part_number,
          name);
        ++floatCount;
    }

    ss << floatCount;

    QVector<QVariant> itemData = {
      name,
      QString(ss.str().c_str()),
      Imf::FloatVectorAttribute::staticTypeName()};
    attrItem->setData(itemData);
    attrItem->setItemName(name);
    attrItem->setPartName(partName);
    attrItem->setPartID(part_number);

    return attrItem;
}


// IDManifest
HeaderItem *HeaderModel::addItem(
  const char *                    name,
  const Imf::IDManifestAttribute &attr,
  HeaderItem *                    parent,
  QString                         partName,
  int                             part_number)
{
    std::stringstream ss;
    // TODO!
    //        ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::IDManifestAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Int
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::IntAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::IntAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Key code
HeaderItem *HeaderModel::addItem(
  const char *                 name,
  const Imf::KeyCodeAttribute &attr,
  HeaderItem *                 parent,
  QString                      partName,
  int                          part_number)
{
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, "", Imf::KeyCodeAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Film NFC code", attr.value().filmMfcCode(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Film type", attr.value().filmType(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Prefix", attr.value().prefix(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Count", attr.value().count(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Perf offset", attr.value().perfOffset(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Prefs per frame", attr.value().perfsPerFrame(), "int"},
      partName,
      part_number,
      name);

    new HeaderItem(
      attrItem,
      {"Perfs per count", attr.value().perfsPerCount(), "int"},
      partName,
      part_number,
      name);

    return attrItem;
}

// Line order
HeaderItem *HeaderModel::addItem(
  const char *                   name,
  const Imf::LineOrderAttribute &attr,
  HeaderItem *                   parent,
  QString                        partName,
  int                            part_number)
{
    std::stringstream ss;
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
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::LineOrderAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Matrix33f
HeaderItem *HeaderModel::addItem(
  const char *              name,
  const Imf::M33fAttribute &attr,
  HeaderItem *              parent,
  QString                   partName,
  int                       part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::M33fAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Matrix33d
HeaderItem *HeaderModel::addItem(
  const char *              name,
  const Imf::M33dAttribute &attr,
  HeaderItem *              parent,
  QString                   partName,
  int                       part_number)
{
    std::stringstream ss;
    // TODO
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::M33dAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Matrix44f
HeaderItem *HeaderModel::addItem(
  const char *              name,
  const Imf::M44fAttribute &attr,
  HeaderItem *              parent,
  QString                   partName,
  int                       part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::M44fAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Matrix44d
HeaderItem *HeaderModel::addItem(
  const char *              name,
  const Imf::M44dAttribute &attr,
  HeaderItem *              parent,
  QString                   partName,
  int                       part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::M44dAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// Preview image
HeaderItem *HeaderModel::addItem(
  const char *                      name,
  const Imf::PreviewImageAttribute &attr,
  HeaderItem *                      parent,
  QString                           partName,
  int                               part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::PreviewImageAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Rational
HeaderItem *HeaderModel::addItem(
  const char *                  name,
  const Imf::RationalAttribute &attr,
  HeaderItem *                  parent,
  QString                       partName,
  int                           part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::RationalAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// String
HeaderItem *HeaderModel::addItem(
  const char *                name,
  const Imf::StringAttribute &attr,
  HeaderItem *                parent,
  QString                     partName,
  int                         part_number)
{
    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, attr.value().c_str(), Imf::StringAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}

// String vector
HeaderItem *HeaderModel::addItem(
  const char *                      name,
  const Imf::StringVectorAttribute &attr,
  HeaderItem *                      parent,
  QString                           partName,
  int                               part_number)
{
    HeaderItem *      attrItem = new HeaderItem(parent);
    std::stringstream ss;
    // TODO

    size_t stringCount = 0;

    for (auto sIt = attr.value().cbegin(); sIt != attr.value().cend(); sIt++) {
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

    QVector<QVariant> itemData = {
      name,
      QString(ss.str().c_str()),
      Imf::StringVectorAttribute::staticTypeName()};
    attrItem->setData(itemData);
    attrItem->setItemName(name);
    attrItem->setPartName(partName);
    attrItem->setPartID(part_number);

    return attrItem;
}


// Tile description
HeaderItem *HeaderModel::addItem(
  const char *                         name,
  const Imf::TileDescriptionAttribute &attr,
  HeaderItem *                         parent,
  QString                              partName,
  int                                  part_number)
{
    std::stringstream ss;
    // TODO

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::TileDescriptionAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Timecode
HeaderItem *HeaderModel::addItem(
  const char *                  name,
  const Imf::TimeCodeAttribute &attr,
  HeaderItem *                  parent,
  QString                       partName,
  int                           part_number)
{
    std::stringstream ss;
    // TODO
    ss << attr.value().hours() << ":" << attr.value().minutes() << ":"
       << attr.value().seconds() << " f" << attr.value().frame();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::TimeCodeAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector2i
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V2iAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V2iAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector2f
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V2fAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V2fAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector2d
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V2dAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V2dAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector3i
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V3iAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V3iAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector3f
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V3fAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V3fAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}


// Vector3d
HeaderItem *HeaderModel::addItem(
  const char *             name,
  const Imf::V3dAttribute &attr,
  HeaderItem *             parent,
  QString                  partName,
  int                      part_number)
{
    std::stringstream ss;
    ss << attr.value();

    HeaderItem *attrItem = new HeaderItem(
      parent,
      {name, ss.str().c_str(), Imf::V3dAttribute::staticTypeName()},
      partName,
      part_number,
      name);

    return attrItem;
}
