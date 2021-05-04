#include "ImageModel.h"

#include <cstddef>
#include <cmath>
#include <array>
#include <fstream>

ImageModel::ImageModel(QObject *parent)
    : QObject(parent)
    , m_pixelBuffer(nullptr)
    , m_width(0)
    , m_height(0)
    , m_isImageLoaded(false)
    , m_exposure(0)
    , m_imageLoadingWatcher(new QFutureWatcher<void>(this))
    , m_imageEditingWatcher(new QFutureWatcher<void>(this))
{
}


ImageModel::~ImageModel()
{
    delete[] m_pixelBuffer;
}


void ImageModel::setExposure(double value)
{
    // TODO

    emit exposureChanged(value);
}
