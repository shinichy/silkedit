#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeySequence>
#include <QSizePolicy>
#include <QCompleter>
#include <QCheckBox>
#include <QPainter>

#include "FindReplaceView.h"
#include "ui_FindReplaceView.h"
#include "App.h"
#include "TextEdit.h"
#include "LineEdit.h"
#include "core/Config.h"
#include "core/Theme.h"
#include "core/Util.h"

using core::Document;
using core::Region;
using core::Config;
using core::ColorSettings;
using core::Theme;
using core::Util;

namespace {
const char* PREVIOUS_MATCH_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Previous match");
const char* NEXT_MATCH_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Next match");
const char* MATCH_CASE_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Match Case");
const char* REGEX_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Regex");
const char* WHOLE_WORD_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Whole Word");
const char* PRESERVE_CASE_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "Preserve Case");
const char* IN_SELECTION_TEXT = QT_TRANSLATE_NOOP("FindReplaceView", "In Selection");
}

FindReplaceView::FindReplaceView(QWidget* parent)
    : CustomWidget(parent), ui(new Ui::FindReplaceView), m_activeView(nullptr) {
  ui->setupUi(this);

  // Make 'Replace' and 'Replace All' fonts 2 points smaller
  QFont font = ui->replaceButton->font();

  font.setPointSize(font.pointSize() - 2);
  ui->replaceButton->setFont(font);
  ui->replaceAllButton->setFont(font);
  ui->lineEditForFind->setAttribute(Qt::WA_MacShowFocusRect, 0);
  ui->lineEditForReplace->setAttribute(Qt::WA_MacShowFocusRect, 0);

  // The contents margin is the width of the reserved space along each of the QGridLayout's four
  // sides.
  ui->layout->setContentsMargins(3, 1, 0, 3);
  // The spacing() is the width of the automatically allocated spacing between neighboring boxes.
  ui->layout->setSpacing(0);

  QCompleter* searchCompleter = new QCompleter(this);
  searchCompleter->setModel(&m_searchHistoryModel);
  ui->lineEditForFind->setCompleter(searchCompleter);
  QCompleter* replaceCompleter = new QCompleter(this);
  replaceCompleter->setModel(&m_replaceHistoryModel);
  ui->lineEditForReplace->setCompleter(replaceCompleter);

  connect(ui->lineEditForFind, &LineEdit::returnPressed, this, &FindReplaceView::findNext);
  connect(ui->lineEditForFind, &LineEdit::shiftReturnPressed, this, &FindReplaceView::findPrevious);
  connect(ui->lineEditForFind, &LineEdit::textChanged, this, [=](const QString&) {
    int result = highlightMatches();
    selectFirstMatch();
    QPalette palette;
    if (!ui->lineEditForFind->text().isEmpty() && result == 0) {
      palette.setColor(QPalette::Text, Qt::red);
    } else {
      palette.setColor(QPalette::Text, Qt::black);
    }
    ui->lineEditForFind->setPalette(palette);
  });
  connect(ui->lineEditForFind, &LineEdit::focusIn, this, [=] {
    updateActiveCursorPos();
    highlightMatches();
  });
  connect(ui->replaceButton, &QPushButton::pressed, this, &FindReplaceView::replace);
  connect(ui->replaceAllButton, &QPushButton::pressed, this, &FindReplaceView::replaceAll);
  connect(ui->prevButton, &QPushButton::pressed, this, &FindReplaceView::findPrevious);
  connect(ui->nextButton, &QPushButton::pressed, this, &FindReplaceView::findNext);
  connect(ui->regexChk, &CheckBox::stateChanged, this, &FindReplaceView::highlightMatches);
  connect(ui->matchCaseChk, &CheckBox::stateChanged, this, &FindReplaceView::highlightMatches);
  connect(ui->wholeWordChk, &CheckBox::stateChanged, this, &FindReplaceView::highlightMatches);
  connect(ui->inSelectionChk, &CheckBox::stateChanged, this, [=] {
    updateSelectionRegion();
    highlightMatches();
  });

  connect(ui->preserveCaseChk, &CheckBox::stateChanged, this, &FindReplaceView::highlightMatches);
  connect(&Config::singleton(), &Config::themeChanged, this, &FindReplaceView::setTheme);

  // todo: configure mnemonic in keymap.yml using condition
  ui->prevButton->setToolTip(tr(PREVIOUS_MATCH_TEXT));
  ui->nextButton->setToolTip(tr(NEXT_MATCH_TEXT));
  ui->regexChk->setToolTip(tr(REGEX_TEXT));
  ui->matchCaseChk->setToolTip(tr(MATCH_CASE_TEXT));
  ui->wholeWordChk->setToolTip(tr(WHOLE_WORD_TEXT));
  ui->inSelectionChk->setToolTip(tr(IN_SELECTION_TEXT));
  ui->preserveCaseChk->setToolTip(tr(PRESERVE_CASE_TEXT));

  setLayout(ui->layout);
  setTheme(Config::singleton().theme());
}

