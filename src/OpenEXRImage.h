#pragma once

#include <QAbstractItemModel>
#include <OpenEXR/ImfMultiPartInputFile.h>

class OpenEXRItem {
public:
    explicit OpenEXRItem(
            OpenEXRItem *parentItem = nullptr,
            const QVector<QVariant> &data = QVector<QVariant>())
        : m_itemData(data)
        , m_parentItem(parentItem)
    {
        // If not root item
        if (m_parentItem) {
            m_parentItem->appendChild(this);
        }
    }

    ~OpenEXRItem() {
        qDeleteAll(m_childItems);
    }

    void appendData(QVariant sibbling) {
        m_itemData.append(sibbling);
    }

    void setData(QVector<QVariant> data) {
        m_itemData = data;
    }

    OpenEXRItem *child(int row) {
        if (row < 0 || row >= m_childItems.size()) {
            return nullptr;
        }

        return m_childItems.at(row);
    }

    int childCount() const {
        return m_childItems.size();
    }

    int columnCount() const {
        return m_itemData.size();
    }

    QVariant data(int column) const {
        if (column < 0 || column >= m_itemData.size()) {
            return QVariant();
        }

        return m_itemData.at(column);
    }

    int row() const {
        if (m_parentItem) {
            return m_parentItem->m_childItems.indexOf(const_cast<OpenEXRItem*>(this));
        }

        return 0;
    }

    OpenEXRItem *parentItem() {
        return m_parentItem;
    }


protected:
    void appendChild(OpenEXRItem *child) {
        m_childItems.append(child);
    }

private:
    QVector<OpenEXRItem*> m_childItems;
    OpenEXRItem* m_parentItem;

    QVector<QVariant> m_itemData;
};

#include <OpenEXR/ImfChannelListAttribute.h>

class OpenEXRChannelHierarchy {
public:
    OpenEXRChannelHierarchy(OpenEXRChannelHierarchy* parent = nullptr)
        : m_parentItem(parent)
        , m_channelPtr(nullptr)
    {}

    OpenEXRChannelHierarchy* getLeaf(const QString channelName) {
        QStringList channelHierachy = channelName.split(".");

        OpenEXRChannelHierarchy *leafPtr = this;

        for (auto s : channelHierachy) {
            if (leafPtr->m_childItems.contains(s)) {
                leafPtr = leafPtr->m_childItems[s];
            } else {
                OpenEXRChannelHierarchy *newPtr = new OpenEXRChannelHierarchy(leafPtr);
                newPtr->m_rootName = s;
                leafPtr->m_childItems.insert(s, newPtr);
                leafPtr = newPtr;
            }
        }

        return leafPtr;
    }

    void addLeaf(
            const QString channelName,
            const Imf::Channel* leafChannel)
    {
        OpenEXRChannelHierarchy *leafNode = getLeaf(channelName);
        leafNode->m_channelPtr = leafChannel;
    }

    // TODO Better data representation...
    // Consider inherit from same base class
    OpenEXRItem* constructItemHierarchy(OpenEXRItem* parent) {
        if (m_childItems.size() == 0) {
            // This is a terminal leaf
            assert(m_channelPtr != nullptr);
            return new OpenEXRItem(parent, {m_rootName, " ", "framebuffer"});
        }

        // TODO: add + 1 if has a framebuffer
        OpenEXRItem* currRoot = new OpenEXRItem(
                    parent, {m_rootName, (int)getNChilds(), "channellist"});

        if (m_channelPtr) {
            // It's a leaf...
            // Both are valid but I prefer the nested representation
            // OpenEXRItem* leafNode = new OpenEXRItem(parent, {m_rootName, "", "framebuffer"});
            new OpenEXRItem(currRoot, {".", "", "framebuffer"});
        }

        for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
            it.value()->constructItemHierarchy(currRoot);
        }

        return currRoot;
    }

    size_t getNChilds() const {
        return m_childItems.size();
    }

    QString getFullName() const {
        QString name = m_rootName;

        OpenEXRChannelHierarchy* parent = m_parentItem;

        while(parent) {
            name += parent->m_rootName + "." + name;
            parent = parent->m_parentItem;
        }

        return name;
    }

private:
    QMap<QString, OpenEXRChannelHierarchy*> m_childItems;
    OpenEXRChannelHierarchy* m_parentItem;

    const Imf::Channel* m_channelPtr;
    QString m_rootName;
};

class OpenEXRImage: public QAbstractItemModel {
public:
    OpenEXRImage(const QString& filename, QObject *parent);
    ~OpenEXRImage();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:

    OpenEXRItem* createItem(const char* name,
                            const Imf::Attribute& attr,
                            OpenEXRItem *parent);

private:
    Imf::MultiPartInputFile m_exrIn;
    std::vector<std::vector<std::string>> m_headerItems;

    OpenEXRItem *m_rootItem;

};
