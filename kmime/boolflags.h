/*
    Copyright (c) 1999-2001 the KMime authors.
    See file AUTHORS for details

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
  @file
  This file is part of the API for handling @ref MIME data and
  defines the BoolFlags class.

  @brief
  Defines the BoolFlags class.

  @author see AUTHORS file.
*/

#ifndef __KMIME_BOOLFLAGS_H__
#define __KMIME_BOOLFLAGS_H__

#include "kmime_export.h"

namespace KMime {

/**
  @brief
  Provides a class for storing boolean values in single bytes.

  This class provides functionality similar to QBitArray but requires
  much less memory.  Only 16-bits (or 2-bytes) can be stored.
*/
class KMIME_EXPORT BoolFlags {

  public:
    /**
      Constructs an empty 2-byte flag storage.
    */
    BoolFlags() { clear(); }

    /**
      Destroys the flag storage.
    */
    ~BoolFlags() {}

    /**
      Sets bit number @p i to the value @p b.

      @param i is the bit number. Valid values are 0 through 15.
      Higher values will be silently ignored.
      @param b is the value to set for bit @p i.
    */
    void set( unsigned int i, bool b=true );

    /**
      Get bit number @p i.

      @param i is the bit number. Valid values are 0 through 15.
      Higher values all return @c false.
      @return Value of the single bit @p i.
      Invalid bit numbers return @c false.
    */
    bool get( unsigned int i );

    /**
       Sets all bits to false.
    */
    void clear() { mBits[0]=0; mBits[1]=0; }

    /**
      Returns a pointer to the data structure used to store the bits.
    */
    unsigned char *data() { return mBits; }

  private:
    /**
      Two bytes (at least) of storage for the bits.
    */
    unsigned char mBits[2];  //space for 16 flags
};

}  //namespace KMime

#endif // __KMIME_BOOLFLAGS_H__
