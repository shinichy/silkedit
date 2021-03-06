#pragma once

#include <QStatusBar>

#include "core/macros.h"

class TextEdit;
class LanguageComboBox;
class EncodingComboBox;
class LineSeparatorComboBox;
class BOMComboBox;
class QMainWindow;
namespace core {
struct Language;
class Encoding;
class BOM;
class Theme;
}

class StatusBar : public QStatusBar{
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  explicit StatusBar(QMainWindow* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void onActiveViewChanged(QWidget *oldEditView, QWidget *newEditView);
  void setLanguage(const QString& scope);
  void setEncoding(const core::Encoding& encoding);
  void setLineSeparator(const QString& separator);
  void setBOM(const core::BOM& bom);
  void setActiveTextEditLanguage();
  void setActiveTextEditEncoding();
  void setActiveTextEditLineSeparator();
  void setActiveTextEditBOM();

 signals:
  void languageChanged(const QString& scopeName);

 private:
  LanguageComboBox* m_langComboBox;
  LineSeparatorComboBox* m_separatorComboBox;
  EncodingComboBox* m_encComboBox;
  BOMComboBox* m_bomComboBox;

  void setTheme(const core::Theme* theme);
  void setCurrentLanguage(const QString &langName);
};

Q_DECLARE_METATYPE(StatusBar*)
