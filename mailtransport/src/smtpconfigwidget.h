/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on MailTransport code by:
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>

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

#ifndef MAILTRANSPORT_SMTPCONFIGWIDGET_H
#define MAILTRANSPORT_SMTPCONFIGWIDGET_H

#include "transportconfigwidget.h"

namespace MailTransport
{

class Transport;

/**
  @internal
*/
class SMTPConfigWidgetPrivate;

/**
  @internal
  Configuration widget for a SMTP transport.
*/
class SMTPConfigWidget : public TransportConfigWidget
{
    Q_OBJECT

public:
    explicit SMTPConfigWidget(Transport *transport, QWidget *parent = Q_NULLPTR);
    //virtual ~SMTPConfigWidget();

public Q_SLOTS:
    /** reimpl */
    virtual void apply();

private Q_SLOTS:
    void checkSmtpCapabilities();
    void passwordsLoaded();
    void slotFinished(const QList<int> &results);
    void hostNameChanged(const QString &text);
    void encryptionChanged(int enc);
    void ensureValidAuthSelection();

private:
    Q_DECLARE_PRIVATE(SMTPConfigWidget)

    void init();

};

} // namespace MailTransport

#endif // MAILTRANSPORT_SMTPCONFIGWIDGET_H
