#pragma once

#include <QVector>
#include <QVariant>

class OpenEXRHeaderItem {
public:
    explicit OpenEXRHeaderItem(
            OpenEXRHeaderItem *parentItem = nullptr,
            const QVector<QVariant> &data = QVector<QVariant>());

    ~OpenEXRHeaderItem();

    void appendData(QVariant sibbling);

    void setData(QVector<QVariant> data);

    OpenEXRHeaderItem *child(int row);

    int childCount() const;

    int columnCount() const;

    QVariant data(int column) const;

    int row() const;

    OpenEXRHeaderItem *parentItem();

protected:
    void appendChild(OpenEXRHeaderItem *child) {
        m_childItems.append(child);
    }

private:
    QVector<QVariant> m_itemData;
    OpenEXRHeaderItem* m_parentItem;
    QVector<OpenEXRHeaderItem*> m_childItems;
};
