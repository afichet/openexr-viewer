#include <view/mainwindow.h>

#include <QApplication>

#if defined(WIN32)

#include <Windows.h>

int CALLBACK WinMain(
        _In_ HINSTANCE /*hInstance*/,
        _In_ HINSTANCE /*hPrevInstance*/,
        _In_ LPSTR     /*lpCmdLine*/,
        _In_ int       /*nCmdShow*/
        )
{
    int         argc = 0;

    char**      argv = nullptr;
    argc = __argc;

    argv = __argv;

#else
int main(int argc, char *argv[])
{
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
