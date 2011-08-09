/* OctaveGUI - A graphical user interface for Octave
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This module is a custom lexer for QScintilla for use with the Octave-GUI.
 * Examples for lexers can be found in the sources of Qscintilla-gpl-2.5.1,
 * QScintilla is a port to Qt of Neil Hodgson's Scintilla C++ editor control.
 */


#ifndef LEXEROCTAVE_H
#define LEXEROCTAVE_H

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>


class QSCINTILLA_EXPORT LexerOctaveGui : public QsciLexer
{
    Q_OBJECT

public:
    // the used styles
    enum
      {
        Default = 0,
        Comment = 1,
        Command = 2,
        Number = 3,
        Keyword = 4,
        SingleQuotedString = 5,
        Operator = 6,
        Identifier = 7,
        DoubleQuotedString = 8
      };

    LexerOctaveGui(QObject *parent = 0);
    virtual ~LexerOctaveGui();
    const char *language() const;
    const char *lexer() const;
    QColor defaultColor(int style) const;
    QFont defaultFont(int style) const;
    const char *keywords(int set) const;
    QString description(int style) const;

private:
    LexerOctaveGui(const LexerOctaveGui &);
    LexerOctaveGui &operator=(const LexerOctaveGui &);
};

#endif
