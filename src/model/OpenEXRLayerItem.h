#pragma once

#include <model/OpenEXRHeaderItem.h>

#include <OpenEXR/ImfChannelListAttribute.h>

class OpenEXRLayerItem {
public:
    OpenEXRLayerItem(OpenEXRLayerItem* parent = nullptr);

    ~OpenEXRLayerItem();

    void addLeaf(
            const QString channelName,
            const Imf::Channel* leafChannel);

    OpenEXRHeaderItem* constructItemHierarchy(OpenEXRHeaderItem* parent, int partID);

    size_t getNChilds() const;

    bool hasRGBChilds() const;

    QString getFullName() const;

protected:
    OpenEXRLayerItem* getAddLeaf(const QString channelName);

private:
    QMap<QString, OpenEXRLayerItem*> m_childItems;
    OpenEXRLayerItem* m_parentItem;

    const Imf::Channel* m_channelPtr;
    QString m_rootName;

    QString m_channelName;
};
