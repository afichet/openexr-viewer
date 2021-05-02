#pragma once

#include <QAbstractItemModel>

#include <OpenEXR/ImfMultiPartInputFile.h>

#include <model/OpenEXRHeaderItem.h>
#include <model/OpenEXRLayerItem.h>


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

    OpenEXRHeaderItem* createItem(
            const char* name,
            const Imf::Attribute& attr,
            OpenEXRHeaderItem *parent,
            int part_number);

private:
    Imf::MultiPartInputFile m_exrIn;
    std::vector<std::vector<std::string>> m_headerItems;


    OpenEXRHeaderItem *m_rootItem;
    std::vector<OpenEXRLayerItem*> m_partRootLayer;
};
