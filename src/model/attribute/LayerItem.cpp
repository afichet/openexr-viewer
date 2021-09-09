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

#include "LayerItem.h"

#include <util/ColorTransform.h>

#include <cassert>

#include <QString>
#include <QPainter>

#include <ImfMultiPartInputFile.h>
#include <ImfFrameBuffer.h>
#include <ImfInputPart.h>
#include <ImfHeader.h>

LayerItem::LayerItem(
        Imf::MultiPartInputFile &file,
        LayerItem *pParent,
        const std::string& leafName,
        const std::string& originalChannelName,
        const Imf::Channel *pChannel,
        int part)
    : m_pParentItem(pParent)
    , m_part(part)
    , m_rootName("")
    , m_leafName(leafName)
    , m_channelName(originalChannelName)
    , m_fileHandle(file)
    , m_pChannel(pChannel)
    , m_previewSize(64)
    , m_previewBuffer(new uchar[4 * m_previewSize * m_previewSize])
{
    if (pParent) {
        m_rootName = pParent->getFullName();
        pParent->m_childItems.push_back(this);
    }

    m_type = constructType();
}

LayerItem::~LayerItem()
{
    for (LayerItem* it: m_childItems) {
        delete it;
    }

    delete[] m_previewBuffer;
}

LayerItem *LayerItem::addLeaf(
        Imf::MultiPartInputFile &file,
        const std::string& channelName,
        const Imf::Channel* pChannel,
        int part)
{
    QStringList channelHierachy = QString::fromStdString(channelName).split(".");

    LayerItem *pLeafPtr = this;

    for (auto &leafName : channelHierachy) {
        LayerItem *pExistingLeaf = pLeafPtr->child(leafName.toStdString());

        if (pExistingLeaf != nullptr) {
            pLeafPtr = pExistingLeaf;
        } else {
            LayerItem *pNewLeaf  = new LayerItem(
                        file,
                        pLeafPtr,
                        leafName.toStdString(),
                        "",
                        nullptr,
                        part);

            pLeafPtr = pNewLeaf;
        }
    }

    // Sanity check
    if (pLeafPtr->m_pChannel) {
        std::cerr << "The leaf is already populated with a framebuffer!" << std::endl;
        std::cerr << "Leaf dump:" << std::endl
                  << "----------" << std::endl
                  << "channel name: " << pLeafPtr->m_channelName << std::endl
                  << "root name:    " << pLeafPtr->m_rootName << std::endl
                  << "leaf name:    " << pLeafPtr->m_leafName << std::endl;

        assert(0);
    }

    // Saves the original full channel name
    pLeafPtr->m_channelName = channelName;
    pLeafPtr->m_pChannel    = pChannel;

    // Determine channel type based on the leaf name
    pLeafPtr->m_type = pLeafPtr->constructType();

    return pLeafPtr;
}

void LayerItem::createThumbnails()
{
    createThumbnails(this);
}


