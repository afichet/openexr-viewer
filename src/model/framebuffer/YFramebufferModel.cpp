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

#include "YFramebufferModel.h"

#include <util/ColormapModule.h>

#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include <OpenEXR/ImfAttribute.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputPart.h>

#include <Imath/ImathBox.h>

YFramebufferModel::YFramebufferModel(
  const std::string& layerName, QObject* parent)
  : FramebufferModel(parent)
  , m_layer(layerName)
  , m_min(0.f)
  , m_max(1.f)
  , m_cmap(ColormapModule::create("grayscale"))
{}

YFramebufferModel::~YFramebufferModel()
{
    delete m_cmap;
}

void YFramebufferModel::load(Imf::MultiPartInputFile& file, int partId)
{
    QFuture<void> imageLoading = QtConcurrent::run([this, &file, partId]() {
        try {
            Imf::InputPart part(file, partId);

            Imath::Box2i datW = part.header().dataWindow();
            m_width           = datW.max.x - datW.min.x + 1;
            m_height          = datW.max.y - datW.min.y + 1;

            m_pixelAspectRatio = part.header().pixelAspectRatio();

            Imf::Slice graySlice;
            // TODO: Check it that can be guess from the header
            // also, check if this can be nested
            if (m_layer == "BY" || m_layer == "RY") {
                m_width /= 2;
                m_height /= 2;

                m_dataWindow = QRect(datW.min.x, datW.min.y, m_width, m_height);

                Imath::Box2i dispW = part.header().displayWindow();

                int dispW_width  = dispW.max.x - dispW.min.x + 1;
                int dispW_height = dispW.max.y - dispW.min.y + 1;

                m_displayWindow = QRect(
                  dispW.min.x,
                  dispW.min.y,
                  dispW_width / 2,
                  dispW_height / 2);

                // Check to avoid type overflow, width and height are 32bits int
                // representing a 2 dimentional image. Can overflow the type when
                // multiplied together
                // TODO: Use larger type when manipulating framebuffer
                const uint64_t partial_size
                  = (uint64_t)m_width * (uint64_t)m_height;

                if (partial_size > 0x7FFFFFFF) {
                    throw std::runtime_error(
                      "The total image size is too large. May be supported in "
                      "a future revision.");
                }

                m_pixelBuffer.resize(m_width * m_height);

                // Luminance Chroma channels
                graySlice = Imf::Slice::Make(
                  Imf::PixelType::FLOAT,
                  m_pixelBuffer.data(),
                  datW,
                  sizeof(float),
                  m_width * sizeof(float),
                  2,
                  2);
            } else {
                m_dataWindow = QRect(datW.min.x, datW.min.y, m_width, m_height);

                Imath::Box2i dispW = part.header().displayWindow();

                int dispW_width  = dispW.max.x - dispW.min.x + 1;
                int dispW_height = dispW.max.y - dispW.min.y + 1;

                m_displayWindow
                  = QRect(dispW.min.x, dispW.min.y, dispW_width, dispW_height);

                m_pixelBuffer.resize(m_width * m_height);

                graySlice = Imf::Slice::Make(
                  Imf::PixelType::FLOAT,
                  m_pixelBuffer.data(),
                  datW);
            }

            Imf::FrameBuffer framebuffer;

            framebuffer.insert(m_layer, graySlice);

            part.setFrameBuffer(framebuffer);
            part.readPixels(datW.min.y, datW.max.y);

            // Determine min and max of the dataset
            m_datasetMin = std::numeric_limits<double>::infinity();
            m_datasetMax = -std::numeric_limits<double>::infinity();

            for (int i = 0; i < m_width * m_height; i++) {
                m_datasetMin = std::min(m_datasetMin, (double)m_pixelBuffer[i]);
                m_datasetMax = std::max(m_datasetMax, (double)m_pixelBuffer[i]);
            }

            m_image         = QImage(m_width, m_height, QImage::Format_RGB888);
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

std::string YFramebufferModel::getColorInfo(int x, int y) const
{
    if (x < 0 || x >= width() || y < 0 || y >= height()) {
        return "";
    }

    std::stringstream ss;
    ss << "x: " << x << " y: " << y << " | "
       << "value = " << m_pixelBuffer[y * width() + x];

    return ss.str();
}

void YFramebufferModel::setMinValue(double value)
{
    m_min = value;
    updateImage();
}

void YFramebufferModel::setMaxValue(double value)
{
    m_max = value;
    updateImage();
}

void YFramebufferModel::setColormap(ColormapModule::Map map)
{
    if (!m_isImageLoaded) {
        return;
    }

    // Several calls can occur within a short time e.g., when changing exposure
    // Ensure to cancel any previous running conversion and wait for the
    // process to end
    // Also, bad idea to change the colormap if a process is using it
    if (m_imageEditingWatcher->isRunning()) {
        m_imageEditingWatcher->cancel();
        m_imageEditingWatcher->waitForFinished();
    }

    if (m_cmap) {
        delete m_cmap;
        m_cmap = nullptr;
    }

    m_cmap = ColormapModule::create(map);

    updateImage();
}

void YFramebufferModel::updateImage()
{
    if (!m_isImageLoaded) {
        return;
    }

    // Several calls can occur within a short time e.g., when changing exposure
    // Ensure to cancel any previous running conversion and wait for the
    // process to end
    if (m_imageEditingWatcher->isRunning()) {
        m_imageEditingWatcher->cancel();
        m_imageEditingWatcher->waitForFinished();
    }

    QFuture<void> imageConverting = QtConcurrent::run([=]() {
        for (int y = 0; y < m_image.height(); y++) {
            unsigned char* line = m_image.scanLine(y);

            #pragma omp parallel for
            for (int x = 0; x < m_image.width(); x++) {
                float value = m_pixelBuffer[y * m_width + x];
                float RGB[3];

                m_cmap->getRGBValue(value, m_min, m_max, RGB);

                for (int c = 0; c < 3; c++) {
                    line[3 * x + c] = qMax(0, qMin(255, int(255 * RGB[c])));
                }
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
