//
// Copyright (c) 2021 Alban Fichet <alban.fichet at gmx.fr>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
//  * Neither the name of %ORGANIZATION% nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include "OpenEXRHeaderItem.h"


OpenEXRHeaderItem::OpenEXRHeaderItem(OpenEXRHeaderItem *parentItem,
        const QVector<QVariant> &data,
        QString name, int partID)
    : m_itemData(data)
    , m_parentItem(parentItem)
    , m_name(name)
    , m_partID(partID)
{
    // If not root item
    if (m_parentItem) {
        m_parentItem->appendChild(this);
    }
}


OpenEXRHeaderItem::~OpenEXRHeaderItem() {
    qDeleteAll(m_childItems);
}


void OpenEXRHeaderItem::appendData(QVariant sibbling) {
    m_itemData.append(sibbling);
}


void OpenEXRHeaderItem::setData(QVector<QVariant> data) {
    m_itemData = data;
}


OpenEXRHeaderItem *OpenEXRHeaderItem::child(int row) {
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }

    return m_childItems.at(row);
}


int OpenEXRHeaderItem::childCount() const {
    return m_childItems.size();
}


int OpenEXRHeaderItem::columnCount() const {
    return m_itemData.size();
}


QVariant OpenEXRHeaderItem::data(int column) const {
    if (column < 0 || column >= m_itemData.size()) {
        return QVariant();
    }

    return m_itemData.at(column);
}


int OpenEXRHeaderItem::row() const {
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<OpenEXRHeaderItem*>(this));
    }

    return 0;
}


OpenEXRHeaderItem *OpenEXRHeaderItem::parentItem() {
    return m_parentItem;
}
