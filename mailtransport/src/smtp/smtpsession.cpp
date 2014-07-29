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

#include "smtpsession.h"

#include "common.h"
#include "smtp/smtpsessioninterface.h"
#include "smtp/request.h"
#include "smtp/response.h"
#include "smtp/command.h"
#include "smtp/transactionstate.h"

#include <ktcpsocket.h>
#include <KMessageBox>
#include <KIO/PasswordDialog>
#include <kio/authinfo.h>
#include <kio/global.h>
#include <kio/sslui.h>
#include <KLocalizedString>
#include <KDebug>

#include <QtCore/QQueue>

using namespace MailTransport;
using namespace KioSMTP;

class MailTransport::SmtpSessionPrivate : public KioSMTP::SMTPSessionInterface
{
public:
    explicit SmtpSessionPrivate(SmtpSession *session) :
        useTLS(true),
        socket(0),
        currentCommand(0),
        currentTransactionState(0),
        state(Initial),
        q(session)
    {}

    void dataReq()
    {
        /* noop */
    };
    int readData(QByteArray &ba)
    {
        if (data->atEnd()) {
            ba.clear();
            return 0;
        } else {
            Q_ASSERT(data->isOpen());
            ba = data->read(32 * 1024);
            return ba.size();
        }
    }

    void error(int id, const QString &msg)
    {
        qDebug() << id << msg;
        // clear state so further replies don't end up in failed commands etc.
        currentCommand = 0;
        currentTransactionState = 0;

        if (errorMessage.isEmpty()) {
            errorMessage = KIO::buildErrorString(id, msg);
        }
        q->disconnectFromHost();
    }

    void informationMessageBox(const QString &msg, const QString &caption)
    {
        KMessageBox::information(0, msg, caption);
    }

    bool openPasswordDialog(KIO::AuthInfo &authInfo)
    {
        return KIO::PasswordDialog::getNameAndPassword(
                   authInfo.username,
                   authInfo.password,
                   &(authInfo.keepPassword),
                   authInfo.prompt,
                   authInfo.readOnly,
                   authInfo.caption,
                   authInfo.comment,
                   authInfo.commentLabel) == KIO::PasswordDialog::Accepted;
    }

    bool startSsl()
    {
        qDebug();
        Q_ASSERT(socket);
        socket->setAdvertisedSslVersion(KTcpSocket::TlsV1);
        socket->ignoreSslErrors();
        socket->startClientEncryption();
        const bool encrypted = socket->waitForEncrypted(60 * 1000);

        const KSslCipher cipher = socket->sessionCipher();
        if (!encrypted ||
                socket->sslErrors().count() > 0 ||
                socket->encryptionMode() != KTcpSocket::SslClientMode ||
                cipher.isNull() ||
                cipher.usedBits() == 0) {
            qDebug() << "Initial SSL handshake failed. cipher.isNull() is" << cipher.isNull()
                     << ", cipher.usedBits() is" << cipher.usedBits()
                     << ", the socket says:" <<  socket->errorString()
                     << "and the list of SSL errors contains"
                     << socket->sslErrors().count() << "items.";

            if (KIO::SslUi::askIgnoreSslErrors(socket)) {
                return true;
            } else {
                return false;
            }
        } else {
            qDebug() << "TLS negotiation done.";
            return true;
        }
    }

    bool lf2crlfAndDotStuffingRequested() const
    {
        return true;
    }
    QString requestedSaslMethod() const
    {
        return saslMethod;
    }
    TLSRequestState tlsRequested() const
    {
        return useTLS ? ForceTLS : ForceNoTLS;
    }

    void socketConnected()
    {
        qDebug();
        if (destination.protocol() == QLatin1String("smtps")) {
            if (!startSsl()) {
                error(KIO::ERR_SLAVE_DEFINED, i18n("SSL negotiation failed."));
            }
        }
    }

    void socketDisconnected()
    {
        qDebug();
        emit q->result(q);
        q->deleteLater();
    }

    void socketError(KTcpSocket::Error err)
    {
        qDebug() << err;
        error(KIO::ERR_CONNECTION_BROKEN, socket->errorString());

        if (socket->state() != KTcpSocket::ConnectedState) {
            // we have been disconnected by the error condition already, so just signal error result
            emit q->result(q);
            q->deleteLater();
        }
    }

