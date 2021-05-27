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

#include <model/HeaderItem.h>

#include <OpenEXR/ImfChannelListAttribute.h>

class LayerItem
{
  public:
    LayerItem(LayerItem *parent = nullptr);

    ~LayerItem();

    void addLeaf(const QString channelName, const Imf::Channel *leafChannel);

    HeaderItem *
    constructItemHierarchy(HeaderItem *parent, int partID);

    size_t getNChilds() const;

    bool hasRGBChilds() const;
    bool hasRGBAChilds() const;
    bool hasYCChilds() const;
    bool hasYCAChilds() const;
    bool hasYChild() const;
    bool hasYAChilds() const;
    bool hasAChild() const;

    QString getFullName() const;

  protected:
    LayerItem *getAddLeaf(const QString channelName);

  private:
    QMap<QString, LayerItem *> m_childItems;
    LayerItem *                m_parentItem;

    const Imf::Channel *m_channelPtr;
    QString             m_rootName;

    QString m_channelName;
};
