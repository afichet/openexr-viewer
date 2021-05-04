#pragma once

#include "ImageModel.h"

class RGBFramebufferModel: public ImageModel {
public:
    RGBFramebufferModel(
            Imf::MultiPartInputFile& file,
            int partId,
            const QString& parentLayerName,
            QObject *parent = nullptr);


    static float to_sRGB(float rgb_color);


public slots:
    void setExposure(double value);

protected:
    void updateImage();

private:
    int m_partID;
    QString m_parentLayer;

    double m_exposure;
};
