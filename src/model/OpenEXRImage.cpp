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

#include "OpenEXRImage.h"
#include "StdIStream.h"

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>

#include <Imath/ImathBox.h>


OpenEXRImage::OpenEXRImage(const QString& filename, QObject* parent)
  : QObject(parent)
  , m_filename(filename)
  , m_isStream(false)
  , m_exrIn(new Imf::MultiPartInputFile(filename.toStdString().c_str()))
  , m_headerModel(nullptr)
  , m_layerModel(nullptr)
{
    m_headerModel = new HeaderModel(*m_exrIn, m_exrIn->parts(), this);
    m_headerModel->addFile(*m_exrIn, filename);

    m_layerModel = new LayerModel(*m_exrIn, this);
}


OpenEXRImage::OpenEXRImage(std::istream& stream, QObject* parent)
  : QObject(parent)
  , m_isStream(true)
  //  , m_exrIn(StdIStream(stream))
  , m_headerModel(nullptr)
  , m_layerModel(nullptr)
{
    StdIStream s(stream);
    m_exrIn = new Imf::MultiPartInputFile(s);

    m_headerModel = new HeaderModel(*m_exrIn, m_exrIn->parts(), this);
    m_headerModel->addFile(*m_exrIn, "Stream");

    m_layerModel = new LayerModel(*m_exrIn, this);
}


OpenEXRImage::~OpenEXRImage()
{
    delete m_headerModel;
    delete m_layerModel;
    delete m_exrIn;
}
