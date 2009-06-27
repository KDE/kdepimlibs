/*
    Copyright (C) 2007 KovoKs <info@kovoks.nl>
    Copyright (c) 2008 Thomas McGuire <thomas.mcguire@gmx.net>

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

#ifndef MAILTRANSPORT_SERVERTEST_H
#define MAILTRANSPORT_SERVERTEST_H

#include <mailtransport/mailtransport_export.h>
#include <mailtransport/transport.h>

#include <QtGui/QWidget>
#include <QtCore/QHash>

class QProgressBar;

namespace MailTransport
{

class ServerTestPrivate;

/**
  * @class ServerTest
  * This class can be used to test certain server to see if they support stuff.
  * @author Tom Albers <tomalbers@kde.nl>
  */
class MAILTRANSPORT_EXPORT ServerTest : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString server READ server WRITE setServer )
    Q_PROPERTY( QString protocol READ protocol WRITE setProtocol )
    Q_PROPERTY( QProgressBar *progressBar READ progressBar WRITE setProgressBar )

  public:

    /**
     * This enumeration has the special capabilities a server might
     * support. This covers only capabilities not related to authentication.
     * @since 4.1
     */
    enum Capability {
      Pipelining, ///< POP3 only. The server supports pipeplining of commands
      Top,        ///< POP3 only. The server supports fetching only the headers
      UIDL        ///< POP3 only. The server has support for unique identifiers
    };

    /**
      * Constructor
      * @param parent Parent Widget
      */
    ServerTest( QWidget *parent = 0 );

    /**
      * Destructor
      */
    ~ServerTest();

    /**
      * Set the server to test.
      */
    void setServer( const QString &server );

    /**
      * Get the server to test.
      */
    QString server();

    /**
     * Set a custom port to use.
     *
     * Each encryption mode (no encryption or SSL) has a different port.
     * TLS uses the same port as no encryption, because TLS is invoked during
     * a normal session.
     *
     * If this function is never called, the default port is used, which is:
     * (normal first, then SSL)
     * SMTP: 25, 465
     * POP: 110, 995
     * IMAP: 143, 993
     *
     * @param encryptionMode the port will only be used in this encryption mode.
     *                       Valid values for this are only 'None' and 'SSL'.
     * @param port the port to use
     *
     * @since 4.1
     */
    void setPort( Transport::EnumEncryption::type encryptionMode, uint port );

    /**
     * @param encryptionMode the port of this encryption mode is returned.
     *                       Can only be 'None' and 'SSL'
     *
     * @return the port set by @ref setPort or -1 if @ref setPort() was never
     *         called for this encryption mode.
     *
     * @since 4.1
     */
    int port( Transport::EnumEncryption::type encryptionMode );

    /**
     * Sets a fake hostname for the test. This is currently only used when
     * testing a SMTP server; there, the command for testing the capabilities
     * (called EHLO) needs to have the hostname of the client included.
     * The user can however choose to send a fake hostname instead of the
     * local hostname to work around various problems. This fake hostname needs
     * to be set here.
     *
     * @param fakeHostname the fake hostname to send
     */
    void setFakeHostname( const QString &fakeHostname );

    /**
     * @return the fake hostname, as set before with @ref setFakeHostname
     */
    QString fakeHostname();

    /**
      * Makes @p pb the progressbar to use. This class will call show()
      * and hide() and will count down. It does not take ownership of
      * the progressbar.
      */
    void setProgressBar( QProgressBar *pb );

    /**
      * returns the used progressBar
      */
    QProgressBar *progressBar();

    /**
      * Set @p protocol the protocol to test, currently supported are
      * "smtp", "pop" and "imap".
      */
    void setProtocol( const QString &protocol );

    /**
      * returns the protocol
      */
    QString protocol();

    /**
      * Starts the test. Will emit finished() when done.
      */
    void start();

    /**
      * Get the protocols for the normal connections.. Call this only
      * after the finished() signals has been sent.
      * @return an enum of the type Transport::EnumAuthenticationType
      */
    QList< int > normalProtocols();

    /**
      * Get the protocols for the TLS connections. Call this only
      * after the finished() signals has been sent.
      * @return an enum of the type Transport::EnumAuthenticationType
      * @since 4.1
      */
    QList< int > tlsProtocols();

    /**
      * Get the protocols for the SSL connections. Call this only
      * after the finished() signals has been sent.
      * @return an enum of the type Transport::EnumAuthenticationType
      */
    QList< int > secureProtocols();

    /**
     * Get the special capabilities of the server.
     * Call this only after the finished() signals has been sent.
     *
     * @return the list of special capabilities of the server.
     * @since 4.1
     */
    QList< Capability > capabilities() const;

  Q_SIGNALS:
    /**
      * This will be emitted when the test is done. It will contain
      * the values from the enum EnumEncryption which are possible.
      */
    void finished( QList< int > );

  private:
    Q_DECLARE_PRIVATE( ServerTest )
    ServerTestPrivate *const d;

    Q_PRIVATE_SLOT( d, void slotNormalPossible() )
    Q_PRIVATE_SLOT( d, void slotTlsDone() )
    Q_PRIVATE_SLOT( d, void slotSslPossible() )
    Q_PRIVATE_SLOT( d, void slotReadNormal( const QString &text ) )
    Q_PRIVATE_SLOT( d, void slotReadSecure( const QString &text ) )
    Q_PRIVATE_SLOT( d, void slotNormalNotPossible() )
    Q_PRIVATE_SLOT( d, void slotSslNotPossible() )
    Q_PRIVATE_SLOT( d, void slotUpdateProgress() )
};

} // namespace MailTransport

#endif // MAILTRANSPORT_SERVERTEST_H
