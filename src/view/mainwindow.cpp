#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMimeData>
#include <QSettings>

#include <QMdiSubWindow>

#include "RGBFramebufferWidget.h"
#include "FramebufferWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_splitter(new QSplitter(this))
    , m_img(nullptr)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    m_treeView = new QTreeView(m_splitter);
    m_mdiArea = new QMdiArea(m_splitter);
    m_mdiArea->setViewMode(QMdiArea::TabbedView);

    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_mdiArea);

    setCentralWidget(m_splitter);

    connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onDoubleClicked(QModelIndex)));

    readSettings();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete m_img;
}


void MainWindow::open(QString filename)
{
    if (m_img) {
        m_treeView->setModel(nullptr);
        m_mdiArea->closeAllSubWindows();
        delete m_img;
        m_img = nullptr;
    }

    m_img = new OpenEXRImage(filename, m_treeView);
    m_treeView->setModel(m_img->getHeaderModel());
    m_treeView->expandAll();

//    QMdiSubWindow *subWindow1 = new QMdiSubWindow;

//    subWindow1->setWidget(new QLabel("Toto"));
//    subWindow1->setAttribute(Qt::WA_DeleteOnClose);
//    QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(subWindow1);
//    subWindow->show();

}


void MainWindow::on_action_Open_triggered()
{
    const QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open OpenEXR Image"),
                QDir::homePath(),
                tr("Images (*.exr)")
                );

    if (filename.size() != 0) {
        open(filename);
    }
}


void MainWindow::on_action_Quit_triggered()
{
    close();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void MainWindow::dropEvent(QDropEvent *ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();

    if (!urls.empty())
    {
        QString filename = urls[0].toString();
        QString startFileTypeString =
            #ifdef _WIN32
                "file:///";
            #else
                "file://";
            #endif

        if (filename.startsWith(startFileTypeString))
        {
            filename = filename.remove(0, startFileTypeString.length());
            open(filename);
        }
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}


void MainWindow::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "afichet", "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry"      , saveGeometry());
    settings.setValue("state"         , saveState());
    settings.setValue("splitter"      , m_splitter->saveState());
    settings.endGroup();
}


void MainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "afichet", "OpenEXR Viewer");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();
}

void MainWindow::onDoubleClicked(const QModelIndex &index)
{
    OpenEXRHeaderItem* item = static_cast<OpenEXRHeaderItem*>(index.internalPointer());

    if (item->type() == "framebuffer") {
        QString title =
                QString("Part: ") + QString::number(item->getPartID()) + " " +
                QString("Layer: ") + item->getName();

        // Check if the window already exists
        bool windowExists = false;
        for (auto& w: m_mdiArea->subWindowList()) {

            if (w->windowTitle() == title) {
                w->setFocus();
                windowExists = true;
                break;
            }
        }

        // If the window does not exist yet, create it
        if (!windowExists) {
            FramebufferWidget *graphicView = new FramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createImageModel(item->getPartID(), item->getName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    } else if (item->type() == "RGB framebuffer") {
        QString title =
                QString("Part: ") + QString::number(item->getPartID()) + " " +
                QString("Layer: ") + item->getName() + "RGB";

        // Check if the window already exists
        bool windowExists = false;
        for (auto& w: m_mdiArea->subWindowList()) {

            if (w->windowTitle() == title) {
                w->setFocus();
                windowExists = true;
                break;
            }
        }

        // If the window does not exist yet, create it
        if (!windowExists) {
            RGBFramebufferWidget *graphicView = new RGBFramebufferWidget(m_mdiArea);
            graphicView->setModel(m_img->createRGBImageModel(item->getPartID(), item->getName()));

            QMdiSubWindow* subWindow = m_mdiArea->addSubWindow(graphicView);

            subWindow->setWindowTitle(title);
            subWindow->show();
        }
    }
}






