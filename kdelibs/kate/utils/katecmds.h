/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2003-2005 Anders Lund <anders@alweb.dk>
 *  Copyright (C) 2001-2010 Christoph Cullmann <cullmann@kde.org>
 *  Copyright (C) 2001 Charles Samuels <charles@kde.org>
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

#ifndef __KATE_CMDS_H__
#define __KATE_CMDS_H__

#include <ktexteditor/commandinterface.h>

#include <QtCore/QStringList>

class KateDocument;
class KCompletion;

/**
 * The KateCommands namespace collects subclasses of KTextEditor::Command
 * for specific use in kate.
 */
namespace KateCommands
{

/**
 * This KTextEditor::Command provides access to a lot of the core functionality
 * of kate part, settings, utilities, navigation etc.
 * it needs to get a kateview pointer, it will cast the kate::view pointer
 * hard to kateview
 */
class CoreCommands : public KTextEditor::Command, public KTextEditor::CommandExtension,
  public KTextEditor::RangeCommand
{
  public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec( class KTextEditor::View *view, const QString &cmd, QString &errorMsg );

    /**
     * execute command on given range
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @param rangeStart first line in range
     * @param rangeEnd last line in range
     * @return success
     */
    bool exec( class KTextEditor::View *view, const QString &cmd, QString &errorMsg,
        const KTextEditor::Range &range = KTextEditor::Range(-1, -0, -1, 0));

    bool supportsRange(const QString &range);

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help( class KTextEditor::View *, const QString &, QString & ) {return false;}

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    const QStringList &cmds();

    /**
    * override completionObject from interfaces/document.h .
    */
    KCompletion *completionObject( KTextEditor::View *, const QString & );

    virtual void flagCompletions( QStringList& ) {}
    virtual bool wantsToProcessText( const QString & ) { return false; }
    virtual void processText( KTextEditor::View *, const QString & ) {}
};

/**
 * This KTextEditor::Command provides vi 'ex' commands
 */
class ViCommands : public KTextEditor::Command, public KTextEditor::CommandExtension,
  public KTextEditor::RangeCommand
{
  public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param msg message returned from running the command
     * @return success
     */
    bool exec( class KTextEditor::View *view, const QString &cmd, QString &msg );

    /**
     * execute command on given range
     * @param view view to use for execution
     * @param cmd cmd string
     * @param msg message returned from running the command
     * @param rangeStart first line in range
     * @param rangeEnd last line in range
     * @return success
     */
    bool exec( class KTextEditor::View *view, const QString &cmd, QString &msg,
        const KTextEditor::Range &range = KTextEditor::Range(-1, -0, -1, 0));

    bool supportsRange(const QString &range);

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help( class KTextEditor::View *, const QString &, QString & ) {return false;}

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    const QStringList &cmds();

    /**
    * override completionObject from interfaces/document.h .
    */
    KCompletion *completionObject( KTextEditor::View *, const QString & );

    virtual void flagCompletions( QStringList& ) {}
    virtual bool wantsToProcessText( const QString & ) { return false; }
    virtual void processText( KTextEditor::View *, const QString & ) {}
};


/**
 * Support vim/sed style search and replace
 * @author Charles Samuels <charles@kde.org>
 **/
class SedReplace : public KTextEditor::Command, public KTextEditor::RangeCommand
{
  public:
    /**
     * Execute command. Valid command strings are:
     *   -  s/search/replace/  find @c search, replace it with @c replace
     *                         on this line
     *   -  \%s/search/replace/ do the same to the whole file
     *   -  s/search/replace/i do the search and replace case insensitively
     *   -  $s/search/replace/ do the search are replacement to the
     *                         selection only
     *
     * @note   $s/// is currently unsupported
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec (class KTextEditor::View *view, const QString &cmd, QString &errorMsg);

    bool exec (class KTextEditor::View *view, const QString &cmd, QString &errorMsg,
        const KTextEditor::Range &r);

    bool supportsRange(const QString &) { return true; }

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help (class KTextEditor::View *, const QString &, QString &) { return false; }

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    const QStringList &cmds () { static QStringList l("s"); if (l.isEmpty()) l << "%s" << "$s"; return l; }

  private:
    /**
     * Searches one line and does the replacement in the document.
     * If @p replace contains any newline characters, the reamaining part of the
     * line is searched, and the @p line set to the last line number searched.
     * @return the number of replacements performed.
     * @param doc a pointer to the document to work on
     * @param line the number of the line to search. This may be changed by the
     * function, if newlines are inserted.
     * @param find A regular expression pattern to use for searching
     * @param replace a template for replacement. Backspaced integers are
     * replaced with captured texts from the regular expression.
     * @param delim the delimiter character from the command. In the replacement
     * text backsplashes preceding this character are removed.
     * @param nocase parameter for matching the reqular expression.
     * @param repeat If false, the search is stopped after the first match.
     * @param startcol The position in the line to start the search.
     * @param endcol The last column in the line allowed in a match.
     * If it is -1, the whole line is used.
     */
    static int sedMagic(KateDocument *doc, int &line,
                        const QString &find, const QString &replace, const QString &delim,
                        bool noCase, bool repeat,
                        int startcol=0, int endcol=-1);
};

/**
 * insert a unicode or ascii character
 * base 9+1: 1234
 * hex: 0x1234 or x1234
 * octal: 01231
 *
 * prefixed with "char:"
 **/
class Character : public KTextEditor::Command
{
  public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec (class KTextEditor::View *view, const QString &cmd, QString &errorMsg);

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help (class KTextEditor::View *, const QString &, QString &) { return false; }

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    const QStringList &cmds () { static QStringList test("char"); return test; }
};

/**
 * insert the current date/time in the given format
 */
class Date : public KTextEditor::Command
{
  public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec (class KTextEditor::View *view, const QString &cmd, QString &errorMsg);

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help (class KTextEditor::View *, const QString &, QString &) { return false; }

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    const QStringList &cmds () { static QStringList test("date"); return test; }
};


} // namespace KateCommands
#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
