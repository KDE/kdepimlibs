/* This file is part of KDE
   Copyright (C) 2000 by Wolfram Diestel <wolfram@steloj.de>
   Copyright (C) 2005 by Tim Way <tim@way.hrcoxmail.com>
   Copyright (C) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

   This is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
*/

#ifndef _NNTP_H
#define _NNTP_H


#include <kio/global.h>
#include <kio/tcpslavebase.h>

#define MAX_PACKET_LEN 4096

/* TODO:
  - test special post command
  - progress information in get, and maybe post
  - remove unnecessary debug stuff
*/

class NNTPProtocol:public KIO::TCPSlaveBase
{

 public:
  /** Default Constructor
   * @param isSSL is a true or false to indicate whether ssl is to be used
   */
  NNTPProtocol ( const QByteArray & pool, const QByteArray & app, bool isSSL );
  virtual ~NNTPProtocol();

  virtual void get(const KUrl& url );
  virtual void put( const KUrl& url, int permissions, KIO::JobFlags flags );
  virtual void stat(const KUrl& url );
  virtual void listDir(const KUrl& url );
  virtual void setHost(const QString& host, quint16 port,
        const QString& user, const QString& pass);

  /**
    *  Special command: 1 = post article
    *  it takes no other args, the article data are
    *  requested by dataReq() and should be valid
    *  as in RFC850. It's not checked for correctness here.
    *  @deprecated use put() for posting
    */
  virtual void special(const QByteArray& data);

 protected:

  /**
    *  Send a command to the server. Returns the response code and
    *  the response line
    */
  int sendCommand( const QString &cmd );

  /**
    *  Attempt to properly shut down the NNTP connection by sending
    *  "QUIT\r\n" before closing the socket.
    */
  void nntp_close ();

  /**
    * Attempt to initiate a NNTP connection via a TCP socket, if no existing
    * connection could be reused.
    */
  bool nntp_open();

  /**
    * Post article. Invoked by special() and put()
    */
  bool post_article();


 private:
   QString mHost, mUser, mPass;
   quint16 m_port, m_defaultPort;
   bool postingAllowed, opened, isAuthenticated;
   char readBuffer[MAX_PACKET_LEN];
   ssize_t readBufferLen;
   /// Current selected newsgroup
   QString mCurrentGroup;

   /**
    * Fetch all new groups since the given date or (if the date is empty)
    * all available groups.
    * @param since Date as specified in RFC 977 for the NEWGROUPS command
    * @param desc Fetch group description (needs more memory)
    */
   void fetchGroups( const QString &since, bool desc );
   /**
    * Fetch message listing from the given newsgroup.
    * This will use RFC2980 XOVER if available, plain RFC977 STAT/NEXT
    * otherwise.
    * @param group The newsgroup name
    * @param first Serial number of the first message, 0 lists all messages.
    * @param max Maximal number of returned messages, 0 means unlimited.
    * @return true on success, false otherwise.
    */
   bool fetchGroup ( QString &group, unsigned long first = 0, unsigned long max = 0 );
   /**
    * Fetch message listing from the current group using RFC977 STAT/NEXT
    * commands.
    * @param first message number of the first article
    * @return true on success, false otherwise.
    */
   bool fetchGroupRFC977( unsigned long first );
   /**
    * Fetch message listing from the current group using the RFC2980 XOVER
    * command.
    * Additional headers provided by XOVER are added as UDS_EXTRA entries
    * to the listing.
    * @param first message number of the first article
    * @param notSupported boolean reference to indicate if command failed
    * due to missing XOVER support on the server.
    * @return true on success, false otherwise
    */
   bool fetchGroupXOVER( unsigned long first, bool &notSupported );
   /// creates an UDSEntry with file information used in stat and listDir
   void fillUDSEntry ( KIO::UDSEntry & entry, const QString & name, long size,
                       bool is_article, long access = 0 );
   /// error  handling for unexpected responses
   void unexpected_response ( int res_code, const QString & command );
   /**
     * grabs the response line from the server. used after most send_cmd calls. max
     * length for the returned string ( char *data ) is 4096 characters including
     * the "\r\n" terminator.
     */
   int evalResponse ( char *data, ssize_t &len );
   /**
    * Try to authenticate to the server.
    * @return the response code from the server if the mUser/mPassword
    * are available; 281 (successfull authentication) otherwise.
    */
   int authenticate();
};

#endif
