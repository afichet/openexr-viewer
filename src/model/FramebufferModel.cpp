#include "FramebufferModel.h"

#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <OpenEXR/ImfInputPart.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfFrameBuffer.h>

#include <Imath/ImathBox.h>

FramebufferModel::FramebufferModel(
        Imf::MultiPartInputFile &file,
        int partId,
        const QString &layerName,
        QObject *parent)
    : ImageModel(parent)
    , m_partID(partId)
    , m_layer(layerName)
    , m_min(0.f)
    , m_max(1.f)
{
    Imf::InputPart part(file, partId);

    Imath::Box2i dw = part.header().dataWindow();
    m_width  = dw.max.x - dw.min.x + 1;
    m_height = dw.max.y - dw.min.y + 1;

    // TODO viewport

    m_pixelBuffer = new float[m_width * m_height];

    Imf::FrameBuffer framebuffer;
    framebuffer.insert(
                m_layer.toStdString().c_str(),
                Imf::Slice(
                    Imf::PixelType::FLOAT,
                    (char*)m_pixelBuffer,
                    sizeof(float), m_width * sizeof(float))
                );

    part.setFrameBuffer(framebuffer);
    part.readPixels(dw.min.y, dw.max.y);

    updateImage();
}

void FramebufferModel::setMinValue(double value)
{
    m_min = value;
    updateImage();
}

void FramebufferModel::setMaxValue(double value)
{
    m_max = value;
    updateImage();
}

void FramebufferModel::updateImage()
{
    // Several call can occur within a short time e.g., when changing exposure
    // Ensure to cancel any previous running conversion and wait for the
    // process to end
    if (m_imageLoadingWatcher->isRunning()) {
        m_imageLoadingWatcher->cancel();
        m_imageLoadingWatcher->waitForFinished();
    }

    QFuture<void> imageConverting = QtConcurrent::run([=]() {
        m_image = QImage(m_width, m_height, QImage::Format_Grayscale8);

        for (int y = 0; y < m_image.height(); y++) {
            unsigned char * line = m_image.scanLine(y);
            for (int x = 0; x < m_image.width(); x++) {
                float value = (m_pixelBuffer[y * m_width + x] - m_min) / (m_max - m_min);

                line[x] = qMax(0, qMin(255, int(255 * value)));
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
