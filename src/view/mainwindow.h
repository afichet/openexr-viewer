#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QMdiArea>
#include <QSplitter>

#include <QCloseEvent>
#include <QDropEvent>

#include <model/OpenEXRImage.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void open(QString filename);

private slots:
    void on_action_Open_triggered();
    void on_action_Quit_triggered();

protected:
    void closeEvent(QCloseEvent * event) override;
//    void showEvent(QShowEvent* event) override;

private:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;

    void writeSettings();
    void readSettings();

private slots:
    void onDoubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    QSplitter* m_splitter;
    QTreeView* m_treeView;
    QMdiArea* m_mdiArea;

    OpenEXRImage* m_img;
};
#endif // MAINWINDOW_H
