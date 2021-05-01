#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSplitter>
#include <QFileDialog>
#include <QTreeView>
#include <QMdiArea>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_img(nullptr)
{
    ui->setupUi(this);
    QSplitter *splitter = new QSplitter(this);
    m_treeView = new QTreeView(splitter);
    m_treeView->setModel(nullptr);
    QMdiArea *mdiArea = new QMdiArea(splitter);

    splitter->addWidget(m_treeView);
    splitter->addWidget(mdiArea);

    setCentralWidget(splitter);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_img;
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
        if (m_img) {
            m_treeView->setModel(nullptr);
            delete m_img;
            m_img = nullptr;
        }

        m_img = new OpenEXRImage(filename, m_treeView);
        m_treeView->setModel(m_img);
        m_treeView->expandAll();
        //openFile(fileName);
    }
}
