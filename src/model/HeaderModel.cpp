#include "HeaderModel.h"


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
    , m_rootItem(new OpenEXRHeaderItem(
                    nullptr,
                    {tr("Name"), tr("Value"), tr("Type")}))
{
    m_headerItems.resize(n_parts);
    m_partRootLayer.resize(n_parts);
}

HeaderModel::~HeaderModel()
{
    delete m_rootItem;

    for (OpenEXRLayerItem* it: m_partRootLayer) {
        delete it;
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

    OpenEXRHeaderItem *item = static_cast<OpenEXRHeaderItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags HeaderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QVariant HeaderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_rootItem->data(section);
    }

    return QVariant();
}

QModelIndex HeaderModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    OpenEXRHeaderItem *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<OpenEXRHeaderItem*>(parent.internalPointer());
    }

    OpenEXRHeaderItem *childItem = parentItem->child(row);

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

    OpenEXRHeaderItem *childItem = static_cast<OpenEXRHeaderItem*>(index.internalPointer());
    OpenEXRHeaderItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int HeaderModel::rowCount(const QModelIndex &parent) const
{
    OpenEXRHeaderItem *parentItem;

    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<OpenEXRHeaderItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int HeaderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return static_cast<OpenEXRHeaderItem*>(parent.internalPointer())->columnCount();
    }

    return m_rootItem->columnCount();
}

void HeaderModel::addHeader(const Imf_3_0::Header &header, int part)
{
    assert(part < (int)m_headerItems.size());
    assert(part < (int)m_partRootLayer.size());

    OpenEXRHeaderItem *child = new OpenEXRHeaderItem(
                m_rootItem,
                {"OpenEXR Image", part, "part"}
                );


    m_partRootLayer[part] = nullptr;

    size_t n_headerFields = 0;

    for (Imf::Header::ConstIterator it = header.begin(); it != header.end(); it++) {
        addItem(it.name(), it.attribute(), child, part);
        ++n_headerFields;
    }

    // Just in case there is no header info at all...
    // free allocated memory
    if (n_headerFields == 0) {
        delete child;
    }
}

