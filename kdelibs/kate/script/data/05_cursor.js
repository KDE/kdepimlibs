// This file is part of the Kate project within KDE.
// (C) 2009 Dominik Haumann <dhaumann kde org>
// License: LGPL v2 or v3

/**
 * Prototype Cursor.
 *
 * \section cursor_intro Introduction
 * The Cursor class provides the two properties \p line and \p column. Since a
 * lot of text operations are based on lines and columns such as inserting text
 * at a cursor position, the Cursor class plays a central role in KatePart
 * scripting. The entire scripting API is usually based on cursors whenever
 * possible.
 *
 * \section cursor_usage Using Cursors
 * There are several ways to construct a Cursor:
 * \code
 * var cursor1 = new Cursor(); // constructs a (valid) cursor at position (0, 0)
 * var cursor2 = new Cursor(2, 4); // constructs a cursor at position (2, 4)
 * var cursor3 = new Cursor(cursor2); // copies the cursor2
 * var cursor4 = new Cursor().invalid(); // constructs invalid cursor at (-1, -1)
 * \endcode
 *
 * There are several convenience member functions that easy working with
 * Cursors. Use isValid() to check whether a Cursor is a valid text cursor.
 * To compare two cursors either use equals() or compareTo().
 *
 * \see Range
 */
function Cursor()
{
  if (arguments.length == 0) {
    return new Cursor(0, 0);
  } else if (arguments.length == 1 && typeof arguments[0] == "object") {
    // assume: cursor = new Cursor(otherCursor);
    return arguments[0].clone();
  } else if (arguments.length == 2 && typeof arguments[0] == "number"
                                   && typeof arguments[1] == "number") {
    // assume: cursor = new Cursor(line, column);
    this.line = parseInt(arguments[0]);
    this.column = parseInt(arguments[1]);
  } else {
    throw "Wrong usage of Cursor constructor";
  }

  this.clone = function() {
    return new Cursor(this.line, this.column);
  };

  this.isValid = function() {
    return (this.line >= 0) && (this.column >= 0);
  };

  this.invalid = function() {
    return new Cursor(-1, -1);
  };

  this.compareTo = function(other) {
    if (this.line > other.line || (this.line == other.line && this.column > other.column)) {
      return 1;
    } else if (this.line < other.line || (this.line == other.line && this.column < other.column)) {
      return -1;
    } else {
      return 0;
    }
  };

  this.equals = function(other) {
    return (this.line == other.line && this.column == other.column);
  };

  this.toString = function() {
    return "Cursor(" + this.line+ ", " + this.column+ ")";
  };
}


