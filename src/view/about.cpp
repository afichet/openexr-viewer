#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent): QDialog(parent), ui(new Ui::About)
{
    ui->setupUi(this);

    ui->textBrowser->setHtml(
      "<h1>OpenEXR Viewer</h1>"
      "<p><em>"
      + tr("Version ") + QApplication::applicationVersion() + "</em></p>"
      + tr(
        "<h2>Author</h2>                                                        \
                    </p>Alban Fichet &lt;alban.fichet@gmx.fr&gt;</p>                              \
                                                                                            \
                    <h2>Licence GPLv3.</h2>                                                 \
                    <p><strong>Copyright (c) Alban Fichet 2021.</strong></p>                \
                    <p>                                                                     \
                    This program is free software: you can redistribute it and/or modify    \
                    it under the terms of the GNU General Public License as published by    \
                    the Free Software Foundation, either version 3 of the License, or       \
                    (at your option) any later version.</p>                                 \
                    <p>                                                                     \
                    This program is distributed in the hope that it will be useful,         \
                    but WITHOUT ANY WARRANTY; without even the implied warranty of          \
                    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           \
                    GNU General Public License for more details.</p>                        \
                    <p>                                                                     \
                    You should have received a copy of the GNU General Public License       \
                    along with this program.  If not, see <https://www.gnu.org/licenses/>.  \
                    </p>                                                                    \
                                                                                            \
                    <h2>Icons</h2>                                                          \
                    <div>Icons made by                                                      \
                    <a href=\"https://www.flaticon.com/authors/dinosoftlabs\"               \
                       title=\"DinosoftLabs\">DinosoftLabs</a> from                         \
                    <a href=\"https://www.flaticon.com/\"                                   \
                       title=\"Flaticon\">www.flaticon.com</a></div>                        \
                    "));
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    close();
}
