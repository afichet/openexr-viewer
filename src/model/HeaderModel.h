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

#include <QAbstractItemModel>

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfMultiPartInputFile.h>

#include <model/HeaderItem.h>
#include <model/LayerItem.h>

class HeaderModel: public QAbstractItemModel
{
  public:
    HeaderModel(int n_parts, QObject *parent);

    ~HeaderModel();

    void addFile(const Imf::MultiPartInputFile &file, const QString &filename);

    const std::vector<LayerItem *> &getLayers() const
    {
        return m_partRootLayer;
    }


    /**
     * Qt logic for accessing the model
     */
    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(
      int             section,
      Qt::Orientation orientation,
      int             role = Qt::DisplayRole) const override;

    QModelIndex index(
      int                row,
      int                column,
      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;


  private:
    void addHeader(
      const Imf::Header &header,
      HeaderItem *       root,
      const QString      partName,
      int                partID);

    HeaderItem *addItem(
      const char *          name,
      const Imf::Attribute &attr,
      HeaderItem *          parent,
      QString               partName,
      int                   part_number);

  private:
    HeaderItem *                          m_rootItem;
    std::vector<std::vector<std::string>> m_headerItems;

    std::vector<LayerItem *> m_partRootLayer;
};
