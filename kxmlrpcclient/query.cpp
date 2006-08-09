/**************************************************************************
*   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>        *
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <QVariant>
#include <qdom.h>

#include "query.h"

namespace XmlRpc {

/**

	@file

	Implementation of KXmlRpcResult and Query

**/



//small macro taken from HTTP IOSlave
#define KIO_ARGS QByteArray packedArgs; QDataStream kioArgsStream( &packedArgs, IO_WriteOnly ); kioArgsStream

Query *Query::create( const QVariant &id, QObject *parent, const char *name ) {

  return new Query( id, parent, name );
}

void Query::call( const QString &server, const QString &method,
                  const QList<QVariant> &args, const QString &userAgent ) {

	const QString xmlMarkup = markupCall( method, args );

	QByteArray postData;
	QDataStream stream( &postData, IO_WriteOnly );
	stream.writeRawBytes( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

	KIO_ARGS << (int)1 << KUrl(server);
	KIO::TransferJob *job = new KIO::TransferJob(KUrl(server), KIO::CMD_SPECIAL, packedArgs, postData, false);

	if ( !job ) {
		kWarning() << "Unable to create KIO job for " << server << endl;
		return;
	}
	job->addMetaData( "UserAgent", userAgent );
	job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
	job->addMetaData( "ConnectTimeout", "50" );

	connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           this, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
	connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotResult( KIO::Job * ) ) );

	m_pendingJobs.append( job );
}

void Query::slotData( KIO::Job *, const QByteArray &data ) {

	unsigned int oldSize = m_buffer.size();
	m_buffer.resize( oldSize + data.size() );
	memcpy( m_buffer.data() + oldSize, data.data(), data.size() );
}

void Query::slotResult( KIO::Job *job ) {

	m_pendingJobs.remove( job );

	if ( job->error() != 0 ) {

		emit fault( job->error(), job->errorString(), m_id );
		emit finished( this );
		return ;
	}

	QString data = QString::fromUtf8( m_buffer.data(), m_buffer.size() );

	QDomDocument doc;
	QString errMsg;
	int errLine, errCol;
	if ( !doc.setContent( data, false, &errMsg, &errLine, &errCol  ) ) {

		emit fault( -1, i18n( "Received invalid XML markup: %1 at %2:%3" )
                        .arg( errMsg ).arg( errLine ).arg( errCol ), m_id );
		emit finished( this );
		return ;
	}

	m_buffer.truncate( 0 );

	if ( isMessageResponse( doc ) )
		emit message( parseMessageResponse( doc ).data(), m_id );
	else if ( isFaultResponse( doc ) ) {
		emit fault( parseFaultResponse( doc ).errorCode(), parseFaultResponse( doc ).errorString(), m_id );
	} else {
		emit fault( 1, i18n( "Unknown type of XML markup received" ), m_id );
	}

	emit finished( this );
}

bool Query::isMessageResponse( const QDomDocument &doc ) const {

	return doc.documentElement().firstChild().toElement().tagName().lower() == "params";
}

KXmlRpcResult Query::parseMessageResponse( const QDomDocument &doc ) const {

	KXmlRpcResult response;
	response.m_success = true;

	QDomNode paramNode = doc.documentElement().firstChild().firstChild();
	while ( !paramNode.isNull() ) {

		response.m_data << demarshal( paramNode.firstChild().toElement() );
		paramNode = paramNode.nextSibling();
	}

	return response;
}

bool Query::isFaultResponse( const QDomDocument &doc ) const {

	return doc.documentElement().firstChild().toElement().tagName().lower() == "fault";
}

KXmlRpcResult Query::parseFaultResponse( const QDomDocument &doc ) const {

	KXmlRpcResult response;
	response.m_success = false;

	QDomNode errorNode = doc.documentElement().firstChild().firstChild();
	const QVariant errorVariant = demarshal( errorNode.toElement() );
	response.m_errorCode = errorVariant.toMap() [ "faultCode" ].toInt();
	response.m_errorString = errorVariant.toMap() [ "faultString" ].toString();

	return response;
}

QString Query::markupCall( const QString &cmd, const QList<QVariant> &args ) const {

	QString markup = "<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n";

	markup += "<methodName>" + cmd + "</methodName>\r\n";

	if ( !args.isEmpty() ) {

		markup += "<params>\r\n";
		QList<QVariant>::ConstIterator it = args.begin();
		QList<QVariant>::ConstIterator end = args.end();
		for ( ; it != end; ++it )
			markup += "<param>\r\n" + marshal( *it ) + "</param>\r\n";
		markup += "</params>\r\n";
	}

	markup += "</methodCall>\r\n";

	return markup;
}

