#include <view/mainwindow.h>

#include <QApplication>
#include <QFile>

#if defined(WIN32)

#include <Windows.h>

int CALLBACK WinMain(
        _In_ HINSTANCE /*hInstance*/,
        _In_ HINSTANCE /*hPrevInstance*/,
        _In_ LPSTR     /*lpCmdLine*/,
        _In_ int       /*nCmdShow*/
        )
{
    int    argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char *argv[])
{
#endif
    QApplication a(argc, argv);

    QFile f(":/dark_flat/theme.css");

    if (!f.exists()) {
        qWarning() << "Unable to set stylesheet, file not found";
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        a.setStyleSheet(ts.readAll());
    }


    MainWindow w;
    w.show();

    if (argc > 1) {
      w.open(argv[1]);
    }

    return a.exec();
}
