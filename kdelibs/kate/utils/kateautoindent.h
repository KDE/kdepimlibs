/* This file is part of the KDE libraries
   Copyright (C) 2003 Jesse Yurkovich <yurkjes@iit.edu>
   Copyright (C) 2004 >Anders Lund <anders@alweb.dk> (KateVarIndent class)
   Copyright (C) 2005 Dominik Haumann <dhdev@gmx.de> (basic support for config page)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KATE_AUTO_INDENT_H__
#define __KATE_AUTO_INDENT_H__

#include "kateconfig.h"

#include <QtCore/QObject>

#include <ktexteditor/cursor.h>
#include <kactionmenu.h>

class KateDocument;
class KateIndentScript;
class KateHighlighting;

/**
 * Provides Auto-Indent functionality for katepart.
 * This baseclass is a real dummy, does nothing beside remembering the document it belongs too,
 * only to have the object around
 */
class KateAutoIndent : public QObject
{
  Q_OBJECT
  /*
   * Static methods to list indention modes
   */
  public:
    /**
     * List all possible modes by name
     * @return list of modes
     */
    static QStringList listModes ();

    /**
     * Return the mode name given the mode
     * @param mode mode index
     * @return name for this mode index
     */
    static QString modeName (int mode);

    /**
     * Return the mode description
     * @param mode mode index
     * @return mode index
     */
    static QString modeDescription (int mode);

    /**
     * Return the syntax highlighting style required to use this mode
     * @param mode mode index
     * @return required style, or empty if the mode doesn't require any style
     */
    static QString modeRequiredStyle(int mode);

    /**
     * Maps name -> index
     * @param name mode name
     * @return mode index
     */
    static uint modeNumber (const QString &name);

    /**
     * count of modes
     * @return number of existing modes
     */
    static int modeCount ();

  /*
   * Construction + Destruction
   */
  public:
    /**
     * Constructor, creates dummy indenter "None"
     * \param doc parent document
     */
    explicit KateAutoIndent (KateDocument *doc);

    /**
     * Destructor
     */
    ~KateAutoIndent ();

  /*
   * Internal helper for the subclasses and itself
   */
  private:
    /**
     * Produces a string with the proper indentation characters for its length.
     *
     * @param length The length of the indention in characters.
     * @param align Length of alignment, ignored if less of equal to length
     * @return A QString representing @p length characters (factoring in tabs and spaces)
     */
    QString tabString (int length, int align) const;

    /**
     * Set the indent level of the line.
     * \param line line to change indent for
     * \param change set indentation to given number of spaces
     * \param align if align is higher than indentDepth, the difference
     * represents a number of spaces to be added after the indent
     */
    bool doIndent(int line, int indentDepth, int align = 0);

    /**
     * Change the indent of the specified line by the number of levels
     * specified by change. Positive values will indent more, negative values
     * will indent less.
     * \param line line to change indent for
     * \param change change the indentation by given number of spaces
     */
    bool doIndentRelative(int line, int change);

    /**
     * Reuse the indent of the previous line
     * \param line line to change indent for
     */
    void keepIndent ( int line );

    /**
     * Call the indentation script, this is a helper to be used in userTypedChar and indent
     * \param view the view the user work at
     * \param position current cursor position, after the inserted char...
     * \param typedChar the inserted char, indent will just give the script '\n'
     */
    void scriptIndent (KateView *view, const KTextEditor::Cursor &position, QChar typedChar);

    /**
     * Return true if the required style for the script is provided by the highlighter.
     */
    static bool isStyleProvided(const KateIndentScript *script, const KateHighlighting *highlight);

  public:
    /**
     * Switch indenter
     * Nop if already set to given mode
     * Otherwise switch to given indenter or to "None" if no suitable found...
     * @param name indention mode wanted
     */
    void setMode (const QString &name);

    /**
     * Check if the current highlighting mode provides the style required by the
     * current indenter. If not, deactivate the indenter by changing to "normal"
     * mode.
     */
    void checkRequiredStyle();

    /**
     * mode name
     */
    const QString &modeName () const { return m_mode; }

    /**
     * Update indenter's configuration (indention width, etc.)
     * Is called in the updateConfig() of the document and after creation of the indenter...
     */
    void updateConfig ();

    /**
     * Function to provide the common indent/unindent/clean indent functionality to the document
     * This should be generic for all indenters, internally it uses the doIndent function.
     * This works equal for all indenters, even for "none" or the scripts
     * \param range range of text to change indent for
     * \param change level of indents to add or remove, zero will still trigger cleaning of indentation
     * and removal of extra spaces, if option set
     * \return \e true on success, otherwise \e false
     */
    bool changeIndent (const KTextEditor::Range &range, int change);

    /**
     * The document requests the indenter to indent the given range of existing text.
     * This may happen to indent text pasted or to reindent existing text.
     * For "none" and "normal" this is a nop, for the scripts, the expression
     * will be asked for indent level for each line
     * \param view the view the user work at
     * \param range the range of text to indent...
     */
    void indent (KateView *view, const KTextEditor::Range &range);

    /**
     * The user typed some char, the indenter can react on this
     * '\n' will be send as char if the user wraps a line
     * \param view the view the user work at
     * \param position current cursor position, after the inserted char...
     * \param typedChar the inserted char
     */
    void userTypedChar (KateView *view, const KTextEditor::Cursor &position, QChar typedChar);

  public Q_SLOTS:
    void reloadScript();

  /*
   * needed data
   */
  private:
    KateDocument *doc; //!< the document the indenter works on
    int  tabWidth;     //!< The number of characters simulated for a tab
    int  indentWidth;  //!< The number of characters used when tabs are replaced by spaces
    bool  useSpaces;    //!< Should we use spaces or tabs to indent
    bool  keepExtra;    //!< Keep indentation that is not on indentation boundaries
    QString m_mode;
    KateIndentScript *m_script;
};

/**
 * This action provides a list of available indenters and gets plugged
 * into the KateView's KActionCollection.
 */
class KateViewIndentationAction : public KActionMenu
{
  Q_OBJECT

  public:
    KateViewIndentationAction(KateDocument *_doc, const QString& text, QObject *parent);

  private:
    KateDocument* doc;
    QActionGroup *actionGroup;

  public  Q_SLOTS:
    void slotAboutToShow();

  private Q_SLOTS:
    void setMode (QAction*);
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
