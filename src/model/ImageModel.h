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
#pragma once

#include <QObject>
#include <QImage>
#include <QVector>
#include <QFutureWatcher>
#include <array>

#include <OpenEXR/ImfMultiPartInputFile.h>

class ImageModel: public QObject
{
  Q_OBJECT

public:
  ImageModel(QObject *parent = nullptr);
  virtual ~ImageModel();

  const QImage &getLoadedImage() const { return m_image; }

  void getAveragedPatches(std::vector<float> &values);
  bool isImageLoaded() const { return m_isImageLoaded; }

signals:
  void imageChanged();
  void imageLoaded(int width, int height);
  void exposureChanged(double exposure);
  void loadFailed(QString message);

protected:

  float * m_pixelBuffer;
  QImage  m_image;

  int m_width, m_height;

  bool m_isImageLoaded;

//  QPolygonF          _macbethOutline;

  double m_exposure;

  QFutureWatcher<void> *m_imageLoadingWatcher;
  QFutureWatcher<void> *m_imageEditingWatcher;
};

