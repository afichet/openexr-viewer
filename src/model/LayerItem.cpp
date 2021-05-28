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

#include <cassert>

#include <QString>

LayerItem::LayerItem(LayerItem *parent)
  : m_parentItem(parent)
  , m_channelPtr(nullptr)
{}

LayerItem::~LayerItem()
{
    for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
        delete it.value();
    }
}

void LayerItem::addLeaf(
  const QString channelName, const Imf::Channel *leafChannel)
{
    LayerItem *leafNode     = getAddLeaf(channelName);
    leafNode->m_channelPtr  = leafChannel;
    leafNode->m_channelName = channelName;
}

HeaderItem *LayerItem::constructItemHierarchy(
  HeaderItem *parent, const QString &partName, int partID)
{
    if (m_childItems.size() == 0) {
        // This is a terminal leaf
        assert(m_channelPtr != nullptr);
        return new HeaderItem(
          parent,
          {m_rootName, "", "framebuffer"},
          m_channelName,
          partID);
    }

    HeaderItem *currRoot = nullptr;

    // Avoid empty root on top level
    if (m_parentItem) {
        currRoot = new HeaderItem(
          parent,
          {m_rootName, (int)getNChilds(), "channellist"});
    } else {
        currRoot = parent;
    }

    if (m_channelPtr) {
        // It's a leaf...
        // Both are valid but I prefer the nested representation
        // OpenEXRItem* leafNode = new OpenEXRItem(parent, {m_rootName, "",
        // "framebuffer"});
        new HeaderItem(
          currRoot,
          {".", "", "framebuffer"},
          partName,
          partID,
          m_channelName);
    }

    QStringList ignoredKeys;

    // If we find RGB final leaf, we make a virtual group
    if (hasRGBAChilds()) {
        HeaderItem *rgbaRoot = new HeaderItem(
          currRoot,
          {"RGBA", "", "RGB framebuffer"},
          partName,
          partID,
          m_childItems["R"]->m_channelName.chopped(1));

        new HeaderItem(
          rgbaRoot,
          {"R", "", "framebuffer"},
          partName,
          partID,
          m_childItems["R"]->m_channelName);
        new HeaderItem(
          rgbaRoot,
          {"G", "", "framebuffer"},
          partName,
          partID,
          m_childItems["G"]->m_channelName);
        new HeaderItem(
          rgbaRoot,
          {"B", "", "framebuffer"},
          partName,
          partID,
          m_childItems["B"]->m_channelName);
        new HeaderItem(
          rgbaRoot,
          {"A", "", "framebuffer"},
          partName,
          partID,
          m_childItems["A"]->m_channelName);

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
        ignoredKeys.append("A");
    } else if (hasRGBChilds()) {
        HeaderItem *rgbRoot = new HeaderItem(
          currRoot,
          {"RGB", "", "RGB framebuffer"},
          partName,
          partID,
          m_childItems["R"]->m_channelName.chopped(1));

        new HeaderItem(
          rgbRoot,
          {"R", "", "framebuffer"},
          partName,
          partID,
          m_childItems["R"]->m_channelName);
        new HeaderItem(
          rgbRoot,
          {"G", "", "framebuffer"},
          partName,
          partID,
          m_childItems["G"]->m_channelName);
        new HeaderItem(
          rgbRoot,
          {"B", "", "framebuffer"},
          partName,
          partID,
          m_childItems["B"]->m_channelName);

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
    }

    // Same for Luminance Chroma images
    // Not an else, can be both
    // Note if there is both + Alpha channel, that is quite odd, alpha is then
    // shared between to groups
    if (hasYCAChilds()) {
        HeaderItem *ycaRoot = new HeaderItem(
          currRoot,
          {"YCA", "", "YC framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName.chopped(1));

        new HeaderItem(
          ycaRoot,
          {"Y", "", "framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName);
        new HeaderItem(
          ycaRoot,
          {"RY", "", "framebuffer"},
          partName,
          partID,
          m_childItems["RY"]->m_channelName);
        new HeaderItem(
          ycaRoot,
          {"BY", "", "framebuffer"},
          partName,
          partID,
          m_childItems["BY"]->m_channelName);
        new HeaderItem(
          ycaRoot,
          {"A", "", "framebuffer"},
          partName,
          partID,
          m_childItems["A"]->m_channelName);

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
        ignoredKeys.append("A");
    } else if (hasYCChilds()) {
        HeaderItem *ycRoot = new HeaderItem(
          currRoot,
          {"YC", "", "YC framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName.chopped(1));

        new HeaderItem(
          ycRoot,
          {"Y", "", "framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName);
        new HeaderItem(
          ycRoot,
          {"RY", "", "framebuffer"},
          partName,
          partID,
          m_childItems["RY"]->m_channelName);
        new HeaderItem(
          ycRoot,
          {"BY", "", "framebuffer"},
          partName,
          partID,
          m_childItems["BY"]->m_channelName);

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
    } else if (hasYAChilds()) {
        HeaderItem *yaRoot = new HeaderItem(
          currRoot,
          {"YA", "", "Y framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName.chopped(1));

        new HeaderItem(
          yaRoot,
          {"Y", "", "Luminance framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName);
        new HeaderItem(
          yaRoot,
          {"A", "", "framebuffer"},
          partName,
          partID,
          m_childItems["A"]->m_channelName);

        ignoredKeys.append("Y");
        ignoredKeys.append("A");
    } else if (hasYChild()) {
        new HeaderItem(
          currRoot,
          {"Y", "", "Luminance framebuffer"},
          partName,
          partID,
          m_childItems["Y"]->m_channelName.chopped(1));

        ignoredKeys.append("Y");
    }

    for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
        if (!ignoredKeys.contains(it.key()) || it.value()->getNChilds() != 0)
            it.value()->constructItemHierarchy(currRoot, partName, partID);
    }

    return currRoot;
}

size_t LayerItem::getNChilds() const
{
    return m_childItems.size();
}

bool LayerItem::hasRGBChilds() const
{
    return m_childItems.contains("R") && m_childItems["R"]->m_channelPtr
           && m_childItems.contains("G") && m_childItems["G"]->m_channelPtr
           && m_childItems.contains("B") && m_childItems["B"]->m_channelPtr;
}

bool LayerItem::hasRGBAChilds() const
{
    return hasRGBChilds() && hasAChild();
}

bool LayerItem::hasYCChilds() const
{
    return m_childItems.contains("Y") && m_childItems["Y"]->m_channelPtr
           && m_childItems.contains("RY") && m_childItems["RY"]->m_channelPtr
           && m_childItems.contains("BY") && m_childItems["BY"]->m_channelPtr;
}

bool LayerItem::hasYCAChilds() const
{
    return hasYCChilds() && hasAChild();
}

bool LayerItem::hasYChild() const
{
    return m_childItems.contains("Y") && m_childItems["Y"]->m_channelPtr;
}

bool LayerItem::hasYAChilds() const
{
    return hasYChild() && hasAChild();
}

bool LayerItem::hasAChild() const
{
    return m_childItems.contains("A") && m_childItems["A"]->m_channelPtr;
}

QString LayerItem::getFullName() const
{
    QString name = m_rootName;

    LayerItem *parent = m_parentItem;

    while (parent) {
        name += parent->m_rootName + "." + name;
        parent = parent->m_parentItem;
    }

    return name;
}

LayerItem *LayerItem::getAddLeaf(const QString channelName)
{
    QStringList channelHierachy = channelName.split(".");

    LayerItem *leafPtr = this;

    for (auto &s : channelHierachy) {
        if (leafPtr->m_childItems.contains(s)) {
            leafPtr = leafPtr->m_childItems[s];
        } else {
            LayerItem *newPtr  = new LayerItem(leafPtr);
            newPtr->m_rootName = s;
            leafPtr->m_childItems.insert(s, newPtr);
            leafPtr = newPtr;
        }
    }

    return leafPtr;
}
