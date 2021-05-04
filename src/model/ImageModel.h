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

