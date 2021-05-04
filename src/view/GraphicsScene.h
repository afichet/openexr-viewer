#pragma once

#include <QGraphicsScene>

class GraphicsScene: public QGraphicsScene
{
  Q_OBJECT
public:
  virtual ~GraphicsScene();

  GraphicsScene(QObject *parent = 0);

  void dragEnterEvent(QGraphicsSceneDragDropEvent *event);

  void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

  void dropEvent(QGraphicsSceneDragDropEvent *event);

signals:
  void openFileOnDropEvent(QString);
};

