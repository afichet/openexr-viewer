#include "OpenEXRLayerItem.h"


OpenEXRLayerItem::OpenEXRLayerItem(OpenEXRLayerItem *parent)
    : m_parentItem(parent)
    , m_channelPtr(nullptr)
{}

OpenEXRLayerItem::~OpenEXRLayerItem()
{
    for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
        delete it.value();
    }
}


void OpenEXRLayerItem::addLeaf(const QString channelName, const Imf_3_0::Channel *leafChannel)
{
    OpenEXRLayerItem *leafNode = getAddLeaf(channelName);
    leafNode->m_channelPtr = leafChannel;
    leafNode->m_channelName = channelName;
}


OpenEXRHeaderItem *OpenEXRLayerItem::constructItemHierarchy(OpenEXRHeaderItem *parent, int partID) {
    if (m_childItems.size() == 0) {
        // This is a terminal leaf
        assert(m_channelPtr != nullptr);
        return new OpenEXRHeaderItem(parent, {m_rootName, " ", "framebuffer"}, m_channelName, partID);
    }

    // TODO: add + 1 if has a framebuffer
    OpenEXRHeaderItem* currRoot = new OpenEXRHeaderItem(
                parent, {m_rootName, (int)getNChilds(), "channellist"});

    if (m_channelPtr) {
        // It's a leaf...
        // Both are valid but I prefer the nested representation
        // OpenEXRItem* leafNode = new OpenEXRItem(parent, {m_rootName, "", "framebuffer"});
        new OpenEXRHeaderItem(currRoot, {".", "", "framebuffer"}, m_channelName, partID);
    }

    // If we find RGB final leaf, we make a virtual group
    if (hasRGBChilds()) {
        OpenEXRHeaderItem* rgbRoot = new OpenEXRHeaderItem(currRoot, {"RGB", "", "RGB framebuffer"});
        new OpenEXRHeaderItem(rgbRoot, {"R", "", "framebuffer"}, m_childItems["R"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbRoot, {"G", "", "framebuffer"}, m_childItems["G"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbRoot, {"B", "", "framebuffer"}, m_childItems["B"]->m_channelName, partID);

        for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
            if ((it.key() != "R" && it.key() != "G" && it.key() != "B") || it.value()->getNChilds() != 0)
                it.value()->constructItemHierarchy(currRoot, partID);
        }
    } else {
        for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
            it.value()->constructItemHierarchy(currRoot, partID);
        }
    }

    return currRoot;
}


size_t OpenEXRLayerItem::getNChilds() const {
    return m_childItems.size();
}


bool OpenEXRLayerItem::hasRGBChilds() const {
    return m_childItems.contains("R") && m_childItems["R"]->m_channelPtr
            && m_childItems.contains("G") && m_childItems["G"]->m_channelPtr
            && m_childItems.contains("B") && m_childItems["B"]->m_channelPtr;
}


QString OpenEXRLayerItem::getFullName() const {
    QString name = m_rootName;

    OpenEXRLayerItem* parent = m_parentItem;

    while(parent) {
        name += parent->m_rootName + "." + name;
        parent = parent->m_parentItem;
    }

    return name;
}


OpenEXRLayerItem *OpenEXRLayerItem::getAddLeaf(const QString channelName) {
    QStringList channelHierachy = channelName.split(".");

    OpenEXRLayerItem *leafPtr = this;

    for (auto& s : channelHierachy) {
        if (leafPtr->m_childItems.contains(s)) {
            leafPtr = leafPtr->m_childItems[s];
        } else {
            OpenEXRLayerItem *newPtr = new OpenEXRLayerItem(leafPtr);
            newPtr->m_rootName = s;
            leafPtr->m_childItems.insert(s, newPtr);
            leafPtr = newPtr;
        }
    }

    return leafPtr;
}