OpenEXRHeaderItem *HeaderModel::addItem(
        const char *name,
        const Imf_3_0::Attribute &attribute,
        OpenEXRHeaderItem *parent,
        int part_number)
{
    OpenEXRHeaderItem *attrItem = new OpenEXRHeaderItem(parent);

    const char * type = attribute.typeName();

    std::stringstream ss;

    // Box
    if (strcmp(type, Imf::Box2iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::Box2iAttribute::cast(attribute);
        ss << attr.value().min << " " << attr.value().size();
    }
    else if (strcmp(type, Imf::Box2fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::Box2fAttribute::cast(attribute);
        ss << attr.value().min << " " << attr.value().size();
    }
    // Channel List
    else if (strcmp(type, Imf::ChannelListAttribute::staticTypeName()) == 0) {
        auto attr = Imf::ChannelListAttribute::cast(attribute);
        size_t channelCount = 0;

        // Sanity check
        if (m_partRootLayer[part_number]) {
            delete m_partRootLayer[part_number];
            m_partRootLayer[part_number] = nullptr;
        }

        m_partRootLayer[part_number] = new OpenEXRLayerItem;

        for (Imf::ChannelList::ConstIterator chIt = attr.value().begin(); chIt != attr.value().end(); chIt++) {
            m_partRootLayer[part_number]->addLeaf(chIt.name(), &chIt.channel());
            ++channelCount;
        }

        ss << channelCount;

        m_partRootLayer[part_number]->constructItemHierarchy(attrItem, part_number);
    }
    // Chromaticities
    else if (strcmp(type, Imf::ChromaticitiesAttribute::staticTypeName()) == 0) {
        auto attr = Imf::ChromaticitiesAttribute::cast(attribute);
        ss << attr.value().red << " " << attr.value().green << " " << attr.value().blue << " " << attr.value().white;
    }
    // Compression
    else if (strcmp(type, Imf::CompressionAttribute::staticTypeName()) == 0) {
        auto attr = Imf::CompressionAttribute::cast(attribute);
        switch (attr.value()) {
        case Imf::Compression::NO_COMPRESSION:    ss << "no compression"; break;
        case Imf::Compression::RLE_COMPRESSION:   ss << "run length encoding"; break;
        case Imf::Compression::ZIPS_COMPRESSION:  ss << "zlib compression, one scan line at a time"; break;
        case Imf::Compression::ZIP_COMPRESSION:   ss << "zlib compression, in blocks of 16 scan lines"; break;
        case Imf::Compression::PIZ_COMPRESSION:   ss << "piz-based wavelet compression"; break;
        case Imf::Compression::PXR24_COMPRESSION: ss << "lossy 24-bit float compression"; break;
        case Imf::Compression::B44_COMPRESSION:   ss << "lossy 4-by-4 pixel block compression"; break;
        case Imf::Compression::B44A_COMPRESSION:  ss << "lossy 4-by-4 pixel block compression"; break;
        case Imf::Compression::DWAA_COMPRESSION:  ss << "lossy DCT based compression, in blocks of 32 scanlines"; break;
        case Imf::Compression::DWAB_COMPRESSION:  ss << "lossy DCT based compression, in blocks of 256 scanlines"; break;
        default: ss << "unknown compression type: " << attr.value(); break;
        }
    }
    // Deep image
    else if (strcmp(type, Imf::DeepImageStateAttribute::staticTypeName()) == 0) {
        auto attr = Imf::DeepImageStateAttribute::cast(attribute);
        switch (attr.value()) {
        case Imf::DeepImageState::DIS_MESSY: ss << "messy"; break;
        case Imf::DeepImageState::DIS_SORTED: ss << "sorted"; break;
        case Imf::DeepImageState::DIS_NON_OVERLAPPING: ss << "non overlapping"; break;
        case Imf::DeepImageState::DIS_TIDY: ss << "tidy"; break;
        default: ss << "unknown deepimage state: " << attr.value(); break;
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
        switch(attr.value()) {
        case Imf::Envmap::ENVMAP_LATLONG: ss << "Latitude-longitude environment map"; break;
        case Imf::Envmap::ENVMAP_CUBE: ss << "Cube map"; break;
        default: ss << "unknown envmap parametrization: " << attr.value(); break;
        }
    }
    // Float
    else if (strcmp(type, Imf::FloatAttribute::staticTypeName()) == 0) {
        auto attr = Imf::FloatAttribute::cast(attribute);
        ss << attr.value();
    }
    // Float vector
    else if (strcmp(type, Imf::FloatVectorAttribute::staticTypeName()) == 0) {
        auto attr = Imf::FloatVectorAttribute::cast(attribute);
        size_t floatCount = 0;

        for (std::vector<float>::iterator fIt = attr.value().begin(); fIt != attr.value().end(); fIt++) {
            new OpenEXRHeaderItem(attrItem, {*fIt, "", "float"}, name, part_number);
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
        ss << std::endl << "\tCount: " << attr.value().count()
           << std::endl << "\tFilm MFC code: " << attr.value().filmMfcCode()
           << std::endl << "\tFilm type: " << attr.value().filmType()
           << std::endl << "\tPerf offset: " << attr.value().perfOffset()
           << std::endl << "\tPerfs per count: " << attr.value().perfsPerCount()
           << std::endl << "\tPerfs per frame: " << attr.value().perfsPerFrame()
           << std::endl << "\tPrefix: " << attr.value().prefix();
    }
    // Line order
    else if (strcmp(type, Imf::LineOrderAttribute::staticTypeName()) == 0) {
        auto attr = Imf::LineOrderAttribute::cast(attribute);
        switch (attr.value()) {
        case Imf::LineOrder::INCREASING_Y: ss << "Increasing Y: first scan line has lowest y coordinate"; break;
        case Imf::LineOrder::DECREASING_Y: ss << "Decreasing Y: first scan line has highest y coordinate"; break;
        case Imf::LineOrder::RANDOM_Y: ss << "Random Y: tiles are written in random order"; break; // Only for tiled
        default: ss << "unknown line order: " << attr.value(); break;
        }
    }
    // Matrix
    else if (strcmp(type, Imf::M33fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M33fAttribute::cast(attribute);
    }
    else if (strcmp(type, Imf::M33dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M33dAttribute::cast(attribute);
    }
    else if (strcmp(type, Imf::M44fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M44fAttribute::cast(attribute);
    }
    else if (strcmp(type, Imf::M44dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::M44dAttribute::cast(attribute);
    }
    // Opaque -> When unknown
    //        else if (strcmp(type, Imf::OpaqueAttribute::staticTypeName()) == 0) {
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
        auto attr = Imf::StringVectorAttribute::cast(attribute);
        size_t stringCount = 0;

        for (std::vector<std::string>::iterator sIt = attr.value().begin(); sIt != attr.value().end(); sIt++) {
            // Create a child
            new OpenEXRHeaderItem(attrItem, {sIt->c_str(), "", "string"}, name, part_number);
            ++stringCount;
        }

        ss << stringCount;
    }
    // Tile description
    else if (strcmp(type, Imf::TileDescriptionAttribute::staticTypeName()) == 0) {
        auto attr = Imf::TileDescriptionAttribute::cast(attribute);
    }
    // Timecode
    else if (strcmp(type, Imf::TimeCodeAttribute::staticTypeName()) == 0) {
        auto attr = Imf::TimeCodeAttribute::cast(attribute);
        ss << attr.value().hours() << ":" << attr.value().minutes() << ":" << attr.value().seconds() << " f" << attr.value().frame();
    }
    // Vector
    else if (strcmp(type, Imf::V2iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2iAttribute::cast(attribute);
        ss << attr.value();
    }
    else if (strcmp(type, Imf::V2fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2fAttribute::cast(attribute);
        ss << attr.value();
    }
    else if (strcmp(type, Imf::V2dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V2dAttribute::cast(attribute);
        ss << attr.value();
    }
    else if (strcmp(type, Imf::V3iAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3iAttribute::cast(attribute);
        ss << attr.value();
    }
    else if (strcmp(type, Imf::V3fAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3fAttribute::cast(attribute);
        ss << attr.value();
    }
    else if (strcmp(type, Imf::V3dAttribute::staticTypeName()) == 0) {
        auto attr = Imf::V3dAttribute::cast(attribute);
        ss << attr.value();
    }

    QVector<QVariant> itemData = {name, QString(ss.str().c_str()), type};
    attrItem->setData(itemData);
    attrItem->setName(name);
    attrItem->setPartID(part_number);

    return attrItem;
}



