#pragma once

#include "ImageModel.h"

class FramebufferModel: public ImageModel {
public:
    FramebufferModel(
            Imf::MultiPartInputFile &file,
            int partId,
            const QString& layerName,
            QObject* parent = nullptr);

    const QString & getLayerName() const { return m_layer; }
    const int getPartId() const { return m_partID; }

private:
    int m_partID;
    QString m_layer;
};