    bool sendCommandLine(const QByteArray &cmdline)
    {
        if (cmdline.length() < 4096) {
            kDebug(7112) << "C: >>" << cmdline.trimmed().data() << "<<";
        } else {
            kDebug(7112) << "C: <" << cmdline.length() << " bytes>";
        }
        ssize_t numWritten, cmdline_len = cmdline.length();
        if ((numWritten = socket->write(cmdline)) != cmdline_len) {
            kDebug(7112) << "Tried to write " << cmdline_len << " bytes, but only "
                         << numWritten << " were written!" << endl;
            error(KIO::ERR_SLAVE_DEFINED, i18n("Writing to socket failed."));
            return false;
        }
        return true;
    }

    bool run(int type, TransactionState *ts = 0)
    {
        return run(Command::createSimpleCommand(type, this), ts);
    }

    bool run(Command *cmd, TransactionState *ts = 0)
    {
        Q_ASSERT(cmd);
        Q_ASSERT(!currentCommand);
        Q_ASSERT(!currentTransactionState || currentTransactionState == ts);

        // ### WTF?
        if (cmd->doNotExecute(ts)) {
            return true;
        }

        currentCommand = cmd;
        currentTransactionState = ts;

        while (!cmd->isComplete() && !cmd->needsResponse()) {
            const QByteArray cmdLine = cmd->nextCommandLine(ts);
            if (ts && ts->failedFatally()) {
                q->disconnectFromHost(false);
                return false;
            }
            if (cmdLine.isEmpty()) {
                continue;
            }
            if (!sendCommandLine(cmdLine)) {
                q->disconnectFromHost(false);
                return false;
            }
        }
        return true;
    }

    void queueCommand(int type)
    {
        queueCommand(Command::createSimpleCommand(type, this));
    }

    void queueCommand(KioSMTP::Command *command)
    {
        mPendingCommandQueue.enqueue(command);
    }

    bool runQueuedCommands(TransactionState *ts)
    {
        Q_ASSERT(ts);
        Q_ASSERT(!currentTransactionState || ts == currentTransactionState);
        currentTransactionState = ts;
        kDebug(canPipelineCommands(), 7112) << "using pipelining";

        while (!mPendingCommandQueue.isEmpty()) {
            QByteArray cmdline = collectPipelineCommands(ts);
            if (ts->failedFatally()) {
                q->disconnectFromHost(false);
                return false;
            }
            if (ts->failed()) {
                break;
            }
            if (cmdline.isEmpty()) {
                continue;
            }
            if (!sendCommandLine(cmdline) || ts->failedFatally()) {
                q->disconnectFromHost(false);
                return false;
            }
            if (!mSentCommandQueue.isEmpty()) {
                return true; // wait for responses
            }
        }

        if (ts->failed()) {
            qDebug() << "transaction state failed: " << ts->errorCode() << ts->errorMessage();
            if (errorMessage.isEmpty()) {
                errorMessage = ts->errorMessage();
            }
            state = SmtpSessionPrivate::Reset;
            if (!run(Command::RSET, currentTransactionState)) {
                q->disconnectFromHost(false);
            }
            return false;
        }

        delete currentTransactionState;
        currentTransactionState = 0;
        return true;
    }

    QByteArray collectPipelineCommands(TransactionState *ts)
    {
        Q_ASSERT(ts);
        QByteArray cmdLine;
        unsigned int cmdLine_len = 0;

        while (!mPendingCommandQueue.isEmpty()) {

            Command *cmd = mPendingCommandQueue.head();

            if (cmd->doNotExecute(ts)) {
                delete mPendingCommandQueue.dequeue();
                if (cmdLine_len) {
                    break;
                } else {
                    continue;
                }
            }

            if (cmdLine_len && cmd->mustBeFirstInPipeline()) {
                break;
            }

            if (cmdLine_len && !canPipelineCommands()) {
                break;
            }

            while (!cmd->isComplete() && !cmd->needsResponse()) {
                const QByteArray currentCmdLine = cmd->nextCommandLine(ts);
                if (ts->failedFatally()) {
                    return cmdLine;
                }
                const unsigned int currentCmdLine_len = currentCmdLine.length();

                cmdLine_len += currentCmdLine_len;
                cmdLine += currentCmdLine;

                // If we are executing the transfer command, don't collect the whole
                // command line (which may be several MBs) before sending it, but instead
                // send the data each time we have collected 32 KB of the command line.
                //
                // This way, the progress information in clients like KMail works correctly,
                // because otherwise, the TransferCommand would read the whole data from the
                // job at once, then sending it. The progress update on the client however
                // happens when sending data to the job, not when this slave writes the data
                // to the socket. Therefore that progress update is incorrect.
                //
                // 32 KB seems to be a sensible limit. Additionally, a job can only transfer
                // 32 KB at once anyway.
                if (dynamic_cast<TransferCommand *>(cmd) != 0 &&
                        cmdLine_len >= 32 * 1024) {
                    return cmdLine;
                }
            }

            mSentCommandQueue.enqueue(mPendingCommandQueue.dequeue());

            if (cmd->mustBeLastInPipeline()) {
                break;
            }
        }

        return cmdLine;
    }

