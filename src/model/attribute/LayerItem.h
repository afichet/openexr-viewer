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

#include <model/attribute/HeaderItem.h>

#include <OpenEXR/ImfChannelListAttribute.h>

#include <QVariant>
#include <QImage>

#include <vector>

class LayerItem
{
  public:
    enum LayerType {
        // Single channel
        R, G, B, A,
        Y, RY, BY,
        GENERAL,
        // Group Types,
        RGB, RGBA,
        YA, YC, YCA,
        GROUP, PART,
        N_LAYERTYPES
    };

    LayerItem(
            Imf::MultiPartInputFile &file,
            LayerItem *pParent = nullptr,
            const std::string& leafName = "",
            const std::string& originalChannelName = "",
            const Imf::Channel *pChannel = nullptr,
            int part = -1);

    ~LayerItem();

    LayerItem *addLeaf(
            Imf::MultiPartInputFile &file,
            const std::string& channelName,
            const Imf::Channel* pChannel,
            int part = -1
            );


    void createThumbnails();

    // Perfoms the grouping of known layer groups: RGB, RGBA, YC, YCA...
    void groupLayers();

    HeaderItem *constructItemHierarchy(
            HeaderItem *parent,
            const std::string &partName,
            int partID
            );

    LayerItem *child(int index) const;
    LayerItem *child(const std::string& name) const;
    LayerItem *child(const LayerType& type) const;

    int childIndex(const std::string& name) const;
    int childIndex(const LayerType& type) const;

    int childCount() const;

    const std::vector<LayerItem*> & children() const { return m_childItems; }

    LayerItem *parentItem() { return m_pParentItem; }

    bool hasChild(const std::string& name) const;
    bool hasChildLeaf(const std::string& name) const;
    bool hasChildLeaf(const LayerType& type) const;

    bool hasRGBChildLeafs() const;
    bool hasRGBAChildLeafs() const;
    bool hasYCChildLeafs() const;
    bool hasYCAChildLeafs() const;
    bool hasYChildLeaf() const;
    bool hasYAChildLeafs() const;
    bool hasAChildLeaf() const;


    std::string getFullName() const;
    std::string getLeafName() const;
    std::string getOriginalFullName() const;
    int getPart() const;

    const QImage& getPreview() const;

//    void printHierarchy(std::string front) const;

    LayerType getType() const { return m_type; }

  private:
    LayerType constructType();

    void createThumbnails(LayerItem* item);
    void createThumbnail();



    std::vector<LayerItem*> m_childItems;
    LayerItem *             m_pParentItem;

    // Name of the root hierarchy (removes double '.')
    std::string m_rootName;

    // Name of the current leaf
    std::string m_leafName;

    // Full original channel name
    std::string m_channelName;

    LayerType m_type;

    Imf::MultiPartInputFile& m_fileHandle;
    const Imf::Channel *m_pChannel;
    const int m_part;

    int m_previewSize;

    QImage m_preview;
    uchar *m_previewBuffer;
};