FindReplaceView::~FindReplaceView() {}

void FindReplaceView::show() {
  if (isVisible()) {
    showEvent(nullptr);
  }
  QWidget::show();
}

void FindReplaceView::showEvent(QShowEvent*) {
  ui->inSelectionChk->setChecked(false);
  m_selectedRegion = boost::none;
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    QString selectedText = textEdit->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
      // If the selection obtained from an editor spans a line break, the text will contain a
      // Unicode U+2029 paragraph separator character instead of a newline \n character.
      if (selectedText.contains(QChar::ParagraphSeparator)) {
        ui->inSelectionChk->setChecked(true);
      } else {
        ui->lineEditForFind->setText(selectedText);
      }
    }
  }
  ui->lineEditForFind->setFocus();
  ui->lineEditForFind->selectAll();
}

// Override this method to cycle tabs within this view
bool FindReplaceView::focusNextPrevChild(bool next) {
  if (next && ui->replaceAllButton->hasFocus()) {
    ui->lineEditForFind->setFocus(Qt::TabFocusReason);
  } else if (!next && ui->lineEditForFind->hasFocus()) {
    ui->replaceAllButton->setFocus(Qt::TabFocusReason);
  } else {
    return QWidget::focusNextPrevChild(next);
  }

  return true;
}

void FindReplaceView::paintEvent(QPaintEvent*) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void FindReplaceView::findNext() {
  Q_ASSERT(ui->lineEditForFind);

  Document::FindFlags flags = getFindFlags();
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    int pos;
    auto cursor = textEdit->textCursor();
    if (m_selectedRegion) {
      // empty match case like /^/
      if (m_selectedRegion->isEmpty()) {
        if (flags.testFlag(Document::FindFlag::FindInSelection)) {
          if (m_selectedRegion->end() == m_selectionEndPos) {
            pos = m_selectionStartPos;
          } else {
            pos = m_selectedRegion->end() + 1;
          }
        } else {
          cursor.movePosition(QTextCursor::End);
          if (m_selectedRegion->end() == cursor.position()) {
            pos = 0;
          } else {
            pos = m_selectedRegion->end() + 1;
          }
        }
      } else {
        pos = m_selectedRegion->end();
      }
    } else {
      if (flags.testFlag(Document::FindFlag::FindInSelection)) {
        pos = m_selectionStartPos;
      } else {
        pos = cursor.position();
      }
    }

    findText(ui->lineEditForFind->text(), pos);
    m_searchHistoryModel.prepend(ui->lineEditForFind->text());
  }
}

void FindReplaceView::findPrevious() {
  Q_ASSERT(ui->lineEditForFind);

  Document::FindFlags flags = getFindFlags();
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    int pos;
    auto cursor = textEdit->textCursor();
    if (m_selectedRegion) {
      // empty match case like /^/
      if (m_selectedRegion->isEmpty()) {
        if (flags.testFlag(Document::FindFlag::FindInSelection)) {
          if (m_selectedRegion->begin() == m_selectionStartPos) {
            pos = m_selectionEndPos;
          } else {
            pos = m_selectedRegion->begin() - 1;
          }
        } else {
          cursor.movePosition(QTextCursor::End);
          if (m_selectedRegion->begin() == 0) {
            pos = cursor.position();
          } else {
            pos = m_selectedRegion->begin() - 1;
          }
        }
      } else {
        pos = m_selectedRegion->begin();
      }
    } else {
      if (flags.testFlag(Document::FindFlag::FindInSelection)) {
        pos = m_selectionEndPos;
      } else {
        pos = cursor.position();
      }
    }

    findText(ui->lineEditForFind->text(), pos, Document::FindFlag::FindBackward);
    m_searchHistoryModel.prepend(ui->lineEditForFind->text());
  }
}

