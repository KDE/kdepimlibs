/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef KIOSMTP_KIOSLAVESESSION_H
#define KIOSMTP_KIOSLAVESESSION_H

#include "smtpsessioninterface.h"
#include "smtp.h"

namespace KioSMTP
{

class KioSlaveSession : public SMTPSessionInterface
{
public:
    explicit KioSlaveSession(SMTPProtocol *protocol);
    virtual void error(int id, const QString &msg);
    virtual void informationMessageBox(const QString &msg, const QString &caption);
    virtual bool openPasswordDialog(KIO::AuthInfo &authInfo);
    virtual void dataReq();
    virtual int readData(QByteArray &ba);
    virtual bool startSsl();

    virtual QString requestedSaslMethod() const;
    virtual bool eightBitMimeRequested() const;
    virtual bool lf2crlfAndDotStuffingRequested() const;
    virtual bool pipeliningRequested() const;
    virtual TLSRequestState tlsRequested() const;

private:
    SMTPProtocol *m_protocol;
};

}

#endif
