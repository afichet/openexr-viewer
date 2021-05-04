#include "GraphicsScene.h"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>

GraphicsScene::GraphicsScene(QObject *parent): QGraphicsScene(parent) {}


GraphicsScene::~GraphicsScene() {}

void GraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
}

void GraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->setAccepted(true);
}

void GraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();

    if (!urls.empty())
    {
        QString fileName = urls[0].toString();
        QString startFileTypeString =
        #ifdef _WIN32
                "file:///";
#else
                "file://";
#endif

        if (fileName.startsWith(startFileTypeString))
        {
            fileName = fileName.remove(0, startFileTypeString.length());
            emit openFileOnDropEvent(fileName);
        }
    }
}