void FindReplaceView::findFromActiveCursor() {
  findText(ui->lineEditForFind->text(), m_activeCursorPos);
  m_searchHistoryModel.prepend(ui->lineEditForFind->text());
}

void FindReplaceView::findText(const QString& text,
                               int searchStartPos,
                               Document::FindFlags otherFlags) {
  //  qDebug() << "searchStartPos" << searchStartPos;
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    Document::FindFlags flags = getFindFlags();
    flags |= otherFlags;
    int begin = 0, end = -1;
    if (flags.testFlag(Document::FindFlag::FindInSelection)) {
      qDebug("InSelection is on. begin: %d, end: %d", m_selectionStartPos, m_selectionEndPos);
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    m_selectedRegion = textEdit->find(text, searchStartPos, begin, end, flags);
  }
}

void FindReplaceView::findText(const QString& text, Document::FindFlags flags) {
  findText(text, -1, flags);
}

int FindReplaceView::highlightMatches() {
  int result = 0;

  if (TextEdit* textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    int begin = 0, end = -1;
    if (ui->inSelectionChk->isChecked()) {
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }

    result =
        textEdit->highlightSearchMatches(ui->lineEditForFind->text(), begin, end, getFindFlags());
  }

  return result;
}

void FindReplaceView::setActiveView(QWidget* view) {
  if (m_connectionForContentsChanged) {
    QObject::disconnect(m_connectionForContentsChanged);
  }
  if (m_connectionForCursorPositionChanged) {
    QObject::disconnect(m_connectionForCursorPositionChanged);
  }
  clearSearchHighlight();

  m_activeView = view;

  auto newTextEdit = qobject_cast<TextEdit*>(view);
  if (newTextEdit) {
    qDebug() << "new TextEdit:" << newTextEdit->document()->path();

    int begin = 0, end = -1;
    if (ui->inSelectionChk->isChecked()) {
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    m_connectionForContentsChanged =
        connect(newTextEdit->document(), &core::Document::contentsChanged, newTextEdit, [=] {
          if (newTextEdit->isSearchMatchesHighlighted()) {
            newTextEdit->highlightSearchMatches(ui->lineEditForFind->text(), begin, end,
                                                getFindFlags());
          }
        }, Qt::UniqueConnection);

    m_connectionForCursorPositionChanged =
        connect(newTextEdit, &TextEdit::cursorPositionChanged, newTextEdit,
                [=] { m_selectedRegion = boost::none; }, Qt::UniqueConnection);
  }

  if (isVisible()) {
    highlightMatches();
  }
}

void FindReplaceView::clearSearchHighlight() {
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    textEdit->clearSearchHighlight();
  }
}

Document::FindFlags FindReplaceView::getFindFlags() {
  Document::FindFlags flags;
  if (ui->regexChk->isChecked()) {
    flags |= Document::FindFlag::FindRegex;
  }
  if (ui->matchCaseChk->isChecked()) {
    flags |= Document::FindFlag::FindCaseSensitively;
  }
  if (ui->wholeWordChk->isChecked()) {
    flags |= Document::FindFlag::FindWholeWords;
  }
  if (ui->inSelectionChk->isChecked()) {
    flags |= Document::FindFlag::FindInSelection;
  }
  return flags;
}

void FindReplaceView::updateSelectionRegion() {
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    QTextCursor cursor = textEdit->textCursor();
    if (cursor.hasSelection()) {
      m_selectionStartPos = cursor.selectionStart();
      m_selectionEndPos = cursor.selectionEnd();
    } else {
      m_selectionStartPos = 0;
      m_selectionEndPos = -1;
    }
  }
}

void FindReplaceView::updateActiveCursorPos() {
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    m_activeCursorPos = textEdit->textCursor().selectionStart();
  }
}

