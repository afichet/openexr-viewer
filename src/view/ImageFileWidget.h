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

#pragma once

#include <QWidget>

#include <QMdiArea>
#include <QSplitter>
#include <QTreeView>

#include <model/OpenEXRImage.h>

class ImageFileWidget: public QWidget
{
    Q_OBJECT
  public:
    explicit ImageFileWidget(QWidget *parent = nullptr);

    virtual ~ImageFileWidget();

    QString    getOpenedFolder() const { return m_openedFolder; }
    QString    getOpenedFilename() const { return m_openedFilename; }
    QByteArray getSplitterState() const
    {
        return m_splitterImageView->saveState();
    }

    void setSplitterState(const QByteArray &state)
    {
        m_splitterImageView->restoreState(state);
    }

  public slots:
    void open(const QString &filename);
    void refresh();

    void setTabbed();
    void setCascade();
    void setTiled();

  protected:
    static QString getTitle(const LayerItem *item);
    QString        getTitle(int partId, const std::string &layer) const;
    void           openAttribute(HeaderItem *item);

    void openLayer(const LayerItem *item);


  private slots:
    void onAttributeDoubleClicked(const QModelIndex &index);
    void onLayerDoubleClicked(const QModelIndex &index);

    void onLoadFailed(const QString &msg);

  private:
    QSplitter *m_splitterImageView;
    QSplitter *m_splitterProperties;
    QTreeView *m_attributesTreeView;
    QTreeView *m_layersTreeView;
    QMdiArea * m_mdiArea;

    OpenEXRImage *m_img;
    QString       m_openedFolder;
    QString       m_openedFilename;
};
