/**
 * Copyright (c) 2021 Alban Fichet <alban dot fichet at gmx dot fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *  * Neither the name of the organization(s) nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "RGBFramebufferModel.h"

#include <util/ColorTransform.h>

#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include <OpenEXR/ImfChromaticitiesAttribute.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputPart.h>
#include <OpenEXR/ImfRgbaYca.h>

#include <Imath/ImathBox.h>

RGBFramebufferModel::RGBFramebufferModel(
  const std::string& parentLayerName, LayerType layerType, QObject* parent)
  : FramebufferModel(parent)
  , m_parentLayer(parentLayerName)
  , m_layerType(layerType)
  , m_exposure(0.)
{}

RGBFramebufferModel::~RGBFramebufferModel() {}

void RGBFramebufferModel::load(
  Imf::MultiPartInputFile& file, int partId, bool hasAlpha)
{
    QFuture<void> imageLoading = QtConcurrent::run([this,
                                                    &file,
                                                    partId,
                                                    hasAlpha]() {
        try {
            Imf::InputPart part(file, partId);

            Imath::Box2i datW = part.header().dataWindow();
            m_width           = datW.max.x - datW.min.x + 1;
            m_height          = datW.max.y - datW.min.y + 1;

            m_pixelAspectRatio = part.header().pixelAspectRatio();

            m_dataWindow = QRect(datW.min.x, datW.min.y, m_width, m_height);

            Imath::Box2i dispW = part.header().displayWindow();

            int dispW_width  = dispW.max.x - dispW.min.x + 1;
            int dispW_height = dispW.max.y - dispW.min.y + 1;

            m_displayWindow
              = QRect(dispW.min.x, dispW.min.y, dispW_width, dispW_height);

            // Check to avoid type overflow, width and height are 32bits int
            // representing a 2 dimentional image. Can overflow the type when
            // multiplied together.
            // 0x1FFFFFFF is a save limit for 4 * 0x7FFFFFFF the max
            // representable int since we need 4 channels.
            // TODO: Use larger type when manipulating framebuffer
            const uint64_t partial_size
              = (uint64_t)m_width * (uint64_t)m_height;

            if (partial_size > 0x1FFFFFFF) {
                throw std::runtime_error(
                  "The total image size is too large. May be supported in a "
                  "future revision.");
            }

            m_pixelBuffer.resize(4 * m_width * m_height);

            // Check if there is specific chromaticities tied to the color
            // representation in this part.
            const Imf::ChromaticitiesAttribute* c
              = part.header().findTypedAttribute<Imf::ChromaticitiesAttribute>(
                "chromaticities");

            Imf::Chromaticities chromaticities;

            if (c != nullptr) {
                chromaticities = c->value();
            }

            // Check if there is alpha channel
            if (hasAlpha) {
                std::string      aLayer = m_parentLayer + "A";
                Imf::FrameBuffer framebuffer;

                Imf::Slice aSlice = Imf::Slice::Make(
                  Imf::PixelType::FLOAT,
                  &m_pixelBuffer[3],
                  datW,
                  4 * sizeof(float),
                  4 * m_width * sizeof(float));

                framebuffer.insert(aLayer, aSlice);

                part.setFrameBuffer(framebuffer);
                part.readPixels(datW.min.y, datW.max.y);

            } else {
                for (int y = 0; y < m_height; y++) {
                    for (int x = 0; x < m_width; x++) {
                        m_pixelBuffer[4 * (y * m_width + x) + 3] = 1.f;
                    }
                }
            }

            switch (m_layerType) {
                case Layer_RGB: {
                    std::string rLayer = m_parentLayer + "R";
                    std::string gLayer = m_parentLayer + "G";
                    std::string bLayer = m_parentLayer + "B";

                    Imf::FrameBuffer framebuffer;

                    Imf::Slice rSlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &m_pixelBuffer[0],
                      datW,
                      4 * sizeof(float),
                      4 * m_width * sizeof(float));

                    Imf::Slice gSlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &m_pixelBuffer[1],
                      datW,
                      4 * sizeof(float),
                      4 * m_width * sizeof(float));

                    Imf::Slice bSlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &m_pixelBuffer[2],
                      datW,
                      4 * sizeof(float),
                      4 * m_width * sizeof(float));

                    framebuffer.insert(rLayer, rSlice);
                    framebuffer.insert(gLayer, gSlice);
                    framebuffer.insert(bLayer, bSlice);

                    part.setFrameBuffer(framebuffer);
                    part.readPixels(datW.min.y, datW.max.y);

                    // Handle custom chromaticities
                    Imath::M44f RGB_XYZ = Imf::RGBtoXYZ(chromaticities, 1.f);
                    Imath::M44f XYZ_RGB
                      = Imf::XYZtoRGB(Imf::Chromaticities(), 1.f);

                    Imath::M44f conversionMatrix = RGB_XYZ * XYZ_RGB;

                    #pragma omp parallel for
                    for (int y = 0; y < m_height; y++) {
                        for (int x = 0; x < m_width; x++) {
                            const float r
                              = m_pixelBuffer[4 * (y * m_width + x) + 0];
                            const float g
                              = m_pixelBuffer[4 * (y * m_width + x) + 1];
                            const float b
                              = m_pixelBuffer[4 * (y * m_width + x) + 2];

                            Imath::V3f rgb(r, g, b);
                            rgb *= conversionMatrix;

                            m_pixelBuffer[4 * (y * m_width + x) + 0] = rgb.x;
                            m_pixelBuffer[4 * (y * m_width + x) + 1] = rgb.y;
                            m_pixelBuffer[4 * (y * m_width + x) + 2] = rgb.z;
                        }
                    }
                } break;

                case Layer_YC: {
                    std::string yLayer  = m_parentLayer + "Y";
                    std::string ryLayer = m_parentLayer + "RY";
                    std::string byLayer = m_parentLayer + "BY";

                    Imf::FrameBuffer framebuffer;

                    std::vector<Imf::Rgba> buff1(m_width * m_height);
                    std::vector<Imf::Rgba> buff2(m_width * m_height);

                    std::vector<float> yBuffer(m_width * m_height);
                    std::vector<float> ryBuffer(m_width / 2 * m_height / 2);
                    std::vector<float> byBuffer(m_width / 2 * m_height / 2);

                    Imf::Slice ySlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &yBuffer[0],
                      datW,
                      sizeof(float),
                      m_width * sizeof(float));

                    Imf::Slice rySlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &ryBuffer[0],
                      datW,
                      sizeof(float),
                      m_width / 2 * sizeof(float),
                      2,
                      2);

                    Imf::Slice bySlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &byBuffer[0],
                      datW,
                      sizeof(float),
                      m_width / 2 * sizeof(float),
                      2,
                      2);

                    framebuffer.insert(yLayer, ySlice);
                    framebuffer.insert(ryLayer, rySlice);
                    framebuffer.insert(byLayer, bySlice);

                    part.setFrameBuffer(framebuffer);
                    part.readPixels(datW.min.y, datW.max.y);

                    // Filling missing values for chroma in the image
                    // TODO: now, naive reconstruction.
                    // Use later Imf::RgbaYca::reconstructChromaHoriz and
                    // Imf::RgbaYca::reconstructChromaVert to reconstruct missing
                    // pixels
                    #pragma omp parallel for
                    for (int y = 0; y < m_height; y++) {
                        for (int x = 0; x < m_width; x++) {
                            const float l = yBuffer[y * m_width + x];

                            /*
                            float ry = 0, by = 0;

                            if (y % 2 == 0) {
                                if (x % 2 == 0) {
                                    ry = ryBuffer[y / 2 * m_width / 2 + x / 2];
                                    by = byBuffer[y / 2 * m_width / 2 + x / 2];
                                } else {
                                    ry = .5 * (ryBuffer[y / 2 * m_width / 2 + x / 2] + ryBuffer[y / 2 * m_width / 2 + x / 2 + 1]);
                                    by = .5 * (byBuffer[y / 2 * m_width / 2 + x / 2] + byBuffer[y / 2 * m_width / 2 + x / 2 + 1]);
                                }
                            } else {
                                if (x % 2 == 0) {
                                    ry = .5 * (ryBuffer[y / 2 * m_width / 2 + x / 2] + ryBuffer[(y / 2 + 1) * m_width / 2 + x / 2]);
                                    by = .5 * (byBuffer[y / 2 * m_width / 2 + x / 2] + byBuffer[(y / 2 + 1) * m_width / 2 + x / 2]);
                                } else {
                                    ry = .25 * (ryBuffer[y / 2 * m_width / 2 + x / 2] + ryBuffer[(y / 2 + 1) * m_width / 2 + x / 2] + ryBuffer[y / 2 * m_width / 2 + x / 2 + 1] + ryBuffer[(y / 2 + 1) * m_width / 2 + x / 2 + 1]);
                                    by = .25 * (byBuffer[y / 2 * m_width / 2 + x / 2] + byBuffer[(y / 2 + 1) * m_width / 2 + x / 2] + byBuffer[y / 2 * m_width / 2 + x / 2 + 1] + byBuffer[(y / 2 + 1) * m_width / 2 + x / 2 + 1]);
                                }
                            }
                            */

                            const float ry
                              = ryBuffer[y / 2 * m_width / 2 + x / 2];
                            const float by
                              = byBuffer[y / 2 * m_width / 2 + x / 2];

                            buff1[y * m_width + x].r = ry;
                            buff1[y * m_width + x].g = l;
                            buff1[y * m_width + x].b = by;
                            // Do not forget the alpha values read earlier
                            buff1[y * m_width + x].a
                              = m_pixelBuffer[4 * (y * m_width + x) + 3];
                        }
                    }

                    Imath::V3f yw = Imf::RgbaYca::computeYw(chromaticities);

                    // Proceed to the YCA -> RGBA conversion
                    #pragma omp parallel for
                    for (int y = 0; y < m_height; y++) {
                        Imf::RgbaYca::YCAtoRGBA(
                          yw,
                          m_width,
                          &buff1[y * m_width],
                          &buff1[y * m_width]);
                    }

                    // Fix over saturated pixels
                    #pragma omp parallel for
                    for (int y = 0; y < m_height; y++) {
                        const Imf::Rgba* scanlines[3];

                        if (y == 0) {
                            scanlines[0] = &buff1[(y + 1) * m_width];
                        } else {
                            scanlines[0] = &buff1[(y - 1) * m_width];
                        }

                        scanlines[1] = &buff1[y * m_width];

                        if (y == m_height - 1) {
                            scanlines[2] = &buff1[(y - 1) * m_width];
                        } else {
                            scanlines[2] = &buff1[(y + 1) * m_width];
                        }

                        Imf::RgbaYca::fixSaturation(
                          yw,
                          m_width,
                          scanlines,
                          &buff2[y * m_width]);
                    }

                    // Handle custom chromaticities
                    Imath::M44f RGB_XYZ = Imf::RGBtoXYZ(chromaticities, 1.f);
                    Imath::M44f XYZ_RGB
                      = Imf::XYZtoRGB(Imf::Chromaticities(), 1.f);

                    Imath::M44f conversionMatrix = RGB_XYZ * XYZ_RGB;

                    #pragma omp parallel for
                    for (int y = 0; y < m_height; y++) {
                        for (int x = 0; x < m_width; x++) {
                            Imath::V3f rgb(
                              buff2[y * m_width + x].r,
                              buff2[y * m_width + x].g,
                              buff2[y * m_width + x].b);

                            rgb = rgb * conversionMatrix;

                            m_pixelBuffer[4 * (y * m_width + x) + 0] = rgb.x;
                            m_pixelBuffer[4 * (y * m_width + x) + 1] = rgb.y;
                            m_pixelBuffer[4 * (y * m_width + x) + 2] = rgb.z;
                        }
                    }
                }

                break;

                case Layer_Y: {
                    std::string yLayer = m_parentLayer;

                    Imf::FrameBuffer framebuffer;

                    Imf::Slice ySlice = Imf::Slice::Make(
                      Imf::PixelType::FLOAT,
                      &m_pixelBuffer[0],
                      datW,
                      4 * sizeof(float),
                      4 * m_width * sizeof(float));

                    framebuffer.insert(yLayer, ySlice);

                    part.setFrameBuffer(framebuffer);
                    part.readPixels(datW.min.y, datW.max.y);

                    #pragma omp parallel for
                    for (int i = 0; i < m_height * m_width; i++) {
                        m_pixelBuffer[4 * i + 1] = m_pixelBuffer[4 * i + 0];
                        m_pixelBuffer[4 * i + 2] = m_pixelBuffer[4 * i + 0];
                        m_pixelBuffer[4 * i + 3] = 1.f;
                    }
                } break;
            }

            m_image = QImage(m_width, m_height, QImage::Format_RGBA8888);
            m_isImageLoaded = true;

            emit imageLoaded();

            updateImage();
        } catch (std::exception& e) {
            emit loadFailed(e.what());
            return;
        }
    });

    m_imageLoadingWatcher->setFuture(imageLoading);
}