void FindReplaceView::selectFirstMatch() {
  Q_ASSERT(ui->lineEditForFind);
  findText(ui->lineEditForFind->text(), m_activeCursorPos);
}

void FindReplaceView::replace() {
  Q_ASSERT(ui->lineEditForReplace);
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    textEdit->replaceSelection(ui->lineEditForReplace->text(), ui->preserveCaseChk->isChecked());
    highlightMatches();
    m_replaceHistoryModel.prepend(ui->lineEditForReplace->text());
  }
}

void FindReplaceView::replaceAll() {
  if (auto textEdit = qobject_cast<TextEdit*>(m_activeView)) {
    int begin = 0, end = -1;
    if (ui->inSelectionChk->isChecked()) {
      begin = m_selectionStartPos;
      end = m_selectionEndPos;
    }
    textEdit->replaceAllSelection(ui->lineEditForFind->text(), ui->lineEditForReplace->text(),
                                  begin, end, getFindFlags(), ui->preserveCaseChk->isChecked());
    m_replaceHistoryModel.prepend(ui->lineEditForReplace->text());
  }
}

void FindReplaceView::hide() {
  clearSearchHighlight();
  if (m_activeView) {
    m_activeView->setFocus();
  }
  QWidget::hide();
}

void FindReplaceView::setTheme(core::Theme* theme) {
  ui->prevButton->setIcon(QIcon(":/images/prevButton-black.svg"));
  ui->nextButton->setIcon(QIcon(":/images/nextButton-black.svg"));

  qDebug("FindReplaceView theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->findReplaceViewSettings != nullptr) {
    ColorSettings* findReplaceViewSettings = theme->findReplaceViewSettings.get();

    QString defaultIconColor = QStringLiteral("black");
    QString checkedIconColor = QStringLiteral("white");

    if (theme->isDarkTheme()) {
      defaultIconColor = "white";
      checkedIconColor = "black";
      ui->prevButton->setIcon(QIcon(":/images/prevButton-white.svg"));
      ui->nextButton->setIcon(QIcon(":/images/nextButton-white.svg"));
    }

    QList<QCheckBox*> allCkButtons = this->findChildren<QCheckBox*>();
    for (int i = 0; i < allCkButtons.size(); i++) {
      const QString& style = QStringLiteral(
                                 "#%1::indicator {"
                                 "image: url(:/images/%1-%2);"
                                 "}"
                                 "#%1::indicator:checked {"
                                 "image: url(:/images/%1-%3);"
                                 "}")
                                 .arg(allCkButtons.at(i)->objectName())
                                 .arg(defaultIconColor)
                                 .arg(checkedIconColor);
      allCkButtons.at(i)->setStyleSheet(style);
    };

    const QString& style =
        QStringLiteral(
            "#%1 {"
            "background-color: %2;"
            "}")
            .arg(this->objectName())
            .arg(Util::qcolorForStyleSheet(findReplaceViewSettings->value("background"))) +
        QStringLiteral(
            "#%1 QPushButton {"
            "color: %2;"
            "outline: 0;"
            "}"
            "#%1 QCheckBox:checked {"
            "background-color: %3;"
            "}")
            .arg(this->objectName())
            .arg(defaultIconColor)
            .arg(Util::qcolorForStyleSheet(
                findReplaceViewSettings->value("buttonCheckedBackgroundColor"))) +
        QStringLiteral(
            "#%1 QPushButton:focus, #%1 QLineEdit:focus {"
            "border-color: %2;"
            "}")
            .arg(this->objectName())
            .arg(Util::qcolorForStyleSheet(findReplaceViewSettings->value("focusColor"))) +
        QStringLiteral(
            "#%1 QPushButton:hover, "
            "#%1 QCheckBox:unchecked:hover {"
            "background:%2;"
            "}"
            "#%1 QPushButton:pressed, "
            "#%1 QCheckBox:unchecked:pressed {"
            "background:%3;"
            "}")
            .arg(this->objectName())
            .arg(Util::qcolorForStyleSheet(findReplaceViewSettings->value("hoverColor")))
            .arg(Util::qcolorForStyleSheet(findReplaceViewSettings->value("pressedColor")));

    this->setStyleSheet(style);
  }
}
