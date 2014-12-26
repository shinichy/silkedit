#include <stextedit.h>

#include "Document.h"

Document::Document(const QString& path, const QString& text) : QTextDocument(text), m_path(path), m_syntaxHighlighter(nullptr) {
  setupLayout();

  int dotPos = path.lastIndexOf('.');
  if (dotPos >= 0) {
    QString ext = path.mid(dotPos + 1);
    qDebug("ext: %s", qPrintable(ext));
    setupSyntaxHighlighter(LanguageProvider::languageFromExtension(ext), text);
  } else {
    qDebug("extension not found. path: %s", qPrintable(path));
  }
}

Document::Document() {
  setupLayout();
  setupSyntaxHighlighter(LanguageProvider::defaultLanguage());
}

void Document::setupLayout() {
  STextDocumentLayout* layout = new STextDocumentLayout(this);
  setDocumentLayout(layout);
}

void Document::setupSyntaxHighlighter(Language* lang, const QString& text) {
  m_lang.reset(lang);
  if (m_lang) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, text);
    m_syntaxHighlighter = new SyntaxHighlighter(this, parser);
  } else {
    qDebug("lang is null");
  }
}

Document::~Document() { qDebug("~Document"); }

Document* Document::create(const QString& path) {
  qDebug("Docment::create(%s)", qPrintable(path));
  QFile file(path);
  if (!file.open(QIODevice::ReadWrite))
    return nullptr;

  QTextStream in(&file);
  return new Document(path, in.readAll());
}

Document* Document::createBlank() { return new Document(); }

bool Document::setLanguage(const QString& scopeName) {
  qDebug("setLanguage: %s", qPrintable(scopeName));
  Language* newLang = LanguageProvider::languageFromScope(scopeName);
  if (m_lang.get() == newLang || (m_lang && newLang && *m_lang == *newLang)) {
    qDebug("lang is already %s", qPrintable(scopeName));
    return false;
  }

  m_lang.reset(newLang);
  if (m_lang && m_syntaxHighlighter) {
    LanguageParser* parser = LanguageParser::create(m_lang->scopeName, toPlainText());
    m_syntaxHighlighter->setParser(parser);
    if (m_syntaxHighlighter) {
      m_syntaxHighlighter->rehighlight();
    }
  }
  return true;
}