/*
   Copyright (c) 2003 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "script.h"


#include <cstdlib>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstring>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>

#include <kurl.h>
#include <kjs/object.h>
#include <kjs/JSVariableObject.h>

using namespace KJS;

QString UString::qstring() const
{
    return QString( reinterpret_cast< const QChar* >( data() ), size() );
}

UString::UString( const QString &s )
{
    const unsigned int len = s.length();
    UChar *data = static_cast<UChar*>( fastMalloc( sizeof(UChar) * len ) );
    memcpy( data, s.unicode(), len * sizeof( UChar ) );
    m_rep = Rep::create( data, len );
}

namespace
{
    class Address
    {
    public:
        struct Error {};
        static Address resolve( const UString& host )
            { return Address( host.qstring(), false ); }
        static Address parse( const UString& ip )
            { return Address( ip.qstring(), true ); }

        operator QHostAddress() const { return m_address; }
        operator UString() const { return UString( m_address.toString() ); }

    private:
        Address( const QString& host, bool numeric )
        {
            if ( numeric ) {
                m_address = QHostAddress( host );
                if ( m_address.isNull() )
                    throw Error();
            } else {
                QHostInfo addresses = QHostInfo::fromName(host);
                if ( addresses.error() || addresses.addresses().isEmpty() )
                    throw Error();
                m_address = addresses.addresses().at(0);
            }
        }

        QHostAddress m_address;
    };

    struct Function : public JSObject
    {
        struct ResolveError {};

        virtual bool implementsCall() const { return true; }

        static int findString( const UString& s, const char* const* values )
        {
            int index = 0;
            UString lower = s.qstring().toLower();
            for ( const char* const* p = values; *p; ++p, ++index )
                if ( lower == *p ) return index;
            return -1;
        }

        static const tm* getTime( ExecState* exec, const List& args )
        {
            time_t now = std::time( 0 );
            if ( args[ args.size() - 1 ]->toString( exec ).qstring().toLower() == "gmt" )
                return std::gmtime( &now );
            else return std::localtime( &now );
        }

        JSValue *checkRange( double value, double min, double max )
        {
            return jsBoolean(( min <= max && value >= min && value <= max ) || ( min > max && ( value <= min || value >= max ) ));
        }
    };

    // isPlainHostName( host )
    // @returns true if @p host doesn't contains a domain part
    struct IsPlainHostName : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 1 ) return jsUndefined();
            return jsBoolean( args[ 0 ]->toString( exec ).qstring().indexOf( "." ) == -1 );
        }
    };

    // dnsDomainIs( host, domain )
    // @returns true if the domain part of @p host matches @p domain
    struct DNSDomainIs : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 2 ) return jsUndefined();
            QString host = args[ 0 ]->toString( exec ).qstring().toLower();
            QString domain = args[ 1 ]->toString( exec ).qstring().toLower();
            return jsBoolean( host.endsWith( domain ) );
        }
    };

    // localHostOrDomainIs( host, fqdn )
    // @returns true if @p host is unqualified or equals @p fqdn
    struct LocalHostOrDomainIs : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 2 ) return jsUndefined();
            UString host = args[ 0 ]->toString( exec ).qstring().toLower();
            if ( host.find( "." ) == -1 ) return jsBoolean( true );
            UString fqdn = args[ 1 ]->toString( exec ).qstring().toLower();
            return jsBoolean( host == fqdn );
        }
    };

    // isResolvable( host )
    // @returns true if host can be resolved via DNS
    struct IsResolvable : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 1 ) return jsUndefined();
            try { Address::resolve( args[ 0 ]->toString( exec ) ); }
            catch ( const Address::Error& ) { return jsBoolean( false ); }
            return jsBoolean( true );
        }
    };

    // isInNet( host, subnet, mask )
    // @returns true if @p host is within the IP subnet
    //          specified via @p subnet and @p mask
    struct IsInNet : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 3 ) return jsUndefined();
            try
            {
                QHostAddress host = Address::resolve( args[ 0 ]->toString( exec ) );
                QHostAddress subnet = Address::parse( args[ 1 ]->toString( exec ) );
                QHostAddress mask = Address::parse( args[ 2 ]->toString( exec ) );

                return jsBoolean( ( host.toIPv4Address() & mask.toIPv4Address() ) ==
                                ( subnet.toIPv4Address() & mask.toIPv4Address() ) );
            }
            catch ( const Address::Error& )
            {
                return jsUndefined();
            }
        }
    };

    // dnsResolve( host )
    // @returns the IP address of @p host in dotted quad notation
    struct DNSResolve : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 1 ) return jsUndefined();
            try { return jsString(Address::resolve( args[ 0 ]->toString( exec ) )); }
            catch ( const Address::Error& ) { return jsUndefined(); }
        }
    };

    // myIpAddress()
    // @returns the local machine's IP address in dotted quad notation
    struct MyIpAddress : public Function
    {
        virtual JSValue *callAsFunction( ExecState*, JSObject*, const List& args )
        {
            if ( args.size() ) return jsUndefined();
            char hostname[ 256 ];
            gethostname( hostname, 255 );
            hostname[ 255 ] = 0;
            try { return jsString(Address::resolve( hostname )); }
            catch ( const Address::Error& ) { return jsUndefined(); }
        }
    };

    // dnsDomainLevels( host )
    // @returns the number of dots ('.') in @p host
    struct DNSDomainLevels : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 1 ) return jsUndefined();
            UString host = args[ 0 ]->toString( exec );
            if ( host.isNull() ) return jsNumber( 0 );
#ifdef __SUNPRO_CC
            /* 
             * Under Solaris, the default STL is the old broken interface 
             * to ::count which takes an extra Size& parameter.
             */
            int c = 0;
            std::count( host.data(), host.data() + host.size(), '.', c );
            return jsNumber(c);
