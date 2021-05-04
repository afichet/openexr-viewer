#include "OpenEXRHeaderItem.h"


OpenEXRHeaderItem::OpenEXRHeaderItem(OpenEXRHeaderItem *parentItem,
        const QVector<QVariant> &data,
        QString name, int partID)
    : m_itemData(data)
    , m_parentItem(parentItem)
    , m_name(name)
    , m_partID(partID)
{
    // If not root item
    if (m_parentItem) {
        m_parentItem->appendChild(this);
    }
}


OpenEXRHeaderItem::~OpenEXRHeaderItem() {
    qDeleteAll(m_childItems);
}


void OpenEXRHeaderItem::appendData(QVariant sibbling) {
    m_itemData.append(sibbling);
}


void OpenEXRHeaderItem::setData(QVector<QVariant> data) {
    m_itemData = data;
}


OpenEXRHeaderItem *OpenEXRHeaderItem::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }

    return m_childItems.at(row);
}


int OpenEXRHeaderItem::childCount() const {
    return m_childItems.size();
}


int OpenEXRHeaderItem::columnCount() const {
    return m_itemData.size();
}


QVariant OpenEXRHeaderItem::data(int column) const {
    if (column < 0 || column >= m_itemData.size()) {
        return QVariant();
    }

    return m_itemData.at(column);
}


int OpenEXRHeaderItem::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<OpenEXRHeaderItem*>(this));
    }

    return 0;
}


OpenEXRHeaderItem *OpenEXRHeaderItem::parentItem() {
    return m_parentItem;
}
