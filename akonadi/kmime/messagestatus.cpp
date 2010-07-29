/*
    This file is part of Akonadi.
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "messagestatus.h"

#include <KDE/KDebug>
#include <QString>
#include "messageflags.h"

/** The message status format. These can be or'd together.
    Note, that the KMMsgStatusIgnored implies the
    status to be Read even if the flags are set
    to Unread or New. This is done in isRead()
    and related getters. So we can preserve the state
    when switching a thread to Ignored and back. */
enum MsgStatus {
    KMMsgStatusUnknown =           0x00000000,
    KMMsgStatusUnread =            0x00000002,
    KMMsgStatusRead =              0x00000004,
    KMMsgStatusDeleted =           0x00000010,
    KMMsgStatusReplied =           0x00000020,
    KMMsgStatusForwarded =         0x00000040,
    KMMsgStatusQueued =            0x00000080,
    KMMsgStatusSent =              0x00000100,
    KMMsgStatusFlag =              0x00000200, // flag means important
    KMMsgStatusWatched =           0x00000400,
    KMMsgStatusIgnored =           0x00000800, // forces isRead()
    KMMsgStatusToAct =             0x00001000,
    KMMsgStatusSpam =              0x00002000,
    KMMsgStatusHam =               0x00004000,
    KMMsgStatusHasAttach =         0x00008000
};

class Akonadi::MessageStatusPrivate
{
public:
  qint32 status;
};

Akonadi::MessageStatus::MessageStatus() : d( new MessageStatusPrivate )
{
  d->status = KMMsgStatusUnknown;
}

Akonadi::MessageStatus::~MessageStatus()
{
  delete d;
}


Akonadi::MessageStatus &Akonadi::MessageStatus::operator = ( const Akonadi::MessageStatus &other )
{
  d->status = other.d->status;
  return *this;
}

bool Akonadi::MessageStatus::operator == ( const Akonadi::MessageStatus &other ) const
{
  return ( d->status == other.d->status );
}

bool Akonadi::MessageStatus::operator != ( const Akonadi::MessageStatus &other ) const
{
  return ( d->status != other.d->status );
}

bool Akonadi::MessageStatus::operator & ( const Akonadi::MessageStatus &other ) const
{
  return ( d->status & other.d->status );
}

void Akonadi::MessageStatus::clear()
{
  d->status = KMMsgStatusUnknown;
}

void Akonadi::MessageStatus::set( const Akonadi::MessageStatus &other )
{
  // Those stati are exclusive, but we have to lock at the
  // internal representation because Ignored can manipulate
  // the result of the getter methods.
  if ( other.d->status & KMMsgStatusUnread ) {
    setUnread();
  }
  if ( other.d->status & KMMsgStatusRead ) {
    setRead();
  }
  if ( other.isDeleted() ) {
    setDeleted();
  }
  if ( other.isReplied() ) {
    setReplied();
  }
  if ( other.isForwarded() ) {
    setForwarded();
  }
  if ( other.isQueued() ) {
    setQueued();
  }
  if ( other.isSent() ) {
    setSent();
  }
  if ( other.isImportant() ) {
    setImportant();
  }

  if ( other.isWatched() ) {
    setWatched();
  }
  if ( other.isIgnored() ) {
    setIgnored();
  }
  if ( other.isToAct() ) {
    setToAct();
  }
  if ( other.isSpam() ) {
    setSpam();
  }
  if ( other.isHam() ) {
    setHam();
  }
  if ( other.hasAttachment() ) {
    setHasAttachment();
  }
}

void Akonadi::MessageStatus::toggle( const Akonadi::MessageStatus &other )
{
  if ( other.isDeleted() ) {
    setDeleted( !( d->status & KMMsgStatusDeleted ) );
  }
  if ( other.isReplied() ) {
    setReplied( !( d->status & KMMsgStatusReplied ) );
  }
  if ( other.isForwarded() ) {
    setForwarded( !( d->status & KMMsgStatusForwarded ) );
  }
  if ( other.isQueued() ) {
    setQueued( !( d->status & KMMsgStatusQueued ) );
  }
  if ( other.isSent() ) {
    setSent( !( d->status & KMMsgStatusSent ) );
  }
  if ( other.isImportant() ) {
    setImportant( !( d->status & KMMsgStatusFlag ) );
  }

  if ( other.isWatched() ) {
    setWatched( !( d->status & KMMsgStatusWatched ) );
  }
  if ( other.isIgnored() ) {
    setIgnored( !( d->status & KMMsgStatusIgnored ) );
  }
  if ( other.isToAct() ) {
    setToAct( !( d->status & KMMsgStatusToAct ) );
  }
  if ( other.isSpam() ) {
    setSpam( !( d->status & KMMsgStatusSpam ) );
  }
  if ( other.isHam() ) {
    setHam( !( d->status & KMMsgStatusHam ) );
  }
  if ( other.hasAttachment() ) {
    setHasAttachment( !( d->status & KMMsgStatusHasAttach ) );
  }
}

