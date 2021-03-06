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

#include "ColormapModule.h"

#include <exception>

#include "BBGRColormap.h"
#include "TabulatedColormap.h"
#include "YColormap.h"

#include <cstring>

Colormap* ColormapModule::create(const std::string& name)
{
    const char* colormap_name = name.c_str();

    if (strcmp(colormap_name, "bbgr") == 0) {
        return new BBGRColormap();
    } else if (
      strcmp(colormap_name, "turbo") == 0 || strcmp(colormap_name, "magma") == 0
      || strcmp(colormap_name, "inferno") == 0
      || strcmp(colormap_name, "plasma") == 0
      || strcmp(colormap_name, "viridis") == 0) {
        return new TabulatedColormap(colormap_name);
    } else if (strcmp(colormap_name, "grayscale") == 0) {
        return new YColormap();
    } else {
        throw std::exception();
    }
}

Colormap* ColormapModule::create(Map map)
{
    switch (map) {
        case GRAYSCALE:
            return new YColormap();
        case BBGR:
            return new BBGRColormap();
        case TURBO:
            return new TabulatedColormap(TabulatedColormap::TAB_TUBRO);
        case MAGMA:
            return new TabulatedColormap(TabulatedColormap::TAB_MAGMA);
        case INFERNO:
            return new TabulatedColormap(TabulatedColormap::TAB_INFERNO);
        case PLASMA:
            return new TabulatedColormap(TabulatedColormap::TAB_PLASMA);
        case VIRIDIS:
            return new TabulatedColormap(TabulatedColormap::TAB_VIRIDIS);
        case N_MAPS:
            throw std::exception();
    }

    return nullptr;
}

std::string ColormapModule::toString(Map map)
{
    switch (map) {
        case GRAYSCALE:
            return "Grayscale";
        case BBGR:
            return "BBGR";
        case TURBO:
            return "Turbo";
        case MAGMA:
            return "Magma";
        case INFERNO:
            return "Inferno";
        case PLASMA:
            return "Plasma";
        case VIRIDIS:
            return "Viridis";
        case N_MAPS:
            throw std::exception();
    }

    return "";
}