#else
            return jsNumber( std::count(
                host.data(), host.data() + host.size(), '.' ) );
#endif
        }
    };

    // shExpMatch( str, pattern )
    // @returns true if @p str matches the shell @p pattern
    struct ShExpMatch : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() != 2 ) return jsUndefined();
            QRegExp pattern( args[ 1 ]->toString( exec ).qstring(), Qt::CaseSensitive, QRegExp::Wildcard );
            return jsBoolean( pattern.exactMatch(args[ 0 ]->toString( exec ).qstring()) );
        }
    };

    // weekdayRange( day [, "GMT" ] )
    // weekdayRange( day1, day2 [, "GMT" ] )
    // @returns true if the current day equals day or between day1 and day2 resp.
    // If the last argument is "GMT", GMT timezone is used, otherwise local time
    struct WeekdayRange : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() < 1 || args.size() > 3 ) return jsUndefined();
            static const char* const days[] =
                { "sun", "mon", "tue", "wed", "thu", "fri", "sat", 0 };
            int d1 = findString( args[ 0 ]->toString( exec ), days );
            if ( d1 == -1 ) return jsUndefined();

            int d2 = findString( args[ 1 ]->toString( exec ), days );
            if ( d2 == -1 ) d2 = d1;
            return checkRange( getTime( exec, args )->tm_wday, d1, d2 );
        }
    };

    // dateRange( day [, "GMT" ] )
    // dateRange( day1, day2 [, "GMT" ] )
    // dateRange( month [, "GMT" ] )
    // dateRange( month1, month2 [, "GMT" ] )
    // dateRange( year [, "GMT" ] )
    // dateRange( year1, year2 [, "GMT" ] )
    // dateRange( day1, month1, day2, month2 [, "GMT" ] )
    // dateRange( month1, year1, month2, year2 [, "GMT" ] )
    // dateRange( day1, month1, year1, day2, month2, year2 [, "GMT" ] )
    // @returns true if the current date (GMT or local time according to
    // presence of "GMT" as last argument) is within the given range
    struct DateRange : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() < 1 || args.size() > 7 ) return jsUndefined();
            static const char* const months[] =
                { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "nov", "dec", 0 };

            std::vector< double > values;
            for ( int i = 0; i < args.size(); ++i )
            {
                double value = -1;
                if ( args[ i ]->type() == NumberType )
                    value = args[ i ]->toInteger( exec );
                else value = findString( args[ i ]->toString( exec ), months );
                if ( value >= 0 ) values.push_back( value );
                else break;
            }

            const tm* now = getTime( exec, args );

            // day1, month1, year1, day2, month2, year2
            if ( values.size() == 6 )
                return checkRange( ( now->tm_year + 1900 ) * 372 + now->tm_mon * 31 + now->tm_mday,
                                   values[ 2 ] * 372 + values[ 1 ] * 31 + values[ 0 ],
                                   values[ 5 ] * 372 + values[ 4 ] * 31 + values[ 3 ] );

            // day1, month1, day2, month2
            else if ( values.size() == 4 &&
                      values[ 1 ] < 12 &&
                      values[ 3 ] < 12 )
                return checkRange( now->tm_mon * 31 + now->tm_mday,
                                   values[ 1 ] * 31 + values[ 0 ],
                                   values[ 3 ] * 31 + values[ 2 ] );

            // month1, year1, month2, year2
            else if ( values.size() == 4 )
                return checkRange( ( now->tm_year + 1900 ) * 12 + now->tm_mon,
                                   values[ 1 ] * 12 + values[ 0 ],
                                   values[ 3 ] * 12 + values[ 2 ] );

            // year1, year2
            else if ( values.size() == 2 &&
                      values[ 0 ] >= 1000 &&
                      values[ 1 ] >= 1000 )
                return checkRange( now->tm_year + 1900, values[ 0 ], values[ 1 ] );

            // day1, day2
            else if ( values.size() == 2 &&
                      args[ 0 ]->type() == NumberType &&
                      args[ 1 ]->type() == NumberType )
                return checkRange( now->tm_mday, values[ 0 ], values[ 1 ] );

            // month1, month2
            else if ( values.size() == 2 )
                return checkRange( now->tm_mon, values[ 0 ], values[ 1 ] );

            // year
            else if ( values.size() == 1 && values[ 0 ] >= 1000 )
                return checkRange( now->tm_year + 1900, values[ 0 ], values[ 0 ] );

            // day
            else if ( values.size() == 1 && args[ 0 ]->type() == NumberType )
                return checkRange( now->tm_mday, values[ 0 ], values[ 0 ] );

            // month
            else if ( values.size() == 1 )
                return checkRange( now->tm_mon, values[ 0 ], values[ 0 ] );

            else return jsUndefined();
        }
    };

    // timeRange( hour [, "GMT" ] )
    // timeRange( hour1, hour2 [, "GMT" ] )
    // timeRange( hour1, min1, hour2, min2 [, "GMT" ] )
    // timeRange( hour1, min1, sec1, hour2, min2, sec2 [, "GMT" ] )
    // @returns true if the current time (GMT or local based on presence
    // of "GMT" argument) is within the given range
    struct TimeRange : public Function
    {
        virtual JSValue *callAsFunction( ExecState* exec, JSObject*, const List& args )
        {
            if ( args.size() < 1 || args.size() > 7 ) return jsUndefined();

            std::vector< double > values;
            for ( int i = 0; i < args.size(); ++i )
                if ( args[ i ]->type() == NumberType )
                    values.push_back( args[ i ]->toInteger( exec ) );
                else break;

            const tm* now = getTime( exec, args );

            // hour1, min1, sec1, hour2, min2, sec2
            if ( values.size() == 6 )
                return checkRange( now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec,
                                   values[ 0 ] * 3600 + values[ 1 ] * 60 + values[ 2 ],
                                   values[ 3 ] * 3600 + values[ 4 ] * 60 + values[ 5 ] );

            // hour1, min1, hour2, min2
            else if ( values.size() == 4 )
                return checkRange( now->tm_hour * 60 + now->tm_min,
                                   values[ 0 ] * 60 + values[ 1 ],
                                   values[ 2 ] * 60 + values[ 3 ] );

            // hour1, hour2
            else if ( values.size() == 2 )
                return checkRange( now->tm_hour, values[ 0 ], values[ 1 ] );

            // hour
            else if ( values.size() == 1 )
                return checkRange( now->tm_hour, values[ 0 ], values[ 0 ] );

            else return jsUndefined();
        }
    };

    void registerFunctions( ExecState* exec, JSObject *global )
    {
        global->put( exec, "isPlainHostName", new IsPlainHostName );
        global->put( exec, "dnsDomainIs", new DNSDomainIs );
        global->put( exec, "localHostOrDomainIs", new LocalHostOrDomainIs );
        global->put( exec, "isResolvable", new IsResolvable );
        global->put( exec, "isInNet", new IsInNet );
        global->put( exec, "dnsResolve", new DNSResolve );
        global->put( exec, "myIpAddress", new MyIpAddress );
        global->put( exec, "dnsDomainLevels", new DNSDomainLevels );
        global->put( exec, "shExpMatch", new ShExpMatch );
        global->put( exec, "weekdayRange", new WeekdayRange );
        global->put( exec, "dateRange", new DateRange );
        global->put( exec, "timeRange", new TimeRange );
    }
}

