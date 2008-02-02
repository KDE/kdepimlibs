
#ifdef HAVE_LIBSASL2

#include <stdio.h>
#include <QFile>
#include <QDir>
#include <KStandardDirs>

extern "C" {
#include <sasl/sasl.h>
}

inline bool initSASL()
{
#ifdef Q_OS_WIN32
  QByteArray libInstallPath( QFile::encodeName(QDir::toNativeSeparators(KGlobal::dirs()->installPath("lib")+"sasl2")) );
  QByteArray configPath( QFile::encodeName(QDir::toNativeSeparators(KGlobal::dirs()->installPath("config")+"sasl2")) );
  if ( sasl_set_path(SASL_PATH_TYPE_PLUGIN, libInstallPath.data()) != SASL_OK
    || sasl_set_path(SASL_PATH_TYPE_CONFIG, configPath.data()) != SASL_OK )
  {
    fprintf(stderr, "SASL path initialization failed!\n");
    return false;
  }
#endif

  if ( sasl_client_init( NULL ) != SASL_OK ) {
    fprintf(stderr, "SASL library initialization failed!\n");
    return false;
  }
  return true;
}

#endif // HAVE_LIBSASL2