void LayerItem::groupLayers()
{
    if (hasRGBAChildLeafs()) {
        // TODO: Get channel name... a bit hacky for now
        LayerItem* item = child(LayerItem::R);
        std::string layerName = item->m_channelName.substr(0, item->m_channelName.size() - 1);

        LayerItem* rgbaRoot = new LayerItem(
                    m_fileHandle,
                    this,
                    "RGBA",
                    layerName,
                    nullptr,
                    m_part);

        int rIdx = childIndex(LayerItem::R);
        rgbaRoot->m_childItems.push_back(m_childItems[rIdx]);
        m_childItems[rIdx]->m_pParentItem = rgbaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), rIdx));

        int gIdx = childIndex(LayerItem::G);
        rgbaRoot->m_childItems.push_back(m_childItems[gIdx]);
        m_childItems[gIdx]->m_pParentItem = rgbaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), gIdx));

        int bIdx = childIndex(LayerItem::B);
        rgbaRoot->m_childItems.push_back(m_childItems[bIdx]);
        m_childItems[bIdx]->m_pParentItem = rgbaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), bIdx));

        int aIdx = childIndex(LayerItem::A);
        rgbaRoot->m_childItems.push_back(m_childItems[aIdx]);
        m_childItems[aIdx]->m_pParentItem = rgbaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), aIdx));

        for (LayerItem* it: m_childItems) {
            if (it->m_type != LayerType::RGBA) {
                it->groupLayers();
            }
        }
    } else if (hasRGBChildLeafs()) {
        // TODO: Get channel name... a bit hacky for now
        LayerItem* item = child(LayerItem::R);
        std::string layerName = item->m_channelName.substr(0, item->m_channelName.size() - 1);

        LayerItem* rgbRoot = new LayerItem(
                    m_fileHandle,
                    this,
                    "RGB",
                    layerName,
                    nullptr,
                    m_part);

        int rIdx = childIndex(LayerItem::R);
        rgbRoot->m_childItems.push_back(m_childItems[rIdx]);
        m_childItems[rIdx]->m_pParentItem = rgbRoot;
        m_childItems.erase(std::next(m_childItems.begin(), rIdx));

        int gIdx = childIndex(LayerItem::G);
        rgbRoot->m_childItems.push_back(m_childItems[gIdx]);
        m_childItems[gIdx]->m_pParentItem = rgbRoot;
        m_childItems.erase(std::next(m_childItems.begin(), gIdx));

        int bIdx = childIndex(LayerItem::B);
        rgbRoot->m_childItems.push_back(m_childItems[bIdx]);
        m_childItems[bIdx]->m_pParentItem = rgbRoot;
        m_childItems.erase(std::next(m_childItems.begin(), bIdx));

        for (LayerItem* it: m_childItems) {
            if (it->m_type != LayerType::RGB) {
                it->groupLayers();
            }
        }
    } else if (hasYCAChildLeafs()) {
        // TODO: Get channel name... a bit hacky for now
        LayerItem* item = child(LayerItem::Y);
        std::string layerName = item->m_channelName.substr(0, item->m_channelName.size() - 1);

        LayerItem* ycaRoot = new LayerItem(
                    m_fileHandle,
                    this,
                    "YCA",
                    layerName,
                    nullptr,
                    m_part);

        int yIdx = childIndex(LayerItem::Y);
        ycaRoot->m_childItems.push_back(m_childItems[yIdx]);
        m_childItems[yIdx]->m_pParentItem = ycaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), yIdx));

        int ryIdx = childIndex(LayerItem::RY);
        ycaRoot->m_childItems.push_back(m_childItems[ryIdx]);
        m_childItems[ryIdx]->m_pParentItem = ycaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), ryIdx));

        int byIdx = childIndex(LayerItem::BY);
        ycaRoot->m_childItems.push_back(m_childItems[byIdx]);
        m_childItems[byIdx]->m_pParentItem = ycaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), byIdx));

        int aIdx = childIndex(LayerItem::A);
        ycaRoot->m_childItems.push_back(m_childItems[aIdx]);
        m_childItems[aIdx]->m_pParentItem = ycaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), aIdx));

        for (LayerItem* it: m_childItems) {
            if (it->m_type != LayerType::YCA) {
                it->groupLayers();
            }
        }

    } else if (hasYCChildLeafs()) {
        // TODO: Get channel name... a bit hacky for now
        LayerItem* item = child(LayerItem::Y);
        std::string layerName = item->m_channelName.substr(0, item->m_channelName.size() - 1);

        LayerItem* ycRoot = new LayerItem(
                    m_fileHandle,
                    this,
                    "YC",
                    layerName,
                    nullptr,
                    m_part);

        int yIdx = childIndex(LayerItem::Y);
        ycRoot->m_childItems.push_back(m_childItems[yIdx]);
        m_childItems[yIdx]->m_pParentItem = ycRoot;
        m_childItems.erase(std::next(m_childItems.begin(), yIdx));

        int ryIdx = childIndex(LayerItem::RY);
        ycRoot->m_childItems.push_back(m_childItems[ryIdx]);
        m_childItems[ryIdx]->m_pParentItem = ycRoot;
        m_childItems.erase(std::next(m_childItems.begin(), ryIdx));

        int byIdx = childIndex(LayerItem::BY);
        ycRoot->m_childItems.push_back(m_childItems[byIdx]);
        m_childItems[byIdx]->m_pParentItem = ycRoot;
        m_childItems.erase(std::next(m_childItems.begin(), byIdx));

        for (LayerItem* it: m_childItems) {
            if (it->m_type != LayerType::YC) {
                it->groupLayers();
            }
        }
    } else if (hasYAChildLeafs()) {
        // TODO: Get channel name... a bit hacky for now
        LayerItem* item = child(LayerItem::Y);
        std::string layerName = item->m_channelName.substr(0, item->m_channelName.size() - 1);

        LayerItem* yaRoot = new LayerItem(
                    m_fileHandle,
                    this,
                    "YA",
                    layerName,
                    nullptr,
                    m_part);

        int yIdx = childIndex(LayerItem::Y);
        yaRoot->m_childItems.push_back(m_childItems[yIdx]);
        m_childItems[yIdx]->m_pParentItem = yaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), yIdx));

        int aIdx = childIndex(LayerItem::A);
        yaRoot->m_childItems.push_back(m_childItems[aIdx]);
        m_childItems[aIdx]->m_pParentItem = yaRoot;
        m_childItems.erase(std::next(m_childItems.begin(), aIdx));

        for (LayerItem* it: m_childItems) {
            if (it->m_type != LayerType::YA) {
                it->groupLayers();
            }
        }
    } else {
        // No grouping so far...
        for (LayerItem* it: m_childItems) {
            it->groupLayers();
        }
    }
}


