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
        if (m_parentItem)
            m_parentItem->appendChild(this);
    }

//    explicit OpenEXRItem(
//            OpenEXRItem *parentItem = nullptr)
//        : m_itemData()
//        , m_parentItem(parentItem)
//    {
////        if (m_parentItem)
////            m_parentItem->appendChild(this);
//    }

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


//protecte\d:
    void appendChild(OpenEXRItem *child) {
        m_childItems.append(child);
    }

private:
    QVector<OpenEXRItem*> m_childItems;
    OpenEXRItem* m_parentItem;

    QVector<QVariant> m_itemData;
};

class OpenEXRImage: public QAbstractItemModel {
public:
    OpenEXRImage(const QString& filename, QObject *parent);

    ~OpenEXRImage() {
        delete m_rootItem;
    }


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