bool Akonadi::MessageStatus::isOfUnknownStatus() const
{
  return ( d->status == KMMsgStatusUnknown );
}

bool Akonadi::MessageStatus::isUnread() const
{
  return ( d->status & KMMsgStatusUnread && !( d->status & KMMsgStatusIgnored ) );
}

bool Akonadi::MessageStatus::isRead() const
{
  return ( d->status & KMMsgStatusRead || d->status & KMMsgStatusIgnored );
}

bool Akonadi::MessageStatus::isDeleted() const
{
  return ( d->status & KMMsgStatusDeleted );
}

bool Akonadi::MessageStatus::isReplied() const
{
  return ( d->status & KMMsgStatusReplied );
}

bool Akonadi::MessageStatus::isForwarded() const
{
  return ( d->status & KMMsgStatusForwarded );
}

bool Akonadi::MessageStatus::isQueued() const
{
  return ( d->status & KMMsgStatusQueued );
}

bool Akonadi::MessageStatus::isSent() const
{
   return ( d->status & KMMsgStatusSent );
}

bool Akonadi::MessageStatus::isImportant() const
{
  return ( d->status & KMMsgStatusFlag );
}

bool Akonadi::MessageStatus::isWatched() const
{
  return ( d->status & KMMsgStatusWatched );
}

bool Akonadi::MessageStatus::isIgnored() const
{
  return ( d->status & KMMsgStatusIgnored );
}

bool Akonadi::MessageStatus::isToAct() const
{
  return ( d->status & KMMsgStatusToAct );
}

bool Akonadi::MessageStatus::isSpam() const
{
  return ( d->status & KMMsgStatusSpam );
}

bool Akonadi::MessageStatus::isHam() const
{
  return ( d->status & KMMsgStatusHam );
}

bool Akonadi::MessageStatus::hasAttachment() const
{
  return ( d->status & KMMsgStatusHasAttach );
}

void Akonadi::MessageStatus::setUnread()
{
  // unread overrides read
  d->status &= ~KMMsgStatusRead;
  d->status |= KMMsgStatusUnread;
}

void Akonadi::MessageStatus::setRead()
{
  // Unset unread and set read
  d->status &= ~KMMsgStatusUnread;
  d->status |= KMMsgStatusRead;
}

void Akonadi::MessageStatus::setDeleted( bool deleted )
{
  if ( deleted ) {
    d->status |= KMMsgStatusDeleted;
  } else {
    d->status &= ~KMMsgStatusDeleted;
  }
}

void Akonadi::MessageStatus::setReplied( bool replied )
{
  if ( replied ) {
    d->status |= KMMsgStatusReplied;
  } else {
    d->status &= ~KMMsgStatusReplied;
  }
}

void Akonadi::MessageStatus::setForwarded( bool forwarded )
{
  if ( forwarded ) {
    d->status |= KMMsgStatusForwarded;
  } else {
    d->status &= ~KMMsgStatusForwarded;
  }
}

void Akonadi::MessageStatus::setQueued( bool queued )
{
  if ( queued ) {
    d->status |= KMMsgStatusQueued;
  } else {
    d->status &= ~KMMsgStatusQueued;
  }
}

void Akonadi::MessageStatus::setSent( bool sent )
{
  if ( sent ) {
    d->status &= ~KMMsgStatusQueued;
    // FIXME to be discussed if sent messages are Read
    d->status &= ~KMMsgStatusUnread;
    d->status |= KMMsgStatusSent;
  } else {
    d->status &= ~KMMsgStatusSent;
  }
}

void Akonadi::MessageStatus::setImportant( bool important )
{
  if ( important ) {
    d->status |= KMMsgStatusFlag;
  } else {
    d->status &= ~KMMsgStatusFlag;
  }
}

// Watched and ignored are mutually exclusive
void Akonadi::MessageStatus::setWatched( bool watched )
{
  if ( watched ) {
    d->status &= ~KMMsgStatusIgnored;
    d->status |= KMMsgStatusWatched;
  } else {
    d->status &= ~KMMsgStatusWatched;
  }
}

