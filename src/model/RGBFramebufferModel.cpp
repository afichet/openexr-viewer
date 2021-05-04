#include "RGBFramebufferModel.h"

#include <cmath>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <OpenEXR/ImfInputPart.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfFrameBuffer.h>

#include <Imath/ImathBox.h>


RGBFramebufferModel::RGBFramebufferModel(
        Imf::MultiPartInputFile &file,
        int partId,
        const QString &parentLayerName,
        QObject *parent)
    : ImageModel(parent)
    , m_partID(partId)
    , m_parentLayer(parentLayerName)
    , m_exposure(0.)
{
    Imf::InputPart part(file, partId);

    Imath::Box2i dw = part.header().dataWindow();
    m_width  = dw.max.x - dw.min.x + 1;
    m_height = dw.max.y - dw.min.y + 1;

    // TODO viewport

    m_pixelBuffer = new float[3 * m_width * m_height];

    QString rLayer = m_parentLayer + "R";
    QString gLayer = m_parentLayer + "G";
    QString bLayer = m_parentLayer + "B";

    Imf::FrameBuffer framebuffer;
    framebuffer.insert(
                rLayer.toStdString().c_str(),
                Imf::Slice(
                    Imf::PixelType::FLOAT,
                    (char*)&m_pixelBuffer[0],
                    3 * sizeof(float), 3 * m_width * sizeof(float))
                );

    framebuffer.insert(
                gLayer.toStdString().c_str(),
                Imf::Slice(
                    Imf::PixelType::FLOAT,
                    (char*)&m_pixelBuffer[1],
                    3 * sizeof(float), 3 * m_width * sizeof(float))
                );

    framebuffer.insert(
                bLayer.toStdString().c_str(),
                Imf::Slice(
                    Imf::PixelType::FLOAT,
                    (char*)&m_pixelBuffer[2],
                    3 * sizeof(float), 3 * m_width * sizeof(float))
                );

    part.setFrameBuffer(framebuffer);
    part.readPixels(dw.min.y, dw.max.y);

    updateImage();
}


float RGBFramebufferModel::to_sRGB(float rgb_color)
{
    const double a = 0.055;
    if (rgb_color < 0.0031308)
        return 12.92 * rgb_color;
    else
        return (1.0 + a) * std::pow(rgb_color, 1.0 / 2.4) - a;
}


void RGBFramebufferModel::setExposure(double value)
{
    m_exposure = value;
    updateImage();
}


void RGBFramebufferModel::updateImage()
{
    // Several call can occur within a short time e.g., when changing exposure
    // Ensure to cancel any previous running conversion and wait for the
    // process to end
    if (m_imageLoadingWatcher->isRunning()) {
        m_imageLoadingWatcher->cancel();
        m_imageLoadingWatcher->waitForFinished();
    }

    float m_exposure_mul = std::exp2(m_exposure);

    QFuture<void> imageConverting = QtConcurrent::run([=]() {
        m_image = QImage(m_width, m_height, QImage::Format_RGB888);

        for (int y = 0; y < m_image.height(); y++) {
            unsigned char * line = m_image.scanLine(y);
            for (int x = 0; x < m_image.width(); x++) {
                const float r = to_sRGB(m_exposure_mul * m_pixelBuffer[3 * (y * m_width + x) + 0]);
                const float g = to_sRGB(m_exposure_mul * m_pixelBuffer[3 * (y * m_width + x) + 1]);
                const float b = to_sRGB(m_exposure_mul * m_pixelBuffer[3 * (y * m_width + x) + 2]);

                line[3 * x + 0] = qMax(0, qMin(255, int(255.f * r)));
                line[3 * x + 1] = qMax(0, qMin(255, int(255.f * g)));
                line[3 * x + 2] = qMax(0, qMin(255, int(255.f * b)));
            }

            if (m_imageLoadingWatcher->isCanceled()) { break; }
        }

        // We do not notify any canceled process: this would result in
        // potentially corrupted conversion
        if (!m_imageLoadingWatcher->isCanceled()) {
            m_isImageLoaded = true;

            emit imageLoaded(m_width, m_height);
            emit imageChanged();
        }
    });

    m_imageLoadingWatcher->setFuture(imageConverting);
}
