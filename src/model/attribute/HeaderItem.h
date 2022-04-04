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

#include <QVariant>
#include <QVector>

class LayerItem;

class HeaderItem
{
  public:
    HeaderItem(
      HeaderItem*              parentItem = nullptr,
      const QVector<QVariant>& data       = QVector<QVariant>(),
      QString                  partName   = QString(),
      int                      partID     = 0,
      QString                  itemName   = QString(),
      LayerItem*               layerItem  = nullptr);

    //    explicit HeaderItem(
    //      HeaderItem *             parentItem = nullptr,
    //      const QVector<QVariant> &data       = QVector<QVariant>(),
    //      std::string              partName   = "",
    //      int                      partID     = 0,
    //      std::string              itemName   = "");

    ~HeaderItem();

    void appendData(QVariant sibbling);

    void setData(QVector<QVariant> data);

    HeaderItem* child(int row);

    int childCount() const;
    int columnCount() const;

    QVariant data(int column) const;

    int row() const;

    HeaderItem* parentItem();

    QString type() const { return m_itemData[2].toString(); }

    const QString& getPartName() const { return m_partName; }
    int            getPartID() const { return m_partID; }
    const QString& getItemName() const { return m_itemName; }

    // TODO: Hacky for now...
    LayerItem* getLayerItem() const { return m_layerItem; }

    void setPartName(const QString& name) { m_partName = name; }
    void setPartID(int partID) { m_partID = partID; }
    void setItemName(const QString& name) { m_itemName = name; }

  protected:
    void appendChild(HeaderItem* child) { m_childItems.append(child); }

  private:
    QVector<QVariant>    m_itemData;
    HeaderItem*          m_parentItem;
    QVector<HeaderItem*> m_childItems;

    QString m_partName;
    QString m_itemName;
    int     m_partID;

    // TODO: Hacky for now...
    LayerItem* m_layerItem;
};
