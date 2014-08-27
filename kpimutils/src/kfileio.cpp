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

#include "kfileio.h"
#include "kpimutils_export.h"

#include <QDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <kde_file.h> //krazy:exclude=camelcase

#include <QDir>
#include <QByteArray>
#include <QWidget>
#include <QFile>
#include <QFileInfo>

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

namespace KPIMUtils
{

//-----------------------------------------------------------------------------
static void msgDialog(const QString &msg)
{
    KMessageBox::sorry(0, msg, i18n("File I/O Error"));
}

//-----------------------------------------------------------------------------
QByteArray kFileToByteArray(const QString &aFileName, bool aEnsureNL,
                            bool aVerbose)
{
    QByteArray result;
    QFileInfo info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile file(aFileName);

    //assert(aFileName!=0);
    if (aFileName.isEmpty()) {
        return "";
    }

    if (!info.exists()) {
        if (aVerbose) {
            msgDialog(i18n("The specified file does not exist:\n%1", aFileName));
        }
        return QByteArray();
    }
    if (info.isDir()) {
        if (aVerbose) {
            msgDialog(i18n("This is a folder and not a file:\n%1", aFileName));
        }
        return QByteArray();
    }
    if (!info.isReadable()) {
        if (aVerbose) {
            msgDialog(i18n("You do not have read permissions to the file:\n%1", aFileName));
        }
        return QByteArray();
    }
    if (len == 0) {
        return QByteArray();
    }

    if (!file.open(QIODevice::Unbuffered | QIODevice::ReadOnly)) {
        if (aVerbose) {
            switch (file.error()) {
            case QFile::ReadError:
                msgDialog(i18n("Could not read file:\n%1", aFileName));
                break;
            case QFile::OpenError:
                msgDialog(i18n("Could not open file:\n%1", aFileName));
                break;
            default:
                msgDialog(i18n("Error while reading file:\n%1", aFileName));
            }
        }
        return QByteArray();
    }

    result.resize(len + int(aEnsureNL));
    readLen = file.read(result.data(), len);
    if (aEnsureNL) {
        if (result[readLen - 1] != '\n') {
            result[readLen++] = '\n';
            len++;
        } else {
            result.truncate(len);
        }
    }

    if (readLen < len) {
        QString msg = i18np("Could only read 1 byte of %2.",
                            "Could only read %1 bytes of %2.",
                            readLen, len);
        msgDialog(msg);
        result.truncate(readLen);
    }

    return result;
}

//-----------------------------------------------------------------------------
bool kByteArrayToFile(const QByteArray &aBuffer, const QString &aFileName,
                      bool aAskIfExists, bool aBackup, bool aVerbose)
{
    // TODO: use KSaveFile
    QFile file(aFileName);

    //assert(aFileName!=0);
    if (aFileName.isEmpty()) {
        return false;
    }

    if (file.exists()) {
        if (aAskIfExists) {
            QString str;
            str = i18n("File %1 exists.\nDo you want to replace it?", aFileName);
            const int rc =
                KMessageBox::warningContinueCancel(0, str, i18n("Save to File"),
                                                   KGuiItem(i18n("&Replace")));
            if (rc != KMessageBox::Continue) {
                return false;
            }
        }
        if (aBackup) {
            // make a backup copy
            // TODO: use KSaveFile::backupFile()
            QString bakName = aFileName;
            bakName += QLatin1Char('~');
            QFile::remove(bakName);
            if (!QDir::current().rename(aFileName, bakName)) {
                // failed to rename file
                if (!aVerbose) {
                    return false;
                }
                const int rc =
                    KMessageBox::warningContinueCancel(
                        0,
                        i18n("Failed to make a backup copy of %1.\nContinue anyway?", aFileName),
                        i18n("Save to File"), KStandardGuiItem::save());

                if (rc != KMessageBox::Continue) {
                    return false;
                }
            }
        }
    }

    if (!file.open(QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (aVerbose) {
            switch (file.error()) {
            case QFile::WriteError:
                msgDialog(i18n("Could not write to file:\n%1", aFileName));
                break;
            case QFile::OpenError:
                msgDialog(i18n("Could not open file for writing:\n%1", aFileName));
                break;
            default:
                msgDialog(i18n("Error while writing file:\n%1", aFileName));
            }
        }
        return false;
    }

    const int writeLen = file.write(aBuffer.data(), aBuffer.size());

    if (writeLen < 0) {
        if (aVerbose) {
            msgDialog(i18n("Could not write to file:\n%1", aFileName));
        }
        return false;
    } else if (writeLen < aBuffer.size()) {
        QString msg = i18np("Could only write 1 byte of %2.",
                            "Could only write %1 bytes of %2.",
                            writeLen, aBuffer.size());
        if (aVerbose) {
            msgDialog(msg);
        }
        return false;
    }

    return true;
}

QString checkAndCorrectPermissionsIfPossible(const QString &toCheck,
        const bool recursive,
        const bool wantItReadable,
        const bool wantItWritable)
{
    // First we have to find out which type the toCheck is. This can be
    // a directory (follow if recursive) or a file (check permissions).
    // Symlinks are followed as expected.
    QFileInfo fiToCheck(toCheck);
    fiToCheck.setCaching(false);
    QByteArray toCheckEnc = QFile::encodeName(toCheck);
    QString error;
    KDE_struct_stat statbuffer;

    if (!fiToCheck.exists()) {
        error.append(i18n("%1 does not exist", toCheck) + QLatin1Char('\n'));
    }

    // check the access bit of a folder.
    if (fiToCheck.isDir()) {
        if (KDE_stat(toCheckEnc, &statbuffer) != 0) {
            qDebug() << "wantItA: Can't read perms of" << toCheck;
        }
        QDir g(toCheck);
        if (!g.isReadable()) {
            if (chmod(toCheckEnc, statbuffer.st_mode + S_IXUSR) != 0) {
                error.append(i18n("%1 is not accessible and that is "
                                  "unchangeable.", toCheck) + QLatin1Char('\n'));
            } else {
                qDebug() << "Changed access bit for" << toCheck;
            }
        }
    }

    // For each file or folder  we can check if the file is readable
    // and writable, as requested.
    if (fiToCheck.isFile() || fiToCheck.isDir()) {

        if (!fiToCheck.isReadable() && wantItReadable) {
            // Get the current permissions. No need to do anything with an
            // error, it will het added to errors anyhow, later on.
            if (KDE_stat(toCheckEnc, &statbuffer) != 0) {
                qDebug() << "wantItR: Can't read perms of" << toCheck;
            }

            // Lets try changing it.
            if (chmod(toCheckEnc, statbuffer.st_mode + S_IRUSR) != 0) {
                error.append(i18n("%1 is not readable and that is unchangeable.",
                                  toCheck) + QLatin1Char('\n'));
            } else {
                qDebug() << "Changed the read bit for" << toCheck;
            }
        }

        if (!fiToCheck.isWritable() && wantItWritable) {
            // Gets the current persmissions. Needed because it can be changed
            // curing previous operation.
            if (KDE_stat(toCheckEnc, &statbuffer) != 0) {
                qDebug() << "wantItW: Can't read perms of" << toCheck;
            }

            // Lets try changing it.
            if (chmod(toCheckEnc, statbuffer.st_mode + S_IWUSR) != 0) {
                error.append(i18n("%1 is not writable and that is unchangeable.", toCheck) + QLatin1Char('\n'));
            } else {
                qDebug() << "Changed the write bit for" << toCheck;
            }
        }
    }

    // If it is a folder and recursive is true, then we check the contents of
    // the folder.
    if (fiToCheck.isDir() && recursive) {
        QDir g(toCheck);
        // First check if the folder is readable for us. If not, we get
        // some ugly crashes.
        if (!g.isReadable()) {
            error.append(i18n("Folder %1 is inaccessible.", toCheck) + QLatin1Char('\n'));
        } else {
            foreach (const QFileInfo &fi, g.entryInfoList()) {
                QString newToCheck = toCheck + QLatin1Char('/') + fi.fileName();
                if (fi.fileName() != QLatin1String(".") && fi.fileName() != QLatin1String("..")) {
                    error.append(
                        checkAndCorrectPermissionsIfPossible(newToCheck, recursive,
                                wantItReadable, wantItWritable));
                }
            }
        }
    }
    return error;
}

}
