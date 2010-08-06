/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_PORTABLEMEDIAPLAYER_H
#define SOLID_PORTABLEMEDIAPLAYER_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class PortableMediaPlayerPrivate;
    class Device;

    /**
     * This class implements Portable Media Player device interface and represents
     * a portable media player attached to the system.
     * A portable media player is a portable device able to play multimedia files.
     * Some of them have even recording capabilities.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class SOLID_EXPORT PortableMediaPlayer : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(QStringList supportedProtocols READ supportedProtocols)
        Q_PROPERTY(QStringList supportedDrivers READ supportedDrivers)
        Q_DECLARE_PRIVATE(PortableMediaPlayer)
        friend class Device;

    public:

    private:
        /**
         * Creates a new PortableMediaPlayer object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit PortableMediaPlayer(QObject *backendObject);

    public:
        /**
         * Destroys a portable media player object.
         */
        virtual ~PortableMediaPlayer();

        /**
         * Get the Solid::DeviceInterface::Type of the PortableMediaPlayer device interface.
         *
         * @return the PortableMediaPlayer device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::PortableMediaPlayer; }

        /**
         * Retrieves known protocols this device can speak.  This list may be dependent
         * on installed device driver libraries.
         *
         * @return a list of known protocols this device can speak
         */
        QStringList supportedProtocols() const;

        /**
         * Retrieves known installed device drivers that claim to handle this device
         * using the requested protocol.  If protocol is blank, returns a list of
         * all drivers supporting the device.
         *
         * @param protocol The protocol to get drivers for.
         * @return a list of installed drivers meeting the criteria
         */
        QStringList supportedDrivers(QString protocol = QString()) const;

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for the "mtp" driver it will return the serial number
         * of the device.
         *
         * @return the driver specific data
         */
        QVariant driverHandle(const QString &driver) const;
    };
}

#endif