void Akonadi::MessageStatus::setIgnored( bool ignored )
{
  if ( ignored ) {
    d->status &= ~KMMsgStatusWatched;
    d->status |= KMMsgStatusIgnored;
  } else {
    d->status &= ~KMMsgStatusIgnored;
  }
}

void Akonadi::MessageStatus::setToAct( bool toAct )
{
  if ( toAct ) {
    d->status |= KMMsgStatusToAct;
  } else {
    d->status &= ~KMMsgStatusToAct;
  }
}

// Ham and Spam are mutually exclusive
void Akonadi::MessageStatus::setSpam( bool spam )
{
  if ( spam ) {
    d->status &= ~KMMsgStatusHam;
    d->status |= KMMsgStatusSpam;
  } else {
    d->status &= ~KMMsgStatusSpam;
  }
}

void Akonadi::MessageStatus::setHam( bool ham )
{
  if ( ham ) {
    d->status &= ~KMMsgStatusSpam;
    d->status |= KMMsgStatusHam;
  } else {
    d->status &= ~KMMsgStatusHam;
  }
}

void Akonadi::MessageStatus::setHasAttachment( bool withAttachment )
{
  if ( withAttachment ) {
    d->status |= KMMsgStatusHasAttach;
  } else {
    d->status &= ~KMMsgStatusHasAttach;
  }
}

qint32 Akonadi::MessageStatus::toQInt32() const
{
  return d->status;
}


void Akonadi::MessageStatus::fromQInt32( qint32 status )
{
  d->status = status;
}

QString Akonadi::MessageStatus::statusStr() const
{
  QByteArray sstr;
  if ( d->status & KMMsgStatusUnread ) {
    sstr += 'U';
  }
  if ( d->status & KMMsgStatusRead ) {
    sstr += 'R';
  }
  if ( d->status & KMMsgStatusDeleted ) {
    sstr += 'D';
  }
  if ( d->status & KMMsgStatusReplied ) {
    sstr += 'A';
  }
  if ( d->status & KMMsgStatusForwarded ) {
    sstr += 'F';
  }
  if ( d->status & KMMsgStatusQueued ) {
    sstr += 'Q';
  }
  if ( d->status & KMMsgStatusToAct ) {
    sstr += 'K';
  }
  if ( d->status & KMMsgStatusSent ) {
    sstr += 'S';
  }
  if ( d->status & KMMsgStatusFlag ) {
    sstr += 'G';
  }
  if ( d->status & KMMsgStatusWatched ) {
    sstr += 'W';
  }
  if ( d->status & KMMsgStatusIgnored ) {
    sstr += 'I';
  }
  if ( d->status & KMMsgStatusSpam ) {
    sstr += 'P';
  }
  if ( d->status & KMMsgStatusHam ) {
    sstr += 'H';
  }
  if ( d->status & KMMsgStatusHasAttach ) {
    sstr += 'T';
  }

  return QLatin1String( sstr );
}

void Akonadi::MessageStatus::setStatusFromStr( const QString& aStr )
{
  d->status = KMMsgStatusUnknown;

  if ( aStr.contains( QLatin1Char( 'U' ) ) ) {
    setUnread();
  }
  if ( aStr.contains( QLatin1Char( 'R' ) ) ) {
    setRead();
  }
  if ( aStr.contains( QLatin1Char( 'D' ) ) ) {
    setDeleted();
  }
  if ( aStr.contains( QLatin1Char( 'A' ) ) ) {
    setReplied();
  }
  if ( aStr.contains( QLatin1Char( 'F' ) ) ) {
    setForwarded();
  }
  if ( aStr.contains( QLatin1Char( 'Q' ) ) ) {
    setQueued();
  }
  if ( aStr.contains( QLatin1Char( 'K' ) ) ) {
    setToAct();
  }
  if ( aStr.contains( QLatin1Char( 'S' ) ) ) {
    setSent();
  }
  if ( aStr.contains( QLatin1Char( 'G' ) ) ) {
    setImportant();
  }
  if ( aStr.contains( QLatin1Char( 'W' ) ) ) {
    setWatched();
  }
  if ( aStr.contains( QLatin1Char( 'I' ) ) ) {
    setIgnored();
  }
  if ( aStr.contains( QLatin1Char( 'P' ) ) ) {
    setSpam();
  }
  if ( aStr.contains( QLatin1Char( 'H' ) ) ) {
    setHam();
  }
  if ( aStr.contains( QLatin1Char( 'T' ) ) ) {
    setHasAttachment();
  }
  if ( aStr.contains( QLatin1Char( 'C' ) ) ) {
    setHasAttachment( false );
  }
}