QString Query::marshal( const QVariant &arg ) const {

	switch ( arg.type() ) {

		case QVariant::String:
			return "<value><string>" + arg.toString() + "</string></value>\r\n";
		case QVariant::Int:
			return "<value><int>" + QString::number( arg.toInt() ) + "</int></value>\r\n";
		case QVariant::Double:
			return "<value><double>" + QString::number( arg.toDouble() ) + "</double></value>\r\n";
		case QVariant::Bool:
			{
				QString markup = "<value><boolean>";
				markup += arg.toBool() ? "1" : "0";
				markup += "</boolean></value>\r\n";
				return markup;
			}
		case QVariant::ByteArray:
			return "<value><base64>" + arg.toByteArray().toBase64() + "</base64></value>\r\n";
		case QVariant::DateTime:
			return "<value><datetime.iso8601>" + arg.toDateTime().toString( Qt::ISODate ) + "</datetime.iso8601></value>\r\n";
		case QVariant::List:
			{
				QString markup = "<value><array><data>\r\n";
				const QList<QVariant> args = arg.toList();
				QList<QVariant>::ConstIterator it = args.begin();
				QList<QVariant>::ConstIterator end = args.end();
				for ( ; it != end; ++it )
					markup += marshal( *it );
				markup += "</data></array></value>\r\n";
				return markup;
			}
		case QVariant::Map:
			{
				QString markup = "<value><struct>\r\n";
				QMap<QString, QVariant> map = arg.toMap();
				QMap<QString, QVariant>::ConstIterator it = map.begin();
				QMap<QString, QVariant>::ConstIterator end = map.end();
				for ( ; it != end; ++it ) {
					markup += "<member>\r\n";
					markup += "<name>" + it.key() + "</name>\r\n";
					markup += marshal( it.data() );
					markup += "</member>\r\n";
				}
				markup += "</struct></value>\r\n";
				return markup;
			}
		default:
			kWarning() << "Failed to marshal unknown variant type: " << arg.type() << endl;
	};
	return QString::null;
}

QVariant Query::demarshal( const QDomElement &elem ) const {

	Q_ASSERT( elem.tagName().lower() == "value" );

	const QDomElement typeElement = elem.firstChild().toElement();
	const QString typeName = typeElement.tagName().lower();

	if ( typeName == "string" )
		return QVariant( typeElement.text() );
	else if ( typeName == "i4" || typeName == "int" )
		return QVariant( typeElement.text().toInt() );
	else if ( typeName == "double" )
		return QVariant( typeElement.text().toDouble() );
	else if ( typeName == "boolean" ) {

	 if ( typeElement.text().lower() == "true" || typeElement.text() == "1" )
		return QVariant( true );
	else
		return QVariant( false );
	}
	else if ( typeName == "base64" )
		return QVariant( QByteArray::fromBase64( typeElement.text().toLatin1() ) );
	else if ( typeName == "datetime" || typeName == "datetime.iso8601" )
		return QVariant( QDateTime::fromString( typeElement.text(), Qt::ISODate ) );
	else if ( typeName == "array" ) {
		QList<QVariant> values;
		QDomNode valueNode = typeElement.firstChild().firstChild();
		while ( !valueNode.isNull() ) {
			values << demarshal( valueNode.toElement() );
			valueNode = valueNode.nextSibling();
		}
	return QVariant( values );
	} else if ( typeName == "struct" ) {

		QMap<QString, QVariant> map;
		QDomNode memberNode = typeElement.firstChild();
		while ( !memberNode.isNull() ) {
			const QString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
			const QVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
			map[ key ] = data;
			memberNode = memberNode.nextSibling();
		}
		return QVariant( map );
	} else {
		kWarning() << "Cannot demarshal unknown type " << typeName << endl;
	}
	return QVariant();
}

Query::Query( const QVariant &id, QObject *parent, const char *name )
  : QObject( parent, name ), m_id( id )
{}

Query::~Query() {

	QList<KIO::Job*>::Iterator it;
	for ( it = m_pendingJobs.begin(); it != m_pendingJobs.end(); ++it )
		(*it)->kill();
}

} // namespace XmlRpc

#include "query.moc"

