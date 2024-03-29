/**
 * Copyright (c) 2021 Alban Fichet <alban dot fichet at gmx dot fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *  * Neither the name of the organization(s) nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <view/mainwindow.h>
#include <config.h>
#include <QApplication>
#include <QFile>

#if defined(WIN32)

#    include <Windows.h>

int CALLBACK WinMain(
  _In_ HINSTANCE /*hInstance*/,
  _In_ HINSTANCE /*hPrevInstance*/,
  _In_ LPSTR /*lpCmdLine*/,
  _In_ int /*nCmdShow*/
)
{
    int    argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char* argv[])
{
#endif
    QApplication a(argc, argv);
    a.setApplicationVersion(CMAKE_PROJECT_VERSION);

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
        if (strcmp(argv[1], "--") == 0) {
            std::cerr << "Reading from stdin not yet supported." << std::endl;
            exit(1);
            // Read from stdin
            w.open(std::cin);
        } else {
            for (int i = 1; i < argc; i++) {
                w.open(argv[i]);
            }
        }
    }

    return a.exec();
}
