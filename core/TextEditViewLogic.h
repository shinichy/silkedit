﻿#pragma once

#include <boost/optional.hpp>
#include <QString>
#include <QTextDocument>
#include <QTextCursor>

#include "macros.h"

namespace core {

class Regexp;
class Metadata;

/**
 * @brief This class has business logics for TextEditView
 * Created mainly for unit testing
 */
class TextEditViewLogic {
  DISABLE_COPY_AND_MOVE(TextEditViewLogic)
 public:
  static void outdent(QTextDocument* doc, QTextCursor& cursor, int tabWidth);
  static bool isOutdentNecessary(Regexp* increaseIndentPattern,
                                 Regexp* decreaseIndentPattern,
                                 const QString& currentLineText,
                                 const QString& prevLineText,
                                 bool isAtBlockEnd,
                                 int tabWidth);
  static void indentOneLevel(QTextCursor& currentVisibleCursor,
                             bool indentUsingSpaces,
                             int tabWidth);
  static void indentCurrentLine(QTextDocument* doc,
                                QTextCursor& cursor,
                                const QString& prevLineText,
                                const boost::optional<QString>& prevPrevLineText,
                                Metadata* metadata,
                                bool indentUsingSpaces,
                                int tabWidth);

 private:
  TextEditViewLogic() = delete;
  ~TextEditViewLogic() = delete;
};

}  // namespace core
