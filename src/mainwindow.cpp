#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMimeData>
#include <QSettings>

#include <QMdiSubWindow>

// Temporary
#include <QLabel>

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

    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_mdiArea);

    setCentralWidget(m_splitter);

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
    m_treeView->setModel(m_img);
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