std::string RGBFramebufferModel::getColorInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return "";
    }

    std::stringstream ss;
    ss << "x: " << x << " y: " << y << " | "
       << " R: " << m_pixelBuffer[4 * (y * width() + x) + 0]
       << " G: " << m_pixelBuffer[4 * (y * width() + x) + 1]
       << " B: " << m_pixelBuffer[4 * (y * width() + x) + 2]
       << " A: " << m_pixelBuffer[4 * (y * width() + x) + 3];

    return ss.str();
}


float RGBFramebufferModel::getRedInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return 0;
    }

    return m_pixelBuffer[4 * (y * width() + x) + 0];
}


float RGBFramebufferModel::getGreenInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return 0;
    }

    return m_pixelBuffer[4 * (y * width() + x) + 1];
}


float RGBFramebufferModel::getBlueInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return 0;
    }

    return m_pixelBuffer[4 * (y * width() + x) + 2];
}

float RGBFramebufferModel::getAlphaInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return 0;
    }

    return m_pixelBuffer[4 * (y * width() + x) + 3];
}


void RGBFramebufferModel::setExposure(double value)
{
    if (m_exposure == value) return;

    m_exposure = value;
    updateImage();
}

void RGBFramebufferModel::updateImage()
{
    if (!m_isImageLoaded) {
        return;
    }

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
            unsigned char* line = m_image.scanLine(y);

            #pragma omp parallel for
            for (int x = 0; x < m_image.width(); x++) {
                const float r = ColorTransform::to_sRGB(
                  m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 0]);
                const float g = ColorTransform::to_sRGB(
                  m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 1]);
                const float b = ColorTransform::to_sRGB(
                  m_exposure_mul * m_pixelBuffer[4 * (y * m_width + x) + 2]);

                const float a = m_pixelBuffer[4 * (y * m_width + x) + 3];

                line[4 * x + 0] = qMax(0, qMin(255, int(255.f * r)));
                line[4 * x + 1] = qMax(0, qMin(255, int(255.f * g)));
                line[4 * x + 2] = qMax(0, qMin(255, int(255.f * b)));
                line[4 * x + 3] = qMax(0, qMin(255, int(255.f * a)));
            }

            if (m_imageEditingWatcher->isCanceled()) {
                break;
            }
        }

        // We do not notify any canceled process: this would result in
        // potentially corrupted conversion
        if (!m_imageEditingWatcher->isCanceled()) {
            emit imageChanged();
        }
    });

    m_imageEditingWatcher->setFuture(imageConverting);
}
