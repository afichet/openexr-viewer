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
#include "OpenEXRLayerItem.h"

#include <cassert>

#include <QString>


OpenEXRLayerItem::OpenEXRLayerItem(OpenEXRLayerItem *parent)
    : m_parentItem(parent)
    , m_channelPtr(nullptr)
{}

OpenEXRLayerItem::~OpenEXRLayerItem()
{
    for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
        delete it.value();
    }
}


void OpenEXRLayerItem::addLeaf(const QString channelName, const Imf::Channel *leafChannel)
{
    OpenEXRLayerItem *leafNode = getAddLeaf(channelName);
    leafNode->m_channelPtr = leafChannel;
    leafNode->m_channelName = channelName;
}


OpenEXRHeaderItem *OpenEXRLayerItem::constructItemHierarchy(OpenEXRHeaderItem *parent, int partID) {
    if (m_childItems.size() == 0) {
        // This is a terminal leaf
        assert(m_channelPtr != nullptr);
        return new OpenEXRHeaderItem(parent, {m_rootName, "", "framebuffer"}, m_channelName, partID);
    }

    OpenEXRHeaderItem* currRoot = nullptr;

    // Avoid empty root on top level
    if (m_parentItem) {
        currRoot = new OpenEXRHeaderItem(parent, {m_rootName, (int)getNChilds(), "channellist"});
    } else {
        currRoot = parent;
    }

    if (m_channelPtr) {
        // It's a leaf...
        // Both are valid but I prefer the nested representation
        // OpenEXRItem* leafNode = new OpenEXRItem(parent, {m_rootName, "", "framebuffer"});
        new OpenEXRHeaderItem(currRoot, {".", "", "framebuffer"}, m_channelName, partID);
    }

    QStringList ignoredKeys;

    // If we find RGB final leaf, we make a virtual group
    if (hasRGBAChilds()) {
        OpenEXRHeaderItem* rgbaRoot = new OpenEXRHeaderItem(currRoot, {"RGBA", "", "RGB framebuffer"}, m_childItems["R"]->m_channelName.chopped(1), partID);
        
        new OpenEXRHeaderItem(rgbaRoot, {"R", "", "framebuffer"}, m_childItems["R"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbaRoot, {"G", "", "framebuffer"}, m_childItems["G"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbaRoot, {"B", "", "framebuffer"}, m_childItems["B"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbaRoot, {"A", "", "framebuffer"}, m_childItems["A"]->m_channelName, partID);

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
        ignoredKeys.append("A");
    } 
    else if (hasRGBChilds()) {
        OpenEXRHeaderItem* rgbRoot = new OpenEXRHeaderItem(currRoot, {"RGB", "", "RGB framebuffer"}, m_childItems["R"]->m_channelName.chopped(1), partID);
        
        new OpenEXRHeaderItem(rgbRoot, {"R", "", "framebuffer"}, m_childItems["R"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbRoot, {"G", "", "framebuffer"}, m_childItems["G"]->m_channelName, partID);
        new OpenEXRHeaderItem(rgbRoot, {"B", "", "framebuffer"}, m_childItems["B"]->m_channelName, partID);

        ignoredKeys.append("R");
        ignoredKeys.append("G");
        ignoredKeys.append("B");
    }

    // Same for Luminance Chroma images
    // Not an else, can be both
    // Note if there is both + Alpha channel, that is quite odd, alpha is then shared between to groups
    if (hasYCAChilds()) {
        OpenEXRHeaderItem* ycaRoot = new OpenEXRHeaderItem(currRoot, {"YCA", "", "YC framebuffer"}, m_childItems["Y"]->m_channelName.chopped(1), partID);
        
        new OpenEXRHeaderItem(ycaRoot, {"Y", "", "framebuffer"}, m_childItems["Y"]->m_channelName, partID);
        new OpenEXRHeaderItem(ycaRoot, {"RY", "", "framebuffer"}, m_childItems["RY"]->m_channelName, partID);
        new OpenEXRHeaderItem(ycaRoot, {"BY", "", "framebuffer"}, m_childItems["BY"]->m_channelName, partID);
        new OpenEXRHeaderItem(ycaRoot, {"A", "", "framebuffer"}, m_childItems["A"]->m_channelName, partID);

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
        ignoredKeys.append("A");
    }
    else if (hasYCChilds()) {
        OpenEXRHeaderItem* ycRoot = new OpenEXRHeaderItem(currRoot, {"YC", "", "YC framebuffer"}, m_childItems["Y"]->m_channelName.chopped(1), partID);
        
        new OpenEXRHeaderItem(ycRoot, {"Y", "", "framebuffer"}, m_childItems["Y"]->m_channelName, partID);
        new OpenEXRHeaderItem(ycRoot, {"RY", "", "framebuffer"}, m_childItems["RY"]->m_channelName, partID);
        new OpenEXRHeaderItem(ycRoot, {"BY", "", "framebuffer"}, m_childItems["BY"]->m_channelName, partID);

        ignoredKeys.append("Y");
        ignoredKeys.append("RY");
        ignoredKeys.append("BY");
    } else if (hasYAChilds()) {
        OpenEXRHeaderItem* yaRoot = new OpenEXRHeaderItem(currRoot, {"YA", "", "Y framebuffer"}, m_childItems["Y"]->m_channelName.chopped(1), partID);
        
        new OpenEXRHeaderItem(yaRoot, {"Y", "", "Luminance framebuffer"}, m_childItems["Y"]->m_channelName, partID);
        new OpenEXRHeaderItem(yaRoot, {"A", "", "framebuffer"}, m_childItems["A"]->m_channelName, partID);
            
        ignoredKeys.append("Y");
        ignoredKeys.append("A");
    }
    else if (hasYChild()) {
        new OpenEXRHeaderItem(currRoot, {"Y", "", "Luminance framebuffer"}, m_childItems["Y"]->m_channelName.chopped(1), partID);
  
        ignoredKeys.append("Y");
    }

    for (auto it = m_childItems.begin(); it != m_childItems.end(); it++) {
        if (!ignoredKeys.contains(it.key()) || it.value()->getNChilds() != 0)
            it.value()->constructItemHierarchy(currRoot, partID);
    }

    return currRoot;
}


size_t OpenEXRLayerItem::getNChilds() const {
    return m_childItems.size();
}


bool OpenEXRLayerItem::hasRGBChilds() const {
    return m_childItems.contains("R") && m_childItems["R"]->m_channelPtr
            && m_childItems.contains("G") && m_childItems["G"]->m_channelPtr
            && m_childItems.contains("B") && m_childItems["B"]->m_channelPtr;
}


bool OpenEXRLayerItem::hasRGBAChilds() const {
    return hasRGBChilds() && hasAChild();
}


bool OpenEXRLayerItem::hasYCChilds() const
{
    return m_childItems.contains("Y") && m_childItems["Y"]->m_channelPtr
        && m_childItems.contains("RY") && m_childItems["RY"]->m_channelPtr
        && m_childItems.contains("BY") && m_childItems["BY"]->m_channelPtr;
}


bool OpenEXRLayerItem::hasYCAChilds() const
{
    return hasYCChilds() && hasAChild();
}


bool OpenEXRLayerItem::hasYChild() const {
    return m_childItems.contains("Y") && m_childItems["Y"]->m_channelPtr;
}


bool OpenEXRLayerItem::hasYAChilds() const {
    return hasYChild() && hasAChild();
}


bool OpenEXRLayerItem::hasAChild() const {
    return m_childItems.contains("A") && m_childItems["A"]->m_channelPtr;
}


QString OpenEXRLayerItem::getFullName() const {
    QString name = m_rootName;

    OpenEXRLayerItem* parent = m_parentItem;

    while(parent) {
        name += parent->m_rootName + "." + name;
        parent = parent->m_parentItem;
    }

    return name;
}


OpenEXRLayerItem *OpenEXRLayerItem::getAddLeaf(const QString channelName) {
    QStringList channelHierachy = channelName.split(".");

    OpenEXRLayerItem *leafPtr = this;

    for (auto& s : channelHierachy) {
        if (leafPtr->m_childItems.contains(s)) {
            leafPtr = leafPtr->m_childItems[s];
        } else {
            OpenEXRLayerItem *newPtr = new OpenEXRLayerItem(leafPtr);
            newPtr->m_rootName = s;
            leafPtr->m_childItems.insert(s, newPtr);
            leafPtr = newPtr;
        }
    }

    return leafPtr;
}
