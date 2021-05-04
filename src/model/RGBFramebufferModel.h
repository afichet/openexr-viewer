#pragma once

#include "ImageModel.h"

class RGBFramebufferModel: public ImageModel {
public:
    RGBFramebufferModel(
            Imf::MultiPartInputFile& file,
            int partId,
            const QString& parentLayerName,
            QObject *parent = nullptr);

private:
    int m_partID;
    QString m_parentLayer;
};
