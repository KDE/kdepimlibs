/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2005 Hamish Rodda <rodda@kde.org>
 *  Copyright (C) 2008 Dominik Haumann <dhaumann kde org>
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

#ifndef KATELAYOUTCACHE_H
#define KATELAYOUTCACHE_H

#include <QThreadStorage>
#include <QPair>

#include <ktexteditor/range.h>

#include "katetextlayout.h"

class KateRenderer;
class KateEditInfo;

namespace KTextEditor { class Document; }

class KateLineLayoutMap
{
  public:
    KateLineLayoutMap();
    ~KateLineLayoutMap();

    inline void clear();

    inline bool contains(int i) const;

    inline void insert(int realLine, const KateLineLayoutPtr& lineLayoutPtr);

    inline void viewWidthIncreased();
    inline void viewWidthDecreased(int newWidth);

    inline void relayoutLines(int startRealLine, int endRealLine);

    inline void slotEditDone(int fromLine, int toLine, int shiftAmount);

    KateLineLayoutPtr& operator[](int i);

    typedef QPair<int, KateLineLayoutPtr> LineLayoutPair;
  private:
    typedef QVector<LineLayoutPair> LineLayoutMap;
    LineLayoutMap m_lineLayouts;
};

/**
 * This class handles Kate's caching of layouting information (in KateLineLayout
 * and KateTextLayout).  This information is used primarily by both the view and
 * the renderer.
 *
 * We outsource the hardcore layouting logic to the renderer, but other than
 * that, this class handles all manipulation of the layout objects.
 *
 * This is separate from the renderer 1) for clarity 2) so you can have separate
 * caches for separate views of the same document, even for view and printer
 * (if the renderer is made to support rendering onto different targets).
 *
 * @warning The smart-mutex must be locked whenever this is used
 *
 * @author Hamish Rodda \<rodda@kde.org\>
 */

class KateLayoutCache : public QObject
{
  Q_OBJECT

  public:
    explicit KateLayoutCache(KateRenderer* renderer, QObject* parent);

    void clear();

    int viewWidth() const;
    void setViewWidth(int width);

    bool wrap() const;
    void setWrap(bool wrap);

    bool acceptDirtyLayouts();
    void setAcceptDirtyLayouts(bool accept);

    // BEGIN generic methods to get/set layouts
    /**
     * Returns the KateLineLayout for the specified line.
     *
     * If one does not exist, it will be created and laid out.
     * Layouts which are not directly part of the view will be kept until the
     * cache is full or until they are invalidated by other means (eg. the text
     * changes).
     *
     * \param realLine real line number of the layout to retrieve.
     * \param virtualLine virtual line number. only needed if you think it may have changed
     *                    (ie. basically internal to KateLayoutCache)
     */
    KateLineLayoutPtr line(int realLine, int virtualLine = -1);
    /// \overload
    KateLineLayoutPtr line(const KTextEditor::Cursor& realCursor);

    /// Returns the layout describing the text line which is occupied by \p realCursor.
    KateTextLayout textLayout(const KTextEditor::Cursor& realCursor);

    /// Returns the layout of the specified realLine + viewLine.
    /// if viewLine is -1, return the last.
    KateTextLayout textLayout(uint realLine, int viewLine);
    // END

    // BEGIN methods to do with the caching of lines visible within a view
    /// Returns the layout of the corresponding line in the view
    KateTextLayout& viewLine(int viewLine);

    // find the view line of the cursor, relative to the display (0 = top line of view, 1 = second line, etc.)
    // if limitToVisible is true, only lines which are currently visible will be searched for, and -1 returned if the line is not visible.
    int displayViewLine(const KTextEditor::Cursor& virtualCursor, bool limitToVisible = false);

    int viewCacheLineCount() const;
    KTextEditor::Cursor viewCacheStart() const;
    KTextEditor::Cursor viewCacheEnd() const;
    void updateViewCache(const KTextEditor::Cursor& startPos, int newViewLineCount = -1, int viewLinesScrolled = 0);

    void relayoutLines(int startRealLine, int endRealLine);

    // find the index of the last view line for a specific line
    int lastViewLine(int realLine);
    // find the view line of cursor c (0 = same line, 1 = down one, etc.)
    int viewLine(const KTextEditor::Cursor& realCursor);
    int viewLineCount(int realLine);

    void viewCacheDebugOutput() const;
    // END

private Q_SLOTS:
    void slotEditDone(KateEditInfo* edit);

private:
    KateRenderer* m_renderer;

    /**
     * The master cache of all line layouts.
     *
     * Layouts which are not within the current view cache and whose
     * refcount == 1 are only known to the cache and can be safely deleted.
     */
    mutable KateLineLayoutMap m_lineLayouts;

    // Convenience vector for quick direct access to the specific text layout
    KTextEditor::Cursor m_startPos;
    mutable QVector<KateTextLayout> m_textLayouts;

    int m_viewWidth;
    bool m_wrap;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