QSet<QByteArray> Akonadi::MessageStatus::statusFlags() const
{
  QSet<QByteArray> flags;

  // Non handled status:
  // * KMMsgStatusQueued
  // * KMMsgStatusSent
  // * KMMsgStatusSpam
  // * KMMsgStatusHam
  // * KMMsgStatusHasAttach

  if ( d->status & KMMsgStatusDeleted ) {
    flags+= Akonadi::MessageFlags::Deleted;
  } else {
    if ( d->status &  KMMsgStatusRead )
      flags+= Akonadi::MessageFlags::Seen;
    if ( d->status & KMMsgStatusReplied )
      flags+= Akonadi::MessageFlags::Answered;
    if ( d->status & KMMsgStatusFlag )
      flags+= Akonadi::MessageFlags::Flagged;
    // non standard flags
    if ( d->status & KMMsgStatusForwarded )
      flags+= "$FORWARDED";
    if ( d->status & KMMsgStatusToAct )
      flags+= "$TODO";
    if ( d->status & KMMsgStatusWatched )
      flags+= "$WATCHED";
    if ( d->status & KMMsgStatusIgnored )
      flags+= "$IGNORED";
    if ( d->status & KMMsgStatusHasAttach )
      flags+= "$ATTACHMENT";
  }

  return flags;
}

void Akonadi::MessageStatus::setStatusFromFlags( const QSet<QByteArray> &flags )
{
  d->status = KMMsgStatusUnknown;
  setUnread();
  // Non handled status:
  // * KMMsgStatusQueued
  // * KMMsgStatusSent
  // * KMMsgStatusSpam
  // * KMMsgStatusHam
  // * KMMsgStatusHasAttach

  foreach ( const QByteArray &flag, flags ) {
    const QByteArray &upperedFlag = flag.toUpper();
    if ( upperedFlag ==  Akonadi::MessageFlags::Deleted ) {
      setDeleted();
    } else if ( upperedFlag ==  Akonadi::MessageFlags::Seen ) {
      setRead();
    } else if ( upperedFlag ==  Akonadi::MessageFlags::Answered ) {
      setReplied();
    } else if ( upperedFlag ==  Akonadi::MessageFlags::Flagged ) {
      setImportant();

    // non standard flags
    } else if ( upperedFlag ==  "$FORWARDED" ) {
      setForwarded();
    } else if ( upperedFlag ==  "$TODO" ) {
      setToAct();
    } else if ( upperedFlag ==  "$WATCHED" ) {
      setWatched();
    } else if ( upperedFlag ==  "$IGNORED" ) {
      setIgnored();
    } else if ( upperedFlag ==  "$JUNK" ) {
      setSpam();
    } else if ( upperedFlag ==  "$NOTJUNK" ) {
      setHam();
    } else if ( upperedFlag ==  "$ATTACHMENT" ) {
      setHasAttachment( true );
    } else {
      kWarning() << "Unknown flag:" << flag;
    }
  }
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusRead()
{
  Akonadi::MessageStatus st;
  st.setRead();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusUnread()
{
  Akonadi::MessageStatus st;
  st.setUnread();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusDeleted()
{
  Akonadi::MessageStatus st;
  st.setDeleted();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusReplied()
{
  Akonadi::MessageStatus st;
  st.setReplied();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusForwarded()
{
  Akonadi::MessageStatus st;
  st.setForwarded();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusQueued()
{
  Akonadi::MessageStatus st;
  st.setQueued();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusSent()
{
  Akonadi::MessageStatus st;
  st.setSent();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusImportant()
{
  Akonadi::MessageStatus st;
  st.setImportant();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusWatched()
{
  Akonadi::MessageStatus st;
  st.setWatched();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusIgnored()
{
  Akonadi::MessageStatus st;
  st.setIgnored();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusToAct()
{
  Akonadi::MessageStatus st;
  st.setToAct();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusSpam()
{
  Akonadi::MessageStatus st;
  st.setSpam();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusHam()
{
  Akonadi::MessageStatus st;
  st.setHam();
  return st;
}

Akonadi::MessageStatus Akonadi::MessageStatus::statusHasAttachment()
{
  Akonadi::MessageStatus st;
  st.setHasAttachment();
  return st;
}
