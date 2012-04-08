#ifndef KRSS_EXPORTTOOPMLJOB_H
#define KRSS_EXPORTTOOPMLJOB_H

#include "krss_export.h"

#include "akonadi/collection.h"
#include <KJob>

namespace KRss {
  
class KRSS_EXPORT ExportToOpmlJob : public KJob {
    Q_OBJECT
public:
    explicit ExportToOpmlJob( QObject* parent=0 );
    ~ExportToOpmlJob();

    void start();

    QString outputFile() const;
    void setOutputFile( const QString& path );

    bool includeCustomProperties() const;
    void setIncludeCustomProperties( bool includeCustomProperties );

    QString resource() const;
    void setResource( const QString& identifier );

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT(d, void doStart())
    Q_PRIVATE_SLOT(d, void fetchFinished(KJob*))
};

}

#endif
