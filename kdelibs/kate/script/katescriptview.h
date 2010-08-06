/// This file is part of the KDE libraries
/// Copyright (C) 2008 Paul Giannaros <paul@giannaros.org>
/// Copyright (C) 2008 Christoph Cullmann <cullmann@kde.org>
///
/// This library is free software; you can redistribute it and/or
/// modify it under the terms of the GNU Library General Public
/// License as published by the Free Software Foundation; either
/// version 2 of the License, or (at your option) version 3.
///
/// This library is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// Library General Public License for more details.
///
/// You should have received a copy of the GNU Library General Public License
/// along with this library; see the file COPYING.LIB.  If not, write to
/// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
/// Boston, MA 02110-1301, USA.

#ifndef KATE_SCRIPT_VIEW_H
#define KATE_SCRIPT_VIEW_H

#include <QObject>
#include <QtScript/QScriptable>

#include "katepartprivate_export.h"

#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

class KateView;

/**
 * Thinish wrapping around KateView, exposing the methods we want exposed
 * and adding some helper methods.
 *
 * We inherit from QScriptable to have more thight access to the scripting
 * engine.
 *
 * setView _must_ be called before using any other method. This is not checked
 * for the sake of speed.
 */
class KATEPART_TESTS_EXPORT KateScriptView : public QObject, protected QScriptable
{
  /// Properties are accessible with a nicer syntax from JavaScript
  Q_OBJECT

  public:
    KateScriptView (QObject *parent=0);
    void setView (KateView *view);
    KateView *view();

    Q_INVOKABLE KTextEditor::Cursor cursorPosition ();
    /**
     * Set the cursor position in the view.
     * @since 4.4
     */
    Q_INVOKABLE void setCursorPosition(int line, int column);
    Q_INVOKABLE void setCursorPosition(const KTextEditor::Cursor& cursor);


    Q_INVOKABLE KTextEditor::Cursor virtualCursorPosition();
    Q_INVOKABLE void setVirtualCursorPosition(int line, int column);
    Q_INVOKABLE void setVirtualCursorPosition(const KTextEditor::Cursor& cursor);

    Q_INVOKABLE QString selectedText();
    Q_INVOKABLE bool hasSelection();
    Q_INVOKABLE KTextEditor::Range selection();
    Q_INVOKABLE void setSelection(const KTextEditor::Range& range);
    Q_INVOKABLE void removeSelectedText();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();

  private:
    KateView *m_view;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
