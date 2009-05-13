/*
    This file is part of the kcal library.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
  This file is part of the API for handling calendar data and
  defines the ResourceLocal class.

  @author Preston Brown <pbrown@kde.org>
  @author Cornelius Schumacher <schumacher@kde.org>
*/

#ifndef KCAL_RESOURCELOCAL_H
#define KCAL_RESOURCELOCAL_H

#include <QtCore/QString>

#include <kdatetime.h>

#include "kcal_export.h"
#include "resourcecached.h"

namespace KCal {

/**
  @brief Provides a calendar resource stored as a local file.
*/
class KCAL_EXPORT ResourceLocal : public ResourceCached
{
    Q_OBJECT

    friend class ResourceLocalConfig;

  public:

    /**
      Constructs a resource using default  configuration information.
    */
    ResourceLocal();

    /**
      Constructs a resource from configuration information
      stored in a KConfig object.

      @param group the configuration group to read the resource configuration from
    */
    explicit ResourceLocal( const KConfigGroup &group );

    /**
      Constructs a resource for file named @p fileName.

      @param fileName the file to link to the resource.
    */
    explicit ResourceLocal( const QString &fileName );

    /**
      Destroys the resource.
    **/
    virtual ~ResourceLocal();

    /**
      Writes KConfig @p config to a local file.
    **/
    virtual void writeConfig( KConfigGroup &group );

    /**
      Returns the lock.
    **/
    KABC::Lock *lock();

    /**
      Returns the fileName for this resource.

      @see setFileName()
    **/
    QString fileName() const;

    /**
      Sets the fileName for this resource. This will be the local
      file where the resource data will be stored.

      @param fileName the file to use for this resource

      @see fileName()
    **/
    bool setFileName( const QString &fileName );

    /**
      Sets a value for this resource.

      @param key the distinct name for this value.
      @param value the actual data for this value.
    **/
    bool setValue( const QString &key, const QString &value );

    /**
      Dumps the resource.
    **/
    void dump() const;

  protected Q_SLOTS:

    /**
      Reload the resource data from the local file.
    **/
    void reload();

  protected:

    /**
      Actually loads the data from the local file.
    **/
    virtual bool doLoad( bool syncCache );

    /**
      Actually saves the data to the local file.
    **/
    virtual bool doSave( bool syncCache );

    /**
      @copydoc
      ResourceCached::doSave(bool, Incidence*)
    */
    virtual bool doSave( bool syncCache, Incidence *incidence );

    /**
      Called by reload() to reload the resource, if it is already open.
      @return true if successful, else false. If true is returned,
              reload() will emit a resourceChanged() signal.

      @see doLoad(), doSave()
    */
    virtual bool doReload();

    /**
      Returns the date/time the local file was last modified.

      @see doSave()
    **/
    KDateTime readLastModified();

    /**
      Compares this ResourceLocal and @p other for equality.
      Returns true if they are equal.

      @param other the instance to compare with
    **/
    bool operator==( const ResourceLocal &other );

    /**
      Sets this ResourceLocal equal to @p other.
    **/
    ResourceLocal &operator=( const ResourceLocal &other );

  private:
    // Inherited virtual methods which should not be used by derived classes.
    using ResourceCalendar::doLoad;
    using ResourceCalendar::doSave;

    void init();
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
