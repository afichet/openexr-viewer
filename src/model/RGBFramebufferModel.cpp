//
// Copyright (c) 2021 Alban Fichet <alban.fichet at gmx.fr>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//  * Neither the name of %ORGANIZATION% nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
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
    // TODO Support Luminance Chroma & Y

    m_pixelBuffer = new float[3 * m_width * m_height];

    QString rLayer = m_parentLayer + "R";
    QString gLayer = m_parentLayer + "G";
    QString bLayer = m_parentLayer + "B";

    Imf::FrameBuffer framebuffer;

    Imf::Slice rSlice = Imf::Slice::Make(
                Imf::PixelType::FLOAT,
                &m_pixelBuffer[0],
                dw,
                3 * sizeof(float), 3 * m_width * sizeof(float));

    Imf::Slice gSlice = Imf::Slice::Make(
                Imf::PixelType::FLOAT,
                &m_pixelBuffer[1],
                dw,
                3 * sizeof(float), 3 * m_width * sizeof(float));

    Imf::Slice bSlice = Imf::Slice::Make(
                Imf::PixelType::FLOAT,
                &m_pixelBuffer[2],
                dw,
                3 * sizeof(float), 3 * m_width * sizeof(float));

    framebuffer.insert(rLayer.toStdString().c_str(), rSlice);
    framebuffer.insert(gLayer.toStdString().c_str(), gSlice);
    framebuffer.insert(bLayer.toStdString().c_str(), bSlice);

    part.setFrameBuffer(framebuffer);
    part.readPixels(dw.min.y, dw.max.y);

    updateImage();
}

RGBFramebufferModel::~RGBFramebufferModel()
{

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

            #pragma omp parallel for
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
