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
#include "FramebufferModel.h"

#include <util/ColormapModule.h>

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
    , m_cmap(ColormapModule::create("grayscale"))
{
    Imf::InputPart part(file, partId);

    Imath::Box2i dw = part.header().dataWindow();
    m_width  = dw.max.x - dw.min.x + 1;
    m_height = dw.max.y - dw.min.y + 1;

    // TODO viewport

    Imf::Slice graySlice;
    // TODO: Check it that can be guess from the header
    // also, check if this can be nested
    if (layerName == "BY" || layerName == "RY") {
        m_width /= 2;
        m_height /= 2;

        m_pixelBuffer = new float[m_width * m_height];

        // Luminance Chroma channels
        graySlice = Imf::Slice::Make(
                    Imf::PixelType::FLOAT,
                    m_pixelBuffer,
                    dw,
                    sizeof(float), m_width * sizeof(float),
                    2, 2
        );
    } else {
        m_pixelBuffer = new float[m_width * m_height];

        graySlice = Imf::Slice::Make(
                Imf::PixelType::FLOAT,
                m_pixelBuffer,
                dw);
    }

    Imf::FrameBuffer framebuffer;

    framebuffer.insert(m_layer.toStdString().c_str(), graySlice);

    part.setFrameBuffer(framebuffer);
    part.readPixels(dw.min.y, dw.max.y);

    updateImage();
}

FramebufferModel::~FramebufferModel()
{
    delete m_cmap;
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

void FramebufferModel::setColormap(const QString &value)
{
    // Bad idea to change the colormap if a process is using it
    if (m_imageLoadingWatcher->isRunning()) {
        m_imageLoadingWatcher->cancel();
        m_imageLoadingWatcher->waitForFinished();
    }

    if (m_cmap) {
        delete m_cmap;
        m_cmap = nullptr;
    }

    m_cmap = ColormapModule::create(value.toLower().toStdString());

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
        m_image = QImage(m_width, m_height, QImage::Format_RGB888);

        for (int y = 0; y < m_image.height(); y++) {
            unsigned char * line = m_image.scanLine(y);

            #pragma omp parallel for
            for (int x = 0; x < m_image.width(); x++) {
                float value = m_pixelBuffer[y * m_width + x];
                float RGB[3];

                m_cmap->getRGBValue(value, m_min, m_max, RGB);

                for (int c = 0; c < 3; c++) {
                    line[3 * x + c] = qMax(0, qMin(255, int(255 * RGB[c])));
                }
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
