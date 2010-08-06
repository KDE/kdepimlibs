/** kate-script
 * name: Ruby
 * license: LGPL
 * author: Robin Pedersen <robinpeder@gmail.com>
 * revision: 1
 * kate-version: 3.4
 * type: indentation
 * required-syntax-style: ruby
 * indent-languages: ruby
 * priority: 0
 *
 * This file is part of the Kate Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

//BEGIN USER CONFIGURATION
//END USER CONFIGURATION


// specifies the characters which should trigger indent, beside the default '\n'
var triggerCharacters = "cdefhilnrsuw}]";

// Indent after lines that match this regexp
var rxIndent = /^\s*(def|if|unless|for|while|until|class|module|else|elsif|case|when|begin|rescue|ensure|catch)\b/;

// Unindent lines that match this regexp
var rxUnindent = /^\s*((end|when|else|elsif|rescue|ensure)\b|[\]\}])(.*)$/;

function assert(cond)
{
  if (!cond)
    throw "assertion failure";
}

// Return the closest non-empty line, ignoring comments
// (result <= line). Return -1 if the document
function findPrevNonCommentLine(line)
{
  line = document.prevNonEmptyLine(line);
  while (line >= 0 && document.isComment(line, document.firstColumn(line))) {
    line = document.prevNonEmptyLine(line - 1);
  }
  return line;
}

function isLineContinuing(line)
{
  return /\\$/.test(document.line(line));
}

// Return true if the given column is at least equal to the column that
// contains the last non-whitespace character at the given line, or if
// the rest of the line is a comment.
function isLastCodeColumn(line, column)
{
  if (column >= document.lastColumn(line))
    return true;
  else if (document.isComment(line, document.nextNonSpaceColumn(line, column+1)))
    return true;
  else
    return false;
}

// Look for a pattern at the end of the statement.
//
// Returns true if the pattern is found, in a position
// that is not inside a string or a comment, and the position +
// the length of the matching part is either the end of the
// statement, or a comment.
//
// The regexp must be global, and the search is continued until
// a match is found, or the end of the string is reached.
function testAtEnd(stmt, rx)
{
  assert(rx.global);

  var cnt = stmt.content();
  var res;
  while (res = rx.exec(cnt)) {
    var start = res.index;
    var end = rx.lastIndex;
    if (stmt.isCode(start)) {
      if (end == cnt.length)
        return true;
      if (stmt.isComment(end))
        return true;
    }
  }
  return false;
}

function isStmtContinuing(line)
{
  // Is there an open parenthesis?
  var anch = lastAnchor(line+1, 0);
  if (anch.line >= 0)
    return true;

  var stmt = new Statement(line, line);
  var rx = /((\+|\-|\*|\/|\=|&&|\|\||\band\b|\bor\b|,)\s*)/g;

  return testAtEnd(stmt, rx);
}

// Return the first line that is not preceded by a "continuing" line.
// Return currLine if currLine <= 0
function findStmtStart(currLine)
{
  var p, l = currLine;
  do {
    if (l <= 0) return l;
    p = l;
    l = findPrevNonCommentLine(l - 1);
  } while ((l == p-1 && isLineContinuing(l)) || isStmtContinuing(l));
  return p;
}

// Statement class
function Statement(start, end)
{
  this.start = start;
  this.end = end;

  // Convert to string for debugging
  this.toString = function() {
    return "{" + this.start + "," + this.end + "}";
  }
  
  // Return an object having 'line' and 'column' set to the given offset in a statement
  this.offsetToCursor = function(offset) {
    // TODO Provide helper function for this when API is converted to using cursors:
    var line = this.start;
    while (line < this.end && document.lineLength(line) < offset) {
      offset -= document.lineLength(line++) + 1;
    }
    return {line: line, column: offset};
  }

  // Return document.attribute at the given offset in a statement
  this.attribute = function(offset) {
    var cur = this.offsetToCursor(offset);
    return document.attribute(cur.line, cur.column);
  }

  // Return document.isCode at the given offset in a statement
  this.isCode = function(offset) {
    var cur = this.offsetToCursor(offset);
    return document.isCode(cur.line, cur.column);
  }

  // Return document.isComment at the given offset in a statement
  this.isComment = function(offset) {
    var cur = this.offsetToCursor(offset);
    return document.isComment(cur.line, cur.column);
  }

  // Return the indent at the beginning of the statement
  this.indent = function() {
    return document.firstVirtualColumn(this.start)
  }

  // Return the content of the statement from the document
  this.content = function() {
    var cnt = "";
    for (var l = this.start; l <= this.end; l++) {
      cnt += document.line(l).replace(/\\$/, " ");
      if (l < this.end)
        cnt += " ";
    }
    return cnt;
  }
}

// Returns a tuple that contains the first and last line of the
// previous statement before line.
function findPrevStmt(line)
{
  var stmtEnd = findPrevNonCommentLine(line);
  var stmtStart = findStmtStart(stmtEnd);

  return new Statement(stmtStart, stmtEnd);
}

function isBlockStart(stmt)
{
  var cnt = stmt.content();
  var len = cnt.length;

  if (rxIndent.test(cnt))
    return true;

  var rx = /((\bdo\b|\{)(\s*\|.*\|)?\s*)/g;

  return testAtEnd(stmt, rx);
}

function isBlockEnd(stmt)
{
  var cnt = stmt.content();

  return rxUnindent.test(cnt);
}

function findBlockStart(line)
{
  var nested = 0;
  var stmt = new Statement(line, line);
  while (true) {
    if (stmt.start < 0) return stmt;

    stmt = findPrevStmt(stmt.start - 1);
    if (isBlockEnd(stmt)) {
      nested++;
    }
    if (isBlockStart(stmt)) {
      if (nested == 0)
        return stmt;
      else
        nested--;
    }
  }
}

// check if the trigger characters are in the right context,
// otherwise running the indenter might be annoying to the user
function isValidTrigger(line, ch)
{
  if (ch == "" || ch == "\n")
    return true; // Explicit align or new line

  var res = rxUnindent.exec(document.line(line));
  if (res) {
    if (res[3] == "")
      return true; // Exact match
  }
  return false;
}

// Helper function to compare two KTextEditor::Cursor objects.
// Returns 0 if equal, 1 if ca > cb, -1 if ca < cb
function compare(ca, cb)
{
  if (ca.line != cb.line) {
    return (ca.line > cb.line) ? 1 : -1;
  } else if (ca.column != cb.column) {
    return (ca.column > cb.column) ? 1 : -1;
  } else {
    return 0;
  }
}

// Find the last open bracket before the current line.
// Result is a KTextEditor::Cursor object, with an extra attribute, 'ch'
// containing the type of bracket.
function lastAnchor(line, column)
{
  var anch = document.anchor(line, column, '(');
  anch.ch = '(';
  var tmp1 = document.anchor(line, column, '{');
  tmp1.ch = '{';
  var tmp2 = document.anchor(line, column, '[');
  tmp2.ch = '[';

  if (compare(tmp1, anch) == 1)
    anch = tmp1;
  if (compare(tmp2, anch) == 1)
    anch = tmp2;

  return anch;
}

// indent gets three arguments: line, indentWidth in spaces,
// typed character indent
function indent(line, indentWidth, ch)
{
  if (!isValidTrigger(line, ch))
    return -2;

  var prevStmt = findPrevStmt(line - 1);
  if (prevStmt.end < 0)
    return -2; // Can't indent the first line

  var prev = document.prevNonEmptyLine(line);

  // HACK Detect here documents
  if (document.isAttributeName(prev, document.lineLength(prev)-1, "Ruby:Here Document")) {
    return -1; // HERE-DOCUMENT
  }
  // HACK Detect embedded comments
  if (document.isAttributeName(prev, document.lineLength(prev)-1, "Ruby:Blockcomment")) {
    return -1;
  }

  var prevStmtCnt = prevStmt.content();
  var prevStmtInd = prevStmt.indent();

  // Are we inside a parameter list, array or hash?
  var anch = lastAnchor(line, 0);
  if (anch.line >= 0) {
    var shouldIndent = (anch.line == prevStmt.end) || testAtEnd(prevStmt, /,\s*/g);
    if (!isLastCodeColumn(anch.line, anch.column) || lastAnchor(anch.line, anch.column).line >= 0) {
      // TODO This is alignment, should force using spaces instead of tabs:
      if (shouldIndent) {
        anch.column += 1;
        var nextCol = document.nextNonSpaceColumn(anch.line, anch.column);
        if (nextCol > 0 && !document.isComment(anch.line, nextCol))
          anch.column = nextCol;
      }
      // Keep indent of previous statement, while aligning to the anchor column
      return [prevStmtInd, document.toVirtualColumn(anch.line, anch.column)];
    } else {
      return document.firstVirtualColumn(anch.line) + (shouldIndent ? indentWidth : 0);
    }
  }

  // Handle indenting of multiline statements.
  if ((prevStmt.end == line-1 && isLineContinuing(prevStmt.end)) || isStmtContinuing(prevStmt.end)) {
    if (prevStmt.start == prevStmt.end) {
      if (ch == '' && document.firstVirtualColumn(line) > document.firstVirtualColumn(prevStmt.end)) {
        return -2; // Don't force a specific indent level when aligning manually
      }
      return prevStmtInd + indentWidth * 2;
    } else {
      return document.firstVirtualColumn(prevStmt.end);
    }
  }

  if (rxUnindent.test(document.line(line))) {
    var startStmt = findBlockStart(line);
    if (startStmt.start >= 0)
      return startStmt.indent();
    else
      return -2;
  }

  if (isBlockStart(prevStmt)) {
    return prevStmtInd + indentWidth;
  } else if (prevStmtCnt.search(/[\[\{]\s*$/) != -1) {
    return prevStmtInd + indentWidth;
  }

  // Keep current
  return prevStmtInd;
}

// kate: indent-width 2; indent-spaces on;
