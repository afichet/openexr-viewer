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
        const QString& parentLayerName,
        LayerType layerType,
        QObject *parent)
    : ImageModel(parent)
    , m_parentLayer(parentLayerName)
    , m_layerType(layerType)
    , m_exposure(0.)
{}


RGBFramebufferModel::~RGBFramebufferModel()
{}


void RGBFramebufferModel::load(
    Imf::MultiPartInputFile& file,
    int partId,
    bool hasAlpha)
{
    QFuture<void> imageLoading = QtConcurrent::run([&]() {
        try {
            Imf::InputPart part(file, partId);

            Imath::Box2i dw = part.header().dataWindow();
            m_width  = dw.max.x - dw.min.x + 1;
            m_height = dw.max.y - dw.min.y + 1;

            // TODO viewport

            m_pixelBuffer = new float[4 * m_width * m_height];

            // Check if there is alpha channel
            if (hasAlpha) {
                QString aLayer = m_parentLayer + "A";
                Imf::FrameBuffer framebuffer;

                Imf::Slice aSlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &m_pixelBuffer[3],
                        dw,
                        4 * sizeof(float), 4 * m_width * sizeof(float));

                framebuffer.insert(aLayer.toStdString(), aSlice);

                part.setFrameBuffer(framebuffer);
                part.readPixels(dw.min.y, dw.max.y);

            } else {
                for (int y = 0; y < m_height; y++) {
                    for (int x = 0; x < m_width; x++) {
                        m_pixelBuffer[4 * (y * m_width + x) + 3] = 1.f;
                    }
                }
            }

            switch(m_layerType) {
            case Layer_RGB:
            {
                QString rLayer = m_parentLayer + "R";
                QString gLayer = m_parentLayer + "G";
                QString bLayer = m_parentLayer + "B";

                Imf::FrameBuffer framebuffer;

                Imf::Slice rSlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &m_pixelBuffer[0],
                        dw,
                        4 * sizeof(float), 4 * m_width * sizeof(float));

                Imf::Slice gSlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &m_pixelBuffer[1],
                        dw,
                        4 * sizeof(float), 4 * m_width * sizeof(float));

                Imf::Slice bSlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &m_pixelBuffer[2],
                        dw,
                        4 * sizeof(float), 4 * m_width * sizeof(float));


                framebuffer.insert(rLayer.toStdString().c_str(), rSlice);
                framebuffer.insert(gLayer.toStdString().c_str(), gSlice);
                framebuffer.insert(bLayer.toStdString().c_str(), bSlice);

                part.setFrameBuffer(framebuffer);
                part.readPixels(dw.min.y, dw.max.y);
            }
                break;

            case Layer_YC:
            {
                QString yLayer = m_parentLayer + "Y";
                QString ryLayer = m_parentLayer + "RY";
                QString byLayer = m_parentLayer + "BY";

                Imf::FrameBuffer framebuffer;

                float *yBuffer = new float[m_width * m_height];
                float *ryBuffer = new float[m_width/2 * m_height/2];
                float *byBuffer = new float[m_width/2 * m_height/2];

                Imf::Slice ySlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &yBuffer[0],
                        dw,
                        sizeof(float), m_width * sizeof(float));

                Imf::Slice rySlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &ryBuffer[0],
                        dw,
                        sizeof(float), m_width/2 * sizeof(float),
                        2, 2);

                Imf::Slice bySlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &byBuffer[0],
                        dw,
                        sizeof(float), m_width/2 * sizeof(float),
                        2, 2);

                framebuffer.insert(yLayer.toStdString().c_str(), ySlice);
                framebuffer.insert(ryLayer.toStdString().c_str(), rySlice);
                framebuffer.insert(byLayer.toStdString().c_str(), bySlice);

                part.setFrameBuffer(framebuffer);
                part.readPixels(dw.min.y, dw.max.y);

                // Now recompute the image
                // TODO: use chromaticities from header
#pragma omp parallel for
                for (int y = 0; y < m_height; y++) {
                    for (int x = 0; x < m_width; x++) {
                        float l = yBuffer[y * m_width + x];
                        float ry = ryBuffer[y/2 * m_width/2 + x/2];
                        float by = byBuffer[y/2 * m_width/2 + x/2];

                        float r = (ry + 1.f) * l;
                        float b = (by + 1.f) * l;
                        float g = (l - 0.2126 * r - 0.0722 * b) / 0.7152;

                        m_pixelBuffer[4 * (y * m_width + x) + 0] = r;
                        m_pixelBuffer[4 * (y * m_width + x) + 1] = g;
                        m_pixelBuffer[4 * (y * m_width + x) + 2] = b;
                    }
                }

                delete[] yBuffer;
                delete[] ryBuffer;
                delete[] byBuffer;
            }

                break;

            case Layer_Y:
            {
                QString yLayer = m_parentLayer + "Y";

                Imf::FrameBuffer framebuffer;

                Imf::Slice ySlice = Imf::Slice::Make(
                            Imf::PixelType::FLOAT,
                            &m_pixelBuffer[0],
                        dw,
                        4 * sizeof(float), 4 * m_width * sizeof(float));

                framebuffer.insert(yLayer.toStdString().c_str(), ySlice);

                part.setFrameBuffer(framebuffer);
                part.readPixels(dw.min.y, dw.max.y);

#pragma omp parallel for
                for (int y = 0; y < m_height; y++) {
                    for (int x = 0; x < m_width; x++) {
                        m_pixelBuffer[4 * (y * m_width + x) + 1] = m_pixelBuffer[4 * (y * m_width + x) + 0];
                        m_pixelBuffer[4 * (y * m_width + x) + 2] = m_pixelBuffer[4 * (y * m_width + x) + 0];
                    }
                }
            }
                break;
            }

            m_image = QImage(m_width, m_height, QImage::Format_RGBA8888);
            m_isImageLoaded = true;

            emit imageLoaded(m_width, m_height);

            updateImage();
        } catch (std::exception &e) {
            emit loadFailed(e.what());
            return;
        }
    });

    m_imageLoadingWatcher->setFuture(imageLoading);
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
    if (!m_isImageLoaded) { return; }

    // Several call can occur within a short time e.g., when changing exposure
    // Ensure to cancel any previous running conversion and wait for the
    // process to end
    if (m_imageEditingWatcher->isRunning()) {
        m_imageEditingWatcher->cancel();
        m_imageEditingWatcher->waitForFinished();
    }

    float m_exposure_mul = std::exp2(m_exposure);

    QFuture<void> imageConverting = QtConcurrent::run([=]() {
        for (int y = 0; y < m_image.height(); y++) {
            unsigned char * line = m_image.scanLine(y);

            #pragma omp parallel for
            for (int x = 0; x < m_image.width(); x++) {
                const float r = to_sRGB(m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 0]);
                const float g = to_sRGB(m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 1]);
                const float b = to_sRGB(m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 2]);
                
                const float a = m_pixelBuffer[4 * (y * m_width + x) + 3];

                line[4 * x + 0] = qMax(0, qMin(255, int(255.f * r)));
                line[4 * x + 1] = qMax(0, qMin(255, int(255.f * g)));
                line[4 * x + 2] = qMax(0, qMin(255, int(255.f * b)));
                line[4 * x + 3] = qMax(0, qMin(255, int(255.f * a)));
            }

            if (m_imageEditingWatcher->isCanceled()) { break; }
        }

        // We do not notify any canceled process: this would result in
        // potentially corrupted conversion
        if (!m_imageEditingWatcher->isCanceled()) {
            emit imageChanged();
        }
    });

    m_imageEditingWatcher->setFuture(imageConverting);
}
