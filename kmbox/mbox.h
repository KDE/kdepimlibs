/*
    Copyright (c) 2009 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef KMBOX_MBOX_H
#define KMBOX_MBOX_H

#include "kmbox_export.h"
#include "mboxentry.h"

#include <kmime/kmime_message.h>

namespace KMBox {

class MBoxPrivate;

/**
 * @short A class to access mail storages in MBox format.
 *
 * @author Bertjan Broeksema <broeksema@kde.org>
 * @since 4.6
 */
class KMBOX_EXPORT MBox
{
  public:
    /**
     * Describes the type of locking that will be used.
     */
    enum LockType {
      ProcmailLockfile,
      MuttDotlock,
      MuttDotlockPrivileged,
      None
    };

    /**
     * Creates a new mbox object.
     */
    MBox();

    /**
     * Destroys the mbox object.
     *
     * The file will be unlocked if it is still open.
     */
    ~MBox();

    /**
     * Appends @p message to the MBox and returns the corresponding mbox entry for it.
     * You must load a mbox file by making a call to load( const QString& ) before
     * appending entries.
     * The returned mbox entry is <em>only</em> valid for that particular file.
     *
     * @param message The message to append to the mbox.
     * @return the corresponding mbox entry for the message in the file or an invalid mbox entry
     *         if the message was not added.
     */
    MBoxEntry appendMessage( const KMime::Message::Ptr &message );

    /**
     * Retrieve the mbox entry objects for all emails from the file except the
     * @p deleteEntries.
     * The @p deletedEntries should be a list of mbox entries with offsets of deleted messages.
     * @param deletedEntries list of mbox entries that have been deleted and need not be retrieved
     * Note: One <em>must</em> call load() before calling this method.
     */
    MBoxEntry::List entries( const MBoxEntry::List &deletedEntries = MBoxEntry::List() ) const;

    /**
     * Returns the file name that was passed to the last call to load().
     */
    QString fileName() const;

    /**
     * Loads the raw mbox data from disk into the current MBox object. Messages
     * already present are <em>not</em> preserved. This method does not load the
     * full messages into memory but only the offsets of the messages and their
     * sizes. If the file currently is locked this method will do nothing and
     * return false. Appended messages that are not written yet will get lost.
     *
     * @param fileName the name of the mbox on disk.
     * @return true, if successful, false on error.
     *
     * @see save( const QString & )
     */
    bool load( const QString &fileName );

    /**
     * Locks the mbox file using the configured lock method. This can be used
     * for consecutive calls to readMessage and readMessageHeaders. Calling lock()
     * before these calls prevents the mbox file being locked for every call.
     *
     * NOTE: Even when the lock method is None the mbox is internally marked as
     *       locked. This means that it must be unlocked before calling load().
     *
     * @return true if locked successful, false on error.
     *
     * @see setLockType( LockType ), unlock()
     */
    bool lock();

    /**
     * Returns whether or not the mbox currently is locked.
     */
    bool locked() const;

    /**
     * Removes all messages for the given mbox entries from the current reference file
     * (the file that is loaded with load( const QString & ).
     * This method will first check if all lines at the offsets are actually
     * separator lines if this is not then no message will be deleted to prevent
     * corruption.
     *
     * @param deletedEntries The mbox entries of the messages that should be removed from
     *                       the file.
     * @param movedEntries Optional list for storing pairs of mbox entries that got moved
     *                     within the file due to the deletions.
     *                     The @c first member of the pair is the entry with the original offsets
     *                     the @c second member is the entry with the new (current) offset
     *
     * @return true if all offsets refer to a mbox separator line and a file was
     *         loaded, false otherwise. If the latter, the physical file has
     *         not changed.
     */
    bool purge( const MBoxEntry::List &deletedEntries, QList<MBoxEntry::Pair> *movedEntries = 0 );