    void receivedNewData()
    {
        qDebug();
        while (socket->canReadLine()) {
            const QByteArray buffer = socket->readLine();
            qDebug() << "S: >>" << buffer << "<<";
            currentResponse.parseLine(buffer, buffer.size());
            // ...until the response is complete or the parser is so confused
            // that it doesn't think a RSET would help anymore:
            if (currentResponse.isComplete()) {
                handleResponse(currentResponse);
                currentResponse = Response();
            } else if (!currentResponse.isWellFormed()) {
                error(KIO::ERR_NO_CONTENT,
                      i18n("Invalid SMTP response (%1) received.", currentResponse.code()));
            }
        }
    }

    void handleResponse(const KioSMTP::Response &response)
    {
        if (!mSentCommandQueue.isEmpty()) {
            Command *cmd = mSentCommandQueue.head();
            Q_ASSERT(cmd->isComplete());
            cmd->processResponse(response, currentTransactionState);
            if (currentTransactionState->failedFatally()) {
                q->disconnectFromHost(false);
            }
            delete mSentCommandQueue.dequeue();

            if (mSentCommandQueue.isEmpty()) {
                if (!mPendingCommandQueue.isEmpty()) {
                    runQueuedCommands(currentTransactionState);
                } else if (state == Sending) {
                    delete currentTransactionState;
                    currentTransactionState = 0;
                    q->disconnectFromHost(); // we are done
                }
            }
            return;
        }

        if (currentCommand) {
            if (!currentCommand->processResponse(response, currentTransactionState)) {
                q->disconnectFromHost(false);
            }
            while (!currentCommand->isComplete() && !currentCommand->needsResponse()) {
                const QByteArray cmdLine = currentCommand->nextCommandLine(currentTransactionState);
                if (currentTransactionState && currentTransactionState->failedFatally()) {
                    q->disconnectFromHost(false);
                }
                if (cmdLine.isEmpty()) {
                    continue;
                }
                if (!sendCommandLine(cmdLine)) {
                    q->disconnectFromHost(false);
                }
            }
            if (currentCommand->isComplete()) {
                Command *cmd = currentCommand;
                currentCommand = 0;
                currentTransactionState = 0;
                handleCommand(cmd);
            }
            return;
        }

        // command-less responses
        switch (state) {
        case Initial: { // server greeting
            if (!response.isOk()) {
                error(KIO::ERR_COULD_NOT_LOGIN,
                      i18n("The server (%1) did not accept the connection.\n%2",
                           destination.host(), response.errorMessage()));
                break;
            }
            state = EHLOPreTls;
            EHLOCommand *ehloCmdPreTLS = new EHLOCommand(this, myHostname);
            run(ehloCmdPreTLS);
            break;
        }
        default: error(KIO::ERR_SLAVE_DEFINED, i18n("Unhandled response"));
        }
    }

    void handleCommand(Command *cmd)
    {
        switch (state) {
        case StartTLS: {
            // re-issue EHLO to refresh the capability list (could be have
            // been faked before TLS was enabled):
            state = EHLOPostTls;
            EHLOCommand *ehloCmdPostTLS = new EHLOCommand(this, myHostname);
            run(ehloCmdPostTLS);
            break;
        }
        case EHLOPreTls: {
            if ((haveCapability("STARTTLS") &&
                    tlsRequested() != SMTPSessionInterface::ForceNoTLS) ||
                    tlsRequested() == SMTPSessionInterface::ForceTLS) {
                state = StartTLS;
                run(Command::STARTTLS);
                break;
            }
        }
        // fall through
        case EHLOPostTls: {
            // return with success if the server doesn't support SMTP-AUTH or an user
            // name is not specified and metadata doesn't tell us to force it.
            if (!destination.user().isEmpty() ||
                    haveCapability("AUTH") ||
                    !requestedSaslMethod().isEmpty()) {
                authInfo.username = destination.user();
                authInfo.password = destination.password();
                authInfo.prompt = i18n("Username and password for your SMTP account:");

                QStringList strList;
                if (!requestedSaslMethod().isEmpty()) {
                    strList.append(requestedSaslMethod());
                } else {
                    strList = capabilities().saslMethodsQSL();
                }

                state = Authenticated;
                AuthCommand *authCmd =
                    new AuthCommand(this, strList.join(QLatin1String(" ")).toLatin1(),
                                    destination.host(), authInfo);
                run(authCmd);
                break;
            }
        }
        // fall through
        case Authenticated: {
            state = Sending;
            queueCommand(new MailFromCommand(this, request.fromAddress().toLatin1(),
                                             request.is8BitBody(), request.size()));
            // Loop through our To and CC recipients, and send the proper
            // SMTP commands, for the benefit of the server.
            const QStringList recipients = request.recipients();
            for (QStringList::const_iterator it = recipients.begin(); it != recipients.end(); ++it) {
                queueCommand(new RcptToCommand(this, (*it).toLatin1()));
            }

            queueCommand(Command::DATA);
            queueCommand(new TransferCommand(this, QByteArray()));

            TransactionState *ts = new TransactionState;
            if (!runQueuedCommands(ts)) {
                if (ts->errorCode()) {
                    error(ts->errorCode(), ts->errorMessage());
                }
            }
            break;
        }
        case Reset:
            q->disconnectFromHost(true);
            break;
        default:
            error(KIO::ERR_SLAVE_DEFINED, i18n("Unhandled command response."));
        }

        delete cmd;
    }

public:
    QString saslMethod;
    bool useTLS;

