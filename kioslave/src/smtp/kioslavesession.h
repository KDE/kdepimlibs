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

namespace KioSMTP {

class KioSlaveSession : public SMTPSessionInterface
{
public:
    explicit KioSlaveSession(SMTPProtocol *protocol);
    void error(int id, const QString &msg) Q_DECL_OVERRIDE;
    void informationMessageBox(const QString &msg, const QString &caption) Q_DECL_OVERRIDE;
    bool openPasswordDialog(KIO::AuthInfo &authInfo) Q_DECL_OVERRIDE;
    void dataReq() Q_DECL_OVERRIDE;
    int readData(QByteArray &ba) Q_DECL_OVERRIDE;
    bool startSsl() Q_DECL_OVERRIDE;

    QString requestedSaslMethod() const Q_DECL_OVERRIDE;
    bool eightBitMimeRequested() const Q_DECL_OVERRIDE;
    bool lf2crlfAndDotStuffingRequested() const Q_DECL_OVERRIDE;
    bool pipeliningRequested() const Q_DECL_OVERRIDE;
    TLSRequestState tlsRequested() const Q_DECL_OVERRIDE;

private:
    SMTPProtocol *m_protocol;
};

}

#endif
