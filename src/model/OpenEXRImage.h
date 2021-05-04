#pragma once

#include <QAbstractItemModel>

#include <OpenEXR/ImfMultiPartInputFile.h>

#include <model/HeaderModel.h>
#include <model/ImageModel.h>
#include <model/FramebufferModel.h>
#include <model/RGBFramebufferModel.h>

class OpenEXRImage {
public:
    OpenEXRImage(const QString& filename, QObject *parent);
    ~OpenEXRImage();

    HeaderModel* getHeaderModel() const {
        return _headerModel;
    }

    ImageModel* createImageModel(int partId, const QString& layer) {
        return new FramebufferModel(m_exrIn, partId, layer);
    }

    ImageModel* createRGBImageModel(int partId, const QString& parentLayer) {
        return new RGBFramebufferModel(m_exrIn, partId, parentLayer);
    }


private:
    Imf::MultiPartInputFile m_exrIn;

    HeaderModel* _headerModel;

};
