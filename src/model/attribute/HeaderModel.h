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

#include <model/attribute/HeaderItem.h>
#include <model/attribute/LayerItem.h>

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfMultiPartInputFile.h>

#include <OpenEXR/ImfBoxAttribute.h>
#include <OpenEXR/ImfChannelListAttribute.h>
#include <OpenEXR/ImfChromaticitiesAttribute.h>
#include <OpenEXR/ImfCompressionAttribute.h>
#include <OpenEXR/ImfDeepImageStateAttribute.h>
#include <OpenEXR/ImfDoubleAttribute.h>
#include <OpenEXR/ImfEnvmapAttribute.h>
#include <OpenEXR/ImfFloatAttribute.h>
#include <OpenEXR/ImfFloatVectorAttribute.h>
#include <OpenEXR/ImfIDManifestAttribute.h>
#include <OpenEXR/ImfIntAttribute.h>
#include <OpenEXR/ImfKeyCodeAttribute.h>
#include <OpenEXR/ImfLineOrderAttribute.h>
#include <OpenEXR/ImfMatrixAttribute.h>
//#include <OpenEXR/ImfOpaqueAttribute.h>
#include <OpenEXR/ImfPreviewImageAttribute.h>
#include <OpenEXR/ImfRationalAttribute.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <OpenEXR/ImfStringVectorAttribute.h>
#include <OpenEXR/ImfTileDescriptionAttribute.h>
#include <OpenEXR/ImfTimeCodeAttribute.h>
#include <OpenEXR/ImfVecAttribute.h>

class HeaderModel: public QAbstractItemModel
{
  public:
    HeaderModel(Imf::MultiPartInputFile &file, int n_parts, QObject *parent);

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

    // General function. Delegates to one of the specialised
    // function
    HeaderItem *addItem(
      const char *          name,
      const Imf::Attribute &attr,
      HeaderItem *          parent,
      QString               partName,
      int                   part_number);

    // Box2i
    HeaderItem *addItem(
      const char *               name,
      const Imf::Box2iAttribute &attr,
      HeaderItem *               parent,
      QString                    partName,
      int                        part_number);

    // Box2f
    HeaderItem *addItem(
      const char *               name,
      const Imf::Box2fAttribute &attr,
      HeaderItem *               parent,
      QString                    partName,
      int                        part_number);

    // Channel List
    HeaderItem *addItem(
      const char *                     name,
      const Imf::ChannelListAttribute &attr,
      HeaderItem *                     parent,
      QString                          partName,
      int                              part_number);

    // Chromaticities
    HeaderItem *addItem(
      const char *                        name,
      const Imf::ChromaticitiesAttribute &attr,
      HeaderItem *                        parent,
      QString                             partName,
      int                                 part_number);

    // Compression
    HeaderItem *addItem(
      const char *                     name,
      const Imf::CompressionAttribute &attr,
      HeaderItem *                     parent,
      QString                          partName,
      int                              part_number);

    // Deep image state
    HeaderItem *addItem(
      const char *                        name,
      const Imf::DeepImageStateAttribute &attr,
      HeaderItem *                        parent,
      QString                             partName,
      int                                 part_number);

    // Double
    HeaderItem *addItem(
      const char *                name,
      const Imf::DoubleAttribute &attr,
      HeaderItem *                parent,
      QString                     partName,
      int                         part_number);

    // Envmap
    HeaderItem *addItem(
      const char *                name,
      const Imf::EnvmapAttribute &attr,
      HeaderItem *                parent,
      QString                     partName,
      int                         part_number);

    // Float
    HeaderItem *addItem(
      const char *               name,
      const Imf::FloatAttribute &attr,
      HeaderItem *               parent,
      QString                    partName,
      int                        part_number);

    // Float vector
    HeaderItem *addItem(
      const char *                     name,
      const Imf::FloatVectorAttribute &attr,
      HeaderItem *                     parent,
      QString                          partName,
      int                              part_number);

    // IDManifest
    HeaderItem *addItem(
      const char *                    name,
      const Imf::IDManifestAttribute &attr,
      HeaderItem *                    parent,
      QString                         partName,
      int                             part_number);

    // Int
    HeaderItem *addItem(
      const char *             name,
      const Imf::IntAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Key code
    HeaderItem *addItem(
      const char *                 name,
      const Imf::KeyCodeAttribute &attr,
      HeaderItem *                 parent,
      QString                      partName,
      int                          part_number);

    // Line order
    HeaderItem *addItem(
      const char *                   name,
      const Imf::LineOrderAttribute &attr,
      HeaderItem *                   parent,
      QString                        partName,
      int                            part_number);

    // Matrix33f
    HeaderItem *addItem(
      const char *              name,
      const Imf::M33fAttribute &attr,
      HeaderItem *              parent,
      QString                   partName,
      int                       part_number);

    // Matrix33d
    HeaderItem *addItem(
      const char *              name,
      const Imf::M33dAttribute &attr,
      HeaderItem *              parent,
      QString                   partName,
      int                       part_number);

    // Matrix44f
    HeaderItem *addItem(
      const char *              name,
      const Imf::M44fAttribute &attr,
      HeaderItem *              parent,
      QString                   partName,
      int                       part_number);

    // Matrix44d
    HeaderItem *addItem(
      const char *              name,
      const Imf::M44dAttribute &attr,
      HeaderItem *              parent,
      QString                   partName,
      int                       part_number);

    // Preview image
    HeaderItem *addItem(
      const char *                      name,
      const Imf::PreviewImageAttribute &attr,
      HeaderItem *                      parent,
      QString                           partName,
      int                               part_number);

    // Rational
    HeaderItem *addItem(
      const char *                  name,
      const Imf::RationalAttribute &attr,
      HeaderItem *                  parent,
      QString                       partName,
      int                           part_number);

    // String
    HeaderItem *addItem(
      const char *                name,
      const Imf::StringAttribute &attr,
      HeaderItem *                parent,
      QString                     partName,
      int                         part_number);

    // String vector
    HeaderItem *addItem(
      const char *                      name,
      const Imf::StringVectorAttribute &attr,
      HeaderItem *                      parent,
      QString                           partName,
      int                               part_number);


    // Tile description
    HeaderItem *addItem(
      const char *                         name,
      const Imf::TileDescriptionAttribute &attr,
      HeaderItem *                         parent,
      QString                              partName,
      int                                  part_number);

    // Timecode
    HeaderItem *addItem(
      const char *                  name,
      const Imf::TimeCodeAttribute &attr,
      HeaderItem *                  parent,
      QString                       partName,
      int                           part_number);

    // Vector2i
    HeaderItem *addItem(
      const char *             name,
      const Imf::V2iAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Vector2f
    HeaderItem *addItem(
      const char *             name,
      const Imf::V2fAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Vector2d
    HeaderItem *addItem(
      const char *             name,
      const Imf::V2dAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Vector3i
    HeaderItem *addItem(
      const char *             name,
      const Imf::V3iAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Vector3f
    HeaderItem *addItem(
      const char *             name,
      const Imf::V3fAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);

    // Vector3d
    HeaderItem *addItem(
      const char *             name,
      const Imf::V3dAttribute &attr,
      HeaderItem *             parent,
      QString                  partName,
      int                      part_number);



  private:
    HeaderItem *                          m_rootItem;
    std::vector<std::vector<std::string>> m_headerItems;

    std::vector<LayerItem *> m_partRootLayer;

    Imf::MultiPartInputFile &m_fileHandle;
};
