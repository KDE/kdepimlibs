/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2008 Erlend Hamberg <ehamberg@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "katevireplacemode.h"
#include "kateviinputmodemanager.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include "kateconfig.h"

KateViReplaceMode::KateViReplaceMode( KateViInputModeManager *viInputModeManager,
    KateView * view, KateViewInternal * viewInternal ) : KateViModeBase()
{
  m_view = view;
  m_viewInternal = viewInternal;
  m_viInputModeManager = viInputModeManager;
}

KateViReplaceMode::~KateViReplaceMode()
{
}

bool KateViReplaceMode::commandInsertFromLine( int offset )
{
  KTextEditor::Cursor c( m_view->cursorPosition() );
  KTextEditor::Cursor c2( c.line(), c.column()+1 );

  if ( c.line()+offset > doc()->lines() || c.line()+offset < 0 ) {
    return false;
  }

  QString line = doc()->line( c.line()+offset );
  int tabWidth = doc()->config()->tabWidth();
  QChar ch = getCharAtVirtualColumn( line, m_view->virtualCursorColumn(), tabWidth );
  QChar removed = doc()->line( c.line() ).at( c.column() );

  if ( ch == QChar::Null ) {
    return false;
  }

  if ( doc()->replaceText( KTextEditor::Range( c, c2 ), ch ) ) {
    overwrittenChar( removed );
    return true;
  }

  return false;
}

bool KateViReplaceMode::commandMoveOneWordLeft()
{
  KTextEditor::Cursor c( m_view->cursorPosition() );
  c = findPrevWordStart( c.line(), c.column() );

  updateCursor( c );
  return true;
}

bool KateViReplaceMode::commandMoveOneWordRight()
{
  KTextEditor::Cursor c( m_view->cursorPosition() );
  c = findNextWordStart( c.line(), c.column() );

  updateCursor( c );
  return true;
}

/**
 * checks if the key is a valid command
 * @return true if a command was completed and executed, false otherwise
 */
bool KateViReplaceMode::handleKeypress( const QKeyEvent *e )
{
  // backspace should work even if the shift key is down
  if (e->modifiers() != Qt::ControlModifier && e->key() == Qt::Key_Backspace ) {
    backspace();
    return true;
  }

  KTextEditor::Cursor c( m_view->cursorPosition() );

  if ( e->modifiers() == Qt::NoModifier ) {
    switch ( e->key() ) {
    case Qt::Key_Escape:
      m_overwritten.clear();
      startNormalMode();
      return true;
      break;
    case Qt::Key_Left:
      m_overwritten.clear();
      m_view->cursorLeft();
      return true;
    case Qt::Key_Right:
      m_overwritten.clear();
      m_view->cursorRight();
      return true;
    case Qt::Key_Home:
      m_overwritten.clear();
      m_view->home();
      return true;
    case Qt::Key_End:
      m_overwritten.clear();
      m_view->end();
      return true;
    default:
      return false;
      break;
    }
  } else if ( e->modifiers() == Qt::ControlModifier ) {
    switch( e->key() ) {
    case Qt::Key_BracketLeft:
    case Qt::Key_C:
      startNormalMode();
      return true;
      break;
    case Qt::Key_E:
      commandInsertFromLine( 1 );
      return true;
      break;
    case Qt::Key_Y:
      commandInsertFromLine( -1 );
      return true;
      break;
    case Qt::Key_Left:
      m_overwritten.clear();
      commandMoveOneWordLeft();
      return true;
      break;
    case Qt::Key_Right:
      m_overwritten.clear();
      commandMoveOneWordRight();
      return true;
      break;
    default:
      return false;
    }
  }

  return false;
}

void KateViReplaceMode::backspace()
{
  KTextEditor::Cursor c1( m_view->cursorPosition() );
  KTextEditor::Cursor c2( c1.line(), c1.column()-1 );

  if ( c1.column() > 0 ) {
    if ( !m_overwritten.isEmpty() ) {
      doc()->removeText(KTextEditor::Range(c1.line(), c1.column()-1, c1.line(), c1.column()));
      doc()->insertText(c2 , m_overwritten.right( 1 ) );
      m_overwritten.remove( m_overwritten.length()-1, 1);
    }
    updateCursor( c2 );
  }
}