HeaderItem *LayerItem::constructItemHierarchy(
        HeaderItem *parent,
        const std::string &partName,
        int partID)
{
    if (m_childItems.size() == 0) {
        // This is a terminal leaf
        assert(m_pChannel != nullptr);
        return new HeaderItem(
          parent,
          {QString::fromStdString(m_leafName), "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_leafName));
    }

    HeaderItem *currRoot = nullptr;

    // Avoid empty root on top level
    if (m_pParentItem) {
        currRoot = new HeaderItem(
                    parent,
                    {QString::fromStdString(m_leafName), (int)childCount(), "channellist"},
                    QString::fromStdString(partName),
                    partID,
                    QString::fromStdString(m_leafName));
    } else {
        currRoot = parent;
    }

    if (m_pChannel) {
        // It's a leaf...
        // Both are valid but I prefer the nested representation
        // OpenEXRItem* leafNode = new OpenEXRItem(parent, {m_rootName, "",
        // "framebuffer"});
        new HeaderItem(
          currRoot,
          {".", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_leafName));
    }

    QStringList ignoredKeys;

    // If we find RGB final leaf, we make a virtual group
    if (hasRGBAChildLeafs()) {
        HeaderItem *rgbaRoot = new HeaderItem(
          currRoot,
          {"RGBA", "", "RGB framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_rootName));

        new HeaderItem(
          rgbaRoot,
          {"R", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "R");
        new HeaderItem(
          rgbaRoot,
          {"G", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "G");
        new HeaderItem(
          rgbaRoot,
          {"B", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "B");
        new HeaderItem(
          rgbaRoot,
          {"A", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "A");

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
        ignoredKeys.append("A");
    } else if (hasRGBChildLeafs()) {
        HeaderItem *rgbRoot = new HeaderItem(
          currRoot,
          {"RGB", "", "RGB framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_rootName));

        new HeaderItem(
          rgbRoot,
          {"R", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "R");
        new HeaderItem(
          rgbRoot,
          {"G", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "G");
        new HeaderItem(
          rgbRoot,
          {"B", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "B");

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
    }

    // Same for Luminance Chroma images
    // Not an else, can be both
    // Note if there is both + Alpha channel, that is quite odd, alpha is then
    // shared between to groups
    if (hasYCAChildLeafs()) {
        HeaderItem *ycaRoot = new HeaderItem(
          currRoot,
          {"YCA", "", "YC framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_rootName));

        new HeaderItem(
          ycaRoot,
          {"Y", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "Y");
        new HeaderItem(
          ycaRoot,
          {"RY", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "RY");
        new HeaderItem(
          ycaRoot,
          {"BY", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "BY");
        new HeaderItem(
          ycaRoot,
          {"A", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "A");

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
        ignoredKeys.append("A");
    } else if (hasYCChildLeafs()) {
        HeaderItem *ycRoot = new HeaderItem(
          currRoot,
          {"YC", "", "YC framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_rootName));

        new HeaderItem(
          ycRoot,
          {"Y", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "Y");
        new HeaderItem(
          ycRoot,
          {"RY", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "RY");
        new HeaderItem(
          ycRoot,
          {"BY", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "BY");

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
    } else if (hasYAChildLeafs()) {
        HeaderItem *yaRoot = new HeaderItem(
          currRoot,
          {"YA", "", "Y framebuffer"},
          QString::fromStdString(partName),
          partID,
          QString::fromStdString(m_rootName));

        new HeaderItem(
          yaRoot,
          {"Y", "", "Luminance framebuffer"},
          QString::fromStdString(partName),
          partID,
          "Y");
        new HeaderItem(
          yaRoot,
          {"A", "", "framebuffer"},
          QString::fromStdString(partName),
          partID,
          "A");

        ignoredKeys.append("Y");
        ignoredKeys.append("A");
    } else if (hasYChildLeaf()) {
        new HeaderItem(
          currRoot,
          {"Y", "", "Luminance framebuffer"},
          QString::fromStdString(partName),
          partID,
          "Y");

        ignoredKeys.append("Y");
    }

    for (LayerItem* it: m_childItems) {
        if (!ignoredKeys.contains(QString::fromStdString(it->m_leafName)) || it->childCount() != 0) {
            it->constructItemHierarchy(currRoot, partName, partID);
        }
    }

    return currRoot;
}

/* ----------------------------------------------------------------------------
 * Child introspection and access functions
 * ------------------------------------------------------------------------- */

LayerItem *LayerItem::child(int index) const
{
    return m_childItems[index];
}


LayerItem *LayerItem::child(const std::string &name) const
{
    for (LayerItem* it: m_childItems) {
        if (it->m_leafName == name) {
            return it;
        }
    }

    return nullptr;
}


LayerItem *LayerItem::child(const LayerType &type) const
{
    for (LayerItem* it: m_childItems) {
        if (it->m_type == type) {
            return it;
        }
    }

    return nullptr;
}


int LayerItem::childIndex(const std::string &name) const
{
    for (size_t i = 0; i < m_childItems.size(); i++) {
        if (m_childItems[i]->m_leafName == name) {
            return i;
        }
    }

    return -1;
}


int LayerItem::childIndex(const LayerType &type) const
{
    for (size_t i = 0; i < m_childItems.size(); i++) {
        if (m_childItems[i]->m_type == type) {
            return i;
        }
    }

    return -1;
}


int LayerItem::childCount() const
{
    return m_childItems.size();
}


bool LayerItem::hasChild(const std::string &name) const
{
    if (child(name) != nullptr) {
        return true;
    }

    return false;
}


bool LayerItem::hasChildLeaf(const std::string &name) const
{
    LayerItem *childItem = child(name);

    if (childItem != nullptr) {
        return childItem->m_pChannel != nullptr;
    }

    return false;
}


bool LayerItem::hasChildLeaf(const LayerType &type) const
{
    LayerItem *childItem = child(type);

    if (childItem != nullptr) {
        return childItem->m_pChannel != nullptr;
    }

    return false;
}


bool LayerItem::hasRGBChildLeafs() const
{
    return hasChildLeaf(R)
        && hasChildLeaf(G)
        && hasChildLeaf(B);
}


bool LayerItem::hasRGBAChildLeafs() const
{
    return hasRGBChildLeafs()
        && hasAChildLeaf();
}


bool LayerItem::hasYCChildLeafs() const
{
    return hasYChildLeaf()
        && hasChildLeaf(RY)
        && hasChildLeaf(BY);
}

bool LayerItem::hasYCAChildLeafs() const
{
    return hasYCChildLeafs()
        && hasAChildLeaf();
}

bool LayerItem::hasYChildLeaf() const
{
    return hasChildLeaf(Y);
}

bool LayerItem::hasYAChildLeafs() const
{
    return hasYChildLeaf() && hasAChildLeaf();
}

bool LayerItem::hasAChildLeaf() const
{
    return hasChildLeaf(A);
}

/* ----------------------------------------------------------------------------
 * Layer names
 * ------------------------------------------------------------------------- */

std::string LayerItem::getFullName() const
{
    if (m_pParentItem) {
        return m_pParentItem->getFullName() + "." + m_leafName;
    }

    return m_leafName;
}

std::string LayerItem::getLeafName() const
{
    return m_leafName;
}

std::string LayerItem::getOriginalFullName() const
{
    return m_channelName;
}

int LayerItem::getPart() const
{
    return (m_part == -1) ? 0 : m_part; // TODO
    LayerItem const* item = this;

    // Go to the parent untill getting a part
    while (item->m_type != PART) {
        item = item->m_pParentItem;

        // No part so it is single part file, we return 0
        if (item == nullptr) return 0;
    }

    // Check if the item has a valid part ID
    assert(item->m_part >= 0);

    return item->m_part;
}


bool LayerItem::hasPartName() const
{
    // Single part file
    if (m_part == -1) {
        return m_fileHandle.header(0).hasName();
    }

    return m_fileHandle.header(m_part).hasName();
}


std::string LayerItem::getPartName() const
{
    // Single part file
    if (m_part == -1) {
        return m_fileHandle.header(0).name();
    }

    return m_fileHandle.header(m_part).name();
}


const QImage &LayerItem::getPreview() const
{
    return m_preview;
}


LayerItem::LayerType LayerItem::constructType() {
    if (m_leafName == "R") {
        return R;
    } else if (m_leafName == "G") {
        return G;
    } else if (m_leafName == "B") {
        return B;
    } else if (m_leafName == "Y") {
        return Y;
    } else if (m_leafName == "A") {
        return A;
    } else if (m_leafName == "Y") {
        return Y;
    } else if (m_leafName == "RY") {
        return RY;
    } else if (m_leafName == "BY") {
        return BY;
    } else if (m_leafName == "RGB") {
        return RGB;
    } else if (m_leafName == "RGBA") {
        return RGBA;
    } else if (m_leafName == "YA") {
        return YA;
    } else if (m_leafName == "YC") {
        return YC;
    } else if (m_leafName == "YCA") {
        return YCA;
    }

    // None of the above names but still holds a framebuffer
    if (m_pChannel) {
        return GENERAL;
    }

    if (m_pParentItem == nullptr) {
        return PART;
    }

    // If part id is not set to -1, it is a part
    // also, root item is either a part or a file made of multiple parts
    if (m_part != -1 && (m_pParentItem == nullptr || m_pParentItem->m_pParentItem == nullptr)) {
        return PART;
    }

    return GROUP;
}


void LayerItem::createThumbnails(LayerItem *item)
{
    item->createThumbnail();

//#pragma omp parallel for
    for (LayerItem* it: m_childItems) {
        it->createThumbnails(it);
    }
}


void LayerItem::createThumbnail()
{
    memset(m_previewBuffer, 0, 4 * m_previewSize * m_previewSize * sizeof(uchar));

    int colorOffset = 0;
    switch (m_type) {
    case R: colorOffset = 0; break;
    case G: colorOffset = 1; break;
    case B: colorOffset = 2; break;
    default: break;
    }

    switch (m_type) {
    case R:
    case G:
    case B:
    {
        Imf::InputPart part(m_fileHandle, getPart());

        const Imath::Box2i datW  = part.header().dataWindow();
        const Imath::Box2i dispW = part.header().displayWindow();

        const int width        = datW.max.x - datW.min.x + 1;
        const int height       = datW.max.y - datW.min.y + 1;
        const int dispW_width  = dispW.max.x - dispW.min.x + 1;
        const int dispW_height = dispW.max.y - dispW.min.y + 1;

        const QRect dataWindow       = QRect(datW.min.x, datW.min.y, width, height);
        const QRect displayWindow    = QRect(dispW.min.x, dispW.min.y, dispW_width, dispW_height);
        const float pixelAspectRatio = part.header().pixelAspectRatio();

        float* pixelBuffer = new float[width * height];

        const Imf::Slice graySlice = Imf::Slice::Make(
                    Imf::PixelType::FLOAT,
                    pixelBuffer,
                    datW,
                    sizeof(float),
                    width * sizeof(float));

        Imf::FrameBuffer framebuffer;

        framebuffer.insert(m_channelName, graySlice);

        part.setFrameBuffer(framebuffer);
        part.readPixels(datW.min.y, datW.max.y);

        const float aspect = (float)width / (float)height;

        // m_previewSize sets the max size
        int previewHeight = m_previewSize;
        int previewWidth  = m_previewSize;

        printf("Aspect: %f\n", aspect);

        if (aspect > 1.f) {
            previewHeight /= aspect;
            printf("Horizontal, height: %d\n", previewHeight);

        } else {
            previewWidth *= aspect;
            printf("Vertical\n");
        }

        const int previewHeightOffset = (m_previewSize - previewHeight) / 2;

        for (int yOffset = previewHeightOffset; yOffset < previewHeight + previewHeightOffset; yOffset++) {
            const int y = yOffset - previewHeightOffset;
            const int y_orig = std::round((float)y / (float)(previewHeight - 1) * (float)(height - 1));

            if (y_orig >= 0 && y_orig < height) {
                for (int x = 0; x < previewWidth; x++) {
                    const int x_orig = std::round((float)x / (float)(previewWidth - 1) * (float)(width - 1));

                    if (x_orig >= 0 && x_orig < width) {
                        m_previewBuffer[4 * (yOffset * m_previewSize + x) + colorOffset] =
                                ColorTransform::to_sRGB_255(pixelBuffer[y_orig * width + x_orig]);
                        m_previewBuffer[4 * (yOffset * m_previewSize + x) + 3] = 255;
                    }
                }
            }
        }

        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        delete[] pixelBuffer;
    }
        break;

    case Y:
    {
        Imf::InputPart part(m_fileHandle, getPart());

        const Imath::Box2i datW  = part.header().dataWindow();
        const Imath::Box2i dispW = part.header().displayWindow();

        const int width        = datW.max.x - datW.min.x + 1;
        const int height       = datW.max.y - datW.min.y + 1;
        const int dispW_width  = dispW.max.x - dispW.min.x + 1;
        const int dispW_height = dispW.max.y - dispW.min.y + 1;

        const QRect dataWindow       = QRect(datW.min.x, datW.min.y, width, height);
        const QRect displayWindow    = QRect(dispW.min.x, dispW.min.y, dispW_width, dispW_height);
        const float pixelAspectRatio = part.header().pixelAspectRatio();

        float* pixelBuffer = new float[width * height];

        const Imf::Slice graySlice = Imf::Slice::Make(
                    Imf::PixelType::FLOAT,
                    pixelBuffer,
                    datW,
                    sizeof(float),
                    width * sizeof(float));

        Imf::FrameBuffer framebuffer;

        framebuffer.insert(m_channelName, graySlice);

        part.setFrameBuffer(framebuffer);
        part.readPixels(datW.min.y, datW.max.y);

        const float aspect = (float)width / (float)height;

        // m_previewSize sets the max size
        int previewHeight = m_previewSize;
        int previewWidth  = m_previewSize;

        printf("Aspect: %f\n", aspect);

        if (aspect > 1.f) {
            previewHeight /= aspect;
            printf("Horizontal, height: %d\n", previewHeight);

        } else {
            previewWidth *= aspect;
            printf("Vertical\n");
        }

        const int previewHeightOffset = (m_previewSize - previewHeight) / 2;

        for (int yOffset = previewHeightOffset; yOffset < previewHeight + previewHeightOffset; yOffset++) {
            const int y = yOffset - previewHeightOffset;
            const int y_orig = std::round((float)y / (float)(previewHeight - 1) * (float)(height - 1));

            if (y_orig >= 0 && y_orig < height) {
                for (int x = 0; x < previewWidth; x++) {
                    const int x_orig = std::round((float)x / (float)(previewWidth - 1) * (float)(width - 1));
                    const uchar pixelVal = ColorTransform::to_sRGB_255(pixelBuffer[y_orig * width + x_orig]);

                    if (x_orig >= 0 && x_orig < width) {
                        for (int c = 0; c < 3; c++) {
                            m_previewBuffer[4 * (yOffset * m_previewSize + x) + c] = pixelVal;
                        }

                        m_previewBuffer[4 * (yOffset * m_previewSize + x) + 3] = 255;
                    }
                }
            }
        }

        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        delete[] pixelBuffer;
    }
        break;
    case A:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "A");
     }
        break;
    case RY:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "RY");
    }
        break;
    case BY:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "BY");
    }
        break;
    case GENERAL:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "Fb");
    }
        break;
    case GROUP:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "Gr");
    }
        break;
    case PART:
    {
        m_preview = QImage(m_previewBuffer, m_previewSize, m_previewSize, QImage::Format_RGBA8888);

        QPainter painter(&m_preview);
        QFont font = painter.font();

        font.setPixelSize(48);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(0, 0, m_previewSize, m_previewSize), Qt::AlignCenter, "P");
    }
        break;
    }
}
