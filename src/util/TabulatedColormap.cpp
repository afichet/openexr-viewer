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

#include "TabulatedColormap.h"

#include <ColormapData.h>

#include <cassert>
#include <cstring>
#include <exception>
#include <cmath>

TabulatedColormap::TabulatedColormap(): _array(3), _n_elems(1) {}

TabulatedColormap::TabulatedColormap(TabMap map)
{
    switch (map) {
        case TAB_TUBRO:
            init(turbo_colormap_data, 256);
            break;
        case TAB_MAGMA:
            init(magma_colormap_data, 256);
            break;
        case TAB_INFERNO:
            init(inferno_colormap_data, 256);
            break;
        case TAB_PLASMA:
            init(plasma_colormap_data, 256);
            break;
        case TAB_VIRIDIS:
            init(viridis_colormap_data, 256);
            break;
        case N_TABMAPS:
            throw std::exception();
    }
}

TabulatedColormap::TabulatedColormap(const char* name)
{
    if (strcmp(name, "turbo") == 0) {
        init(turbo_colormap_data, 256);
    } else if (strcmp(name, "magma") == 0) {
        init(magma_colormap_data, 256);
    } else if (strcmp(name, "inferno") == 0) {
        init(inferno_colormap_data, 256);
    } else if (strcmp(name, "plasma") == 0) {
        init(plasma_colormap_data, 256);
    } else if (strcmp(name, "viridis") == 0) {
        init(viridis_colormap_data, 256);
    } else {
        throw std::exception();
    }
}

TabulatedColormap::~TabulatedColormap() {}

void TabulatedColormap::getRGBValue(float v, float RGB[]) const
{
    v = std::max(0.f, std::min(1.f, v));

#ifdef TAB_COLORMAP_CLOSEST
    assert(v >= 0.f);
    assert(v <= 1.f);

    const int closet_idx = v * (_n_elems - 1);

    assert(closet_idx < _n_elems);
    assert(closet_idx >= 0);

    memcpy(RGB, &_array[3 * closet_idx], 3 * sizeof(float));
#else
    const int   low  = int(std::floor(v * float(_n_elems - 1)));
    const int   high = std::min(_n_elems - 1, low + 1);
    const float a    = v * float(_n_elems - 1) - low;

    for (int c = 0; c < 3; c++) {
        RGB[c] = _array[3 * low + c]
                 + (_array[3 * high + c] - _array[3 * low + c]) * a;
    }
#endif
}

// void TabulatedColormap::getRGBValue(float v, float v_min, float v_max, float
// RGB[]) const
//{
//    getRGBValue((v - v_min) / (v_max - v_min), RGB);
//}

void TabulatedColormap::init(float* array, int n_elems)
{
    _array.resize(3 * n_elems);
    memcpy(_array.data(), array, 3 * n_elems * sizeof(float));
    _n_elems = n_elems;
}
