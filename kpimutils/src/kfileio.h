/*
  Copyright (c) 2005 Tom Albers <tomalbers@kde.nl>
  Copyright (c) 1997-1999 Stefan Taferner <taferner@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef KPIMUTILS_KFILEIO_H
#define KPIMUTILS_KFILEIO_H

#include "kpimutils_export.h"

class QByteArray;
class QString;
class QWidget;

namespace KPIMUtils
{

/**
  Loads the file with the given filename. Optionally, you can force the data
  to end with a newline character. Moreover, you can suppress warnings.

  @param fileName      Name of the file that should be loaded.
  @param ensureNewline If true, then the data will always have a trailing
                       newline. Defaults to true.
  @param withDialogs   If false, then no warning dialogs are shown in case of
                       problems. Defaults to true.

  @return The contents of the file or an empty QByteArray if loading failed.
*/
KPIMUTILS_EXPORT QByteArray kFileToByteArray(const QString &fileName,
        bool ensureNewline = true,
        bool withDialogs = true);

/**
  Writes the contents of @p buffer to the file with the given filename.

  @param buffer       The data you want to write to the file.
  @param fileName     The output file name
  @param askIfExists  If true, then you will be asked before an existing file
                      is overwritten. If false, then an existing file is
                      overwritten without warning.
  @param createBackup If true, then a backup of existing files will be
                      created. Otherwise, no backup will be made.
  @param withDialogs  If true, then you will be warned in case of problems.
                      Otherwise, no warnings will be issued.

  @return True if writing the data to the file succeeded.
  @return False if writing the data to the file failed.
*/
KPIMUTILS_EXPORT bool kByteArrayToFile(const QByteArray &buffer,
                                       const QString &fileName,
                                       bool askIfExists = false,
                                       bool createBackup = true,
                                       bool withDialogs = true);

/**
  Checks and corrects the permissions of a file or folder, and if requested
  all files and folders below. It gives back a list of files which do not
  have the right permissions. This list can be used to show to the user.

  @param toCheck         The file or folder of which the permissions should
                         be checked.
  @param recursive       Set to true, it will check the contents of a folder
                         for the permissions recursively. If false only
                         toCheck will be checked.
  @param wantItReadable  Set to true, it will check for read permissions.
                         If the read permissions are not available, there will
                         be a attempt to correct this.
  @param wantItWritable  Set to true, it will check for write permissions.
                         If the write permissions are not available, there
                         will be a attempt to correct this.
  @return It will return a string with all files and folders which do not
          have the right permissions. If empty, then all permissions are ok.
*/
KPIMUTILS_EXPORT QString checkAndCorrectPermissionsIfPossible(const QString &toCheck,
        const bool recursive,
        const bool wantItReadable,
        const bool wantItWritable);

/**
 * Removed a directory on the local filesystem whether it is empty or not. All
 * contents are irredeemably lost.
 *
 * @param path          An absolute or relative path to the directory to be
 *                      removed.
 *
 * @return Success or failure.
 */
KPIMUTILS_EXPORT bool removeDirAndContentsRecursively(const QString &path);

}

#endif
