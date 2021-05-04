#pragma once

#include <QVector>
#include <QVariant>

class OpenEXRHeaderItem {
public:
    explicit OpenEXRHeaderItem(
            OpenEXRHeaderItem *parentItem = nullptr,
            const QVector<QVariant> &data = QVector<QVariant>(),
            QString name = QString(),
            int partID = 0);

    ~OpenEXRHeaderItem();

    void appendData(QVariant sibbling);

    void setData(QVector<QVariant> data);

    OpenEXRHeaderItem *child(int row);

    int childCount() const;

    int columnCount() const;

    QVariant data(int column) const;

    int row() const;

    OpenEXRHeaderItem *parentItem();

    QString type() const {
        return m_itemData[2].toString();
    }

    QString getName() const {
        return m_name;
    }

    int getPartID() const {
        return m_partID;
    }

    void setName(const QString& name) {
        m_name = name;
    }

    void setPartID(int partID) {
        m_partID = partID;
    }

protected:
    void appendChild(OpenEXRHeaderItem *child) {
        m_childItems.append(child);
    }

private:
    QVector<QVariant> m_itemData;
    OpenEXRHeaderItem* m_parentItem;
    QVector<OpenEXRHeaderItem*> m_childItems;

    QString m_name;
    int m_partID;
};
