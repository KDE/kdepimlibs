/*  This file is part of the KDE project
    Copyright (C) 2009 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 3 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "smartcardreader.h"
#include "smartcardreader_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/smartcardreader.h>

Solid::SmartCardReader::SmartCardReader(QObject *backendObject)
    : DeviceInterface(*new SmartCardReaderPrivate(), backendObject)
{
}

Solid::SmartCardReader::~SmartCardReader()
{

}

Solid::SmartCardReader::ReaderType Solid::SmartCardReader::readerType() const
{
    Q_D(const SmartCardReader);
    return_SOLID_CALL(Ifaces::SmartCardReader *, d->backendObject(), CardReader, readerType());
}

#include "smartcardreader.moc"

