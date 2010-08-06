#ifndef _BLA_H_
#define _BLA_H_

#include <k3process.h>
#include <QtCore/QString>
#include <QtCore/QObject>

class TestService : public QObject
{
    Q_OBJECT
public:
    TestService(const QString &exec);

    int exec();

public Q_SLOTS:
    void newApp(const QByteArray &appId);
    void endApp(const QByteArray &appId);
    void appExit();
    void stop();

protected:
    int result;
    QString m_exec;
    KProcess proc;
};

#endif