    QUrl destination;
    KTcpSocket *socket;
    QIODevice *data;
    KioSMTP::Response currentResponse;
    KioSMTP::Command *currentCommand;
    KioSMTP::TransactionState *currentTransactionState;
    KIO::AuthInfo authInfo;
    KioSMTP::Request request;
    QString errorMessage;
    QString myHostname;

    enum State {
        Initial,
        EHLOPreTls,
        StartTLS,
        EHLOPostTls,
        Authenticated,
        Sending,
        Reset
    };
    State state;

    typedef QQueue<KioSMTP::Command *> CommandQueue;
    CommandQueue mPendingCommandQueue;
    CommandQueue mSentCommandQueue;

    static bool saslInitialized;

private:
    SmtpSession *q;
};

bool SmtpSessionPrivate::saslInitialized = false;

SmtpSession::SmtpSession(QObject *parent) :
    QObject(parent),
    d(new SmtpSessionPrivate(this))
{
    qDebug();
    d->socket = new KTcpSocket(this);
    connect(d->socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(d->socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
    connect(d->socket, SIGNAL(error(KTcpSocket::Error)), SLOT(socketError(KTcpSocket::Error)));
    connect(d->socket, SIGNAL(readyRead()), SLOT(receivedNewData()), Qt::QueuedConnection);

    if (!d->saslInitialized) {
        if (!initSASL()) {
            exit(-1);
        }
        d->saslInitialized = true;
    }
}

SmtpSession::~SmtpSession()
{
    qDebug();
    delete d;
}

void SmtpSession::setSaslMethod(const QString &method)
{
    d->saslMethod = method;
}

void SmtpSession::setUseTLS(bool useTLS)
{
    d->useTLS = useTLS;
}

void SmtpSession::connectToHost(const QUrl &url)
{
    qDebug() << url;
    d->socket->connectToHost(url.host(), url.port());
}

void SmtpSession::disconnectFromHost(bool nice)
{
    if (d->socket->state() == KTcpSocket::ConnectedState) {
        if (nice) {
            d->run(Command::QUIT);
        }

        d->socket->disconnectFromHost();

        d->clearCapabilities();
        qDeleteAll(d->mPendingCommandQueue);
        d->mPendingCommandQueue.clear();
        qDeleteAll(d->mSentCommandQueue);
        d->mSentCommandQueue.clear();
    }
}

void SmtpSession::sendMessage(const QUrl &destination, QIODevice *data)
{
    d->destination = destination;
    if (d->socket->state() != KTcpSocket::ConnectedState &&
            d->socket->state() != KTcpSocket::ConnectingState) {
        connectToHost(destination);
    }

    d->data = data;
    d->request = Request::fromURL(destination);   // parse settings from URL's query

    if (!d->request.heloHostname().isEmpty()) {
        d->myHostname = d->request.heloHostname();
    } else {
        d->myHostname = QHostInfo::localHostName();
        if (d->myHostname.isEmpty()) {
            d->myHostname = QLatin1String("localhost.invalid");
        } else if (!d->myHostname.contains(QLatin1Char('.'))) {
            d->myHostname += QLatin1String(".localnet");
        }
    }
}

QString SmtpSession::errorMessage() const
{
    return d->errorMessage;
}

#include "moc_smtpsession.cpp"
