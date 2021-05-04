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
#include "BBGRColormap.h"

#include <vector>
#include <cstddef>

BBGRColormap::BBGRColormap() {}

BBGRColormap::~BBGRColormap() {}

void BBGRColormap::getRGBValue(float v, float RGB[]) const
{
    v = clamp(v);

    std::vector<float[3]> scale(6);
    scale[0][0] = 0;
    scale[0][1] = 0;
    scale[0][2] = 0;
    scale[1][0] = 0;
    scale[1][1] = 0;
    scale[1][2] = 1;
    scale[2][0] = 0;
    scale[2][1] = 1;
    scale[2][2] = 1;
    scale[3][0] = 0;
    scale[3][1] = 1;
    scale[3][2] = 0;
    scale[4][0] = 1;
    scale[4][1] = 1;
    scale[4][2] = 0;
    scale[5][0] = 1;
    scale[5][1] = 0;
    scale[5][2] = 0;

    std::vector<float> values(scale.size());

    for (size_t i = 0; i < scale.size(); i++) {
        values[i] = float(i) / float(scale.size() - 1);
    }

    for (size_t i = 1; i < scale.size(); i++) {
        if (v <= values[i]) {
            float interp = place(v, values[i - 1], values[i]);

            for (size_t c = 0; c < 3; c++) {
                RGB[c] = interp * scale[i][c]
                        + (1.f - interp) * scale[i - 1][c];
            }

            return;
        }
    }

    for (size_t c = 0; c < 3; c++) {
        RGB[c] = scale[scale.size() - 1][c];
    }
}

//void BBGRColormap::getRGBValue(float v, float v_min, float v_max, float RGB[]) const
//{
//    getRGBValue((v - v_min) / (v_max - v_min), RGB);
//}

float BBGRColormap::clamp(float v, float v_min, float v_max)
{
    return std::min(std::max(v, v_min), v_max);
}

float BBGRColormap::place(float v, float v_min, float v_max)
{
    v = clamp(v, v_min, v_max);
    return (v - v_min) / (v_max - v_min);
}
