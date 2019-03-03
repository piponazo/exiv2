// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2019 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

namespace Exiv2 {
    /// Supported Image Formats
    enum class ImageTypee{
        none,       // 1
        jpeg,       // 1
        exv,        // 2
        crw,        // 3

        tiff,       // 4
        dng,
        nef,
        pef,
        arw,
        sr2,
        srw,

        mrw,        // 5
        png,        // 6
        cr2,        // 7
        raf,        // 8
        orf,        // 9
        xmp,        // 10
        gif,        // 11
        psd,        // 12
        tga,        // 13
        bmp,        // 14
        jp2,        // 15
        rw2,        // 16
        pgf,        // 17
        webp,       // 23
        bigtiff,       // 25
    };
}

#endif // IMAGE_TYPES_H
