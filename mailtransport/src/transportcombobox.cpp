/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "transportcombobox.h"
#include "transport.h"
#include "transportmanager.h"

using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class TransportComboBoxPrivate
{
public:
    QList<int> transports;
};

TransportComboBox::TransportComboBox(QWidget *parent)
    : KComboBox(parent), d(new TransportComboBoxPrivate)
{
    QMetaObject::invokeMethod(this, "updateComboboxList");
    connect(TransportManager::self(), SIGNAL(transportsChanged()),
            SLOT(updateComboboxList()));
}

TransportComboBox::~TransportComboBox()
{
    delete d;
}

int TransportComboBox::currentTransportId() const
{
    if (currentIndex() >= 0 && currentIndex() < d->transports.count()) {
        return d->transports.at(currentIndex());
    }
    return -1;
}

void TransportComboBox::setCurrentTransport(int transportId)
{
    const int i = d->transports.indexOf(transportId);
    if (i >= 0 && i < count()) {
        setCurrentIndex(i);
    }
}

TransportBase::EnumType::type TransportComboBox::transportType() const
{
    int transtype = TransportManager::self()->transportById(currentTransportId())->type();
    return static_cast<TransportBase::EnumType::type>(transtype);
}

void TransportComboBox::updateComboboxList()
{
    const int oldTransport = currentTransportId();
    clear();

    int defaultId = 0;
    if (!TransportManager::self()->isEmpty()) {
        const QStringList listNames = TransportManager::self()->transportNames();
        const QList<int> listIds = TransportManager::self()->transportIds();
        addItems(listNames);
        setTransportList(listIds);
        defaultId = TransportManager::self()->defaultTransportId();
    }

    if (oldTransport != -1) {
        setCurrentTransport(oldTransport);
    } else {
        setCurrentTransport(defaultId);
    }
}

void TransportComboBox::setTransportList(const QList<int> &transportList)
{
    d->transports = transportList;
}