namespace KPAC
{
    Script::Script( const QString& code )
    {
        m_interpreter = new KJS::Interpreter();
        m_interpreter->ref();
        ExecState* exec  = m_interpreter->globalExec();
        JSObject* global = m_interpreter->globalObject();
        registerFunctions( exec, global );

        Completion result = m_interpreter->evaluate( "", 0, code );
        if ( result.complType() == Throw )
            throw Error( result.value()->toString( exec ).qstring() );
    }
    
    Script::~Script()
    {
        m_interpreter->deref();
    }

    QString Script::evaluate( const KUrl& url )
    {
        ExecState *exec = m_interpreter->globalExec();
        JSValue *findFunc = m_interpreter->globalObject()->get( exec, "FindProxyForURL" );
        JSObject *findObj = findFunc->getObject();
        if (!findObj || !findObj->implementsCall())
            throw Error( "No such function FindProxyForURL" );
    
        List args;
        args.append(jsString(url.url()));
        args.append(jsString(url.host()));
        JSValue *retval = findObj->call( exec, m_interpreter->globalObject(), args );
        
        if ( exec->hadException() ) {
            JSValue *ex = exec->exception();
            exec->clearException();
            throw Error( ex->toString( exec ).qstring() );
        }

        return retval->toString( exec ).qstring();
    }
}

// vim: ts=4 sw=4 et
