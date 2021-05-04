#pragma once

#include <QAbstractItemModel>

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfAttribute.h>

#include <model/OpenEXRHeaderItem.h>
#include <model/OpenEXRLayerItem.h>

class HeaderModel: public QAbstractItemModel {
public:
    HeaderModel(int n_parts, QObject *parent);

    ~HeaderModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addHeader(const Imf::Header &header, int part);

private:
    OpenEXRHeaderItem* addItem(
            const char* name,
            const Imf::Attribute& attr,
            OpenEXRHeaderItem *parent,
            int part_number);

private:
    OpenEXRHeaderItem *m_rootItem;
    std::vector<std::vector<std::string>> m_headerItems;

    std::vector<OpenEXRLayerItem*> m_partRootLayer;
};