    /**
     * Reads the entire message from the file for the given mbox @p entry. If the
     * mbox file is not locked this method will lock the file before reading and
     * unlock it after reading. If the file already is locked, it will not
     * unlock the file after reading the entry.
     *
     * @param entry The entry in the mbox file.
     * @return Message for the given entry or 0 if the file could not be locked
     *         or the entry offset > fileSize.
     *
     * @see lock(), unlock()
     */
    KMime::Message *readMessage( const MBoxEntry &entry );

    /**
     * Reads the headers of the message for the given mbox @p entry. If the
     * mbox file is not locked this method will lock the file before reading and
     * unlock it after reading. If the file already is locked, it will not
     * unlock the file after reading the entry.
     *
     * @param entry The entry in the mbox file.
     * @return QByteArray containing the raw message header data.
     *
     * @see lock(), unlock()
     */
    QByteArray readMessageHeaders( const MBoxEntry &entry );

    /**
     * Reads the entire message from the file for the given mbox @p entry. If the
     * mbox file is not locked this method will lock the file before reading and
     * unlock it after reading. If the file already is locked, it will not
     * unlock the file after reading the entry.
     *
     * @param entry The entry in the mbox file.
     * @return QByteArray containing the raw message data.
     *
     * @see lock(), unlock()
     */
    QByteArray readRawMessage( const MBoxEntry &entry );

    /**
     * Writes the mbox to disk. If the fileName is empty only appended messages
     * will be written to the file that was passed to load( const QString & ).
     * Otherwise the contents of the file that was loaded with load is copied to
     * @p fileName first.
     *
     * @param fileName the name of the file
     * @return true if the save was successful; false otherwise.
     *
     * @see load( const QString & )
     */
    bool save( const QString &fileName = QString() );

    /**
     * Sets the locktype that should be used for locking the mbox file. If the
     * new LockType cannot be used (e.g. the lockfile executable could not be
     * found) the LockType will not be changed.
     * @param ltype the locktype to set
     * This method will not do anything if the mbox object is currently locked
     * to make sure that it doesn't leave a locked file for one of the lockfile
     * / mutt_dotlock methods.
     */
    bool setLockType( LockType ltype );

    /**
     * Sets the lockfile that should be used by the procmail or the KDE lock
     * file method. If this method is not called and one of the before mentioned
     * lock methods is used the name of the lock file will be equal to
     * MBOXFILENAME.lock.
     * @param lockFile the lockfile to set
     */
    void setLockFile( const QString &lockFile );

    /**
     * By default the unlock method will directly unlock the file. However this
     * is expensive in case of many consecutive calls to readEntry. Setting the
     * time out to a non zero value will keep the lock open until the timeout has
     * passed. On each read the timer will be reset.
     * @param msec the time out to set for file lock
     */
    void setUnlockTimeout( int msec );

    /**
     * Unlock the mbox file.
     *
     * @return true if the unlock was successful, false otherwise.
     *
     * @see lock()
     */
    bool unlock();

    /**
     * Set the access mode of the mbox file to read only.
     *
     * If this is set to true, the mbox file can only be read from disk.
     * When the mbox file given in load() can not be opened in readWrite mode,
     * but can be opened in readOnly mode, this flag is automatically set to true.
     * You can still append messages, which are stored in memory
     * until save() is called, but the mbox can not be saved/purged to itself.
     * However it is possible to save it to a different file.
     * @param ro the readOnly flag to use
     *
     * @see save( const QString & )
     *
     * @since 4.14.5
     */
    void setReadOnly(bool ro = true);

    /**
     * Returns if the current access mode is set to readOnly.
     *
     * The access mode can either be set explicitely with setReadOnly() or
     * implicitely by calling load() on a readOnly file.
     *
     * @since 4.14.5
     */
    bool isReadOnly() const;

  private:
    //@cond PRIVATE
    friend class MBoxPrivate;
    MBoxPrivate * const d;
    //@endcond
};

}

#endif // KMBOX_MBOX_H
