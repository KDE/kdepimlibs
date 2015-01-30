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

#include "kioslavesession.h"

using namespace KioSMTP;

KioSMTP::KioSlaveSession::KioSlaveSession(SMTPProtocol *protocol)
    : m_protocol(protocol)
{
}

void KioSMTP::KioSlaveSession::error(int id, const QString &msg)
{
    m_protocol->error(id, msg);
}

void KioSlaveSession::informationMessageBox(const QString &msg, const QString &caption)
{
    m_protocol->messageBox(KIO::SlaveBase::Information, msg, caption);
}

bool KioSMTP::KioSlaveSession::openPasswordDialog(KIO::AuthInfo &authInfo)
{
    return m_protocol->openPasswordDialog(authInfo);
}

void KioSMTP::KioSlaveSession::dataReq()
{
    m_protocol->dataReq();
}

int KioSMTP::KioSlaveSession::readData(QByteArray &ba)
{
    return m_protocol->readData(ba);
}

bool KioSMTP::KioSlaveSession::startSsl()
{
    return m_protocol->startSsl();
}

bool KioSlaveSession::eightBitMimeRequested() const
{
    return m_protocol->metaData(QLatin1String("8bitmime")) == QLatin1String("on");
}

bool KioSlaveSession::lf2crlfAndDotStuffingRequested() const
{
    return m_protocol->metaData(QLatin1String("lf2crlf+dotstuff")) == QLatin1String("slave");
}

bool KioSlaveSession::pipeliningRequested() const
{
    return m_protocol->metaData(QLatin1String("pipelining")) != QLatin1String("off");
}

QString KioSlaveSession::requestedSaslMethod() const
{
    return m_protocol->metaData(QLatin1String("sasl"));
}

KioSMTP::SMTPSessionInterface::TLSRequestState KioSMTP::KioSlaveSession::tlsRequested() const
{
    if (m_protocol->metaData(QLatin1String("tls")) == QLatin1String("off")) {
        return ForceNoTLS;
    }
    if (m_protocol->metaData(QLatin1String("tls")) == QLatin1String("on")) {
        return ForceTLS;
    }
    return UseTLSIfAvailable;
}
