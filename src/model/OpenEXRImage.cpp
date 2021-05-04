#include "OpenEXRImage.h"

#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfChannelList.h>


#include <Imath/ImathBox.h>


// TODO: Remove
#include <iostream>

OpenEXRImage::OpenEXRImage(const QString& filename, QObject *parent)
    : m_exrIn(filename.toStdString().c_str())

{
    _headerModel = new HeaderModel(m_exrIn.parts(), parent);

    // Setup model data
    for (int i = 0; i < m_exrIn.parts(); i++) {
        const Imf::Header & exrHeader = m_exrIn.header(i);
        _headerModel->addHeader(exrHeader, i);
    }
}


OpenEXRImage::~OpenEXRImage() {
    delete _headerModel;
}


