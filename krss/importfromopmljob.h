#ifndef KRSS_IMPORTFROMOPMLJOB_H
#define KRSS_IMPORTFROMOPMLJOB_H

#include "krss_export.h"

#include <Akonadi/Collection>
#include <KJob>

namespace KRss {
  
class KRSS_EXPORT ImportFromOpmlJob : public KJob {
    Q_OBJECT
public:
    explicit ImportFromOpmlJob( QObject* parent=0 );
    ~ImportFromOpmlJob();

    void start();

    QString inputFile() const;
    void setInputFile( const QString& path );

    QString opmlTitle() const;

    Akonadi::Collection::List collections() const;

    Akonadi::Collection parentFolder() const;
    void setParentFolder( const Akonadi::Collection& parentFolder );

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT(d, void doStart())
};

}

#endif
