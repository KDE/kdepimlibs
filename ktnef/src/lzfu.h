/*
    lzfu.h

    Copyright (C) 2003 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */
/**
 * @file
 * This file is part of the API for handling TNEF data and
 * provides the @acronym LZFU decompression functionality.
 *
 * @author Michael Goffioul
 */

#ifndef LZFU_H
#define LZFU_H

class QIODevice;
namespace KTnef
{
/**
 * @acronym LZFU decompress data in compressed Rich Text Format (@acronym RTF).
 * @param input compressed input data.
 * @param output decompressed output data.
 */
int lzfu_decompress(QIODevice *input, QIODevice *output);
}
#endif
