#include <QtGui>
#include "viEngine.h"
#include "viEditView.h"

ViEngine::ViEngine(QObject *parent)
    : m_mode(CMD), m_editor(nullptr), QObject(parent) {}

ViEngine::~ViEngine() {}

void ViEngine::setEditor(ViEditView *editor) {
  m_editor = editor;
#if USE_EVENT_FILTER
  m_editor->installEventFilter(this);
#endif
}

void ViEngine::setMode(Mode mode) {
  if (mode != m_mode) {
    m_mode = mode;
    emit modeChanged(mode);
  }
}

#if 1
bool ViEngine::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_editor && event->type() == QEvent::KeyPress) {
    switch (mode()) {
    case CMD:
      cmdModeKeyPressEvent(static_cast<QKeyEvent *>(event));
      return true;
    case INSERT:
      return insertModeKeyPressEvent(static_cast<QKeyEvent *>(event));
    }
  }
  return false;
}
#else
bool ViEngine::processKeyPressEvent(QKeyEvent *event) {
  switch (mode()) {
  case CMD:
    return cmdModeKeyPressEvent(event);
  case INSERT:
    return insertModeKeyPressEvent(event);
  }
  return false;
}
#endif

bool ViEngine::cmdModeKeyPressEvent(QKeyEvent *event) {
  QString text = event->text();
  if (text == "i") {
    setMode(INSERT);
    return true;
  }
  return false;
}

bool ViEngine::insertModeKeyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    setMode(CMD);
    return true;
  }
  return false;
}
