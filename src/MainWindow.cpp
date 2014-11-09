#include <QStatusBar>

#include "vi.h"
#include "MainWindow.h"
#include "ViEditView.h"
#include "ViEngine.h"
#include "KeymapService.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
  this->setWindowTitle(QObject::tr("Code Editor Example"));

  m_editor = new ViEditView;
  setCentralWidget(m_editor);

  m_viEngine = new ViEngine(m_editor);
  KeymapService::singleton().load("keymap.yml", m_viEngine);

  statusBar()->addWidget(m_cmdLineEdit = new QLineEdit(), 1);
  m_cmdLineEdit->installEventFilter(this);

  connect(m_viEngine, SIGNAL(modeChanged(Mode)), this, SLOT(onModeChanged(Mode)));
  connect(m_viEngine, SIGNAL(modeChanged(Mode)), m_editor, SLOT(setMode(Mode)));
  connect(m_cmdLineEdit, SIGNAL(returnPressed()), this, SLOT(cmdLineReturnPressed()));
  connect(m_cmdLineEdit,
          SIGNAL(cursorPositionChanged(int, int)),
          this,
          SLOT(cmdLineCursorPositionChanged(int, int)));
  connect(
      m_cmdLineEdit, SIGNAL(textChanged(QString)), this, SLOT(cmdLineTextChanged(const QString&)));

  onModeChanged(m_viEngine->mode());
}

MainWindow::~MainWindow() {
}

void MainWindow::onModeChanged(Mode mode) {
  QString text;
  switch (mode) {
    case Mode::CMD:
      text = "CMD";
      break;
    case Mode::INSERT:
      text = "INSERT";
      break;
    case Mode::CMDLINE:
      m_cmdLineEdit->setText(":");
      m_cmdLineEdit->show();
      m_cmdLineEdit->setFocus(Qt::OtherFocusReason);
      return;
  }

  m_cmdLineEdit->hide();
  statusBar()->showMessage(text);
}

void MainWindow::cmdLineReturnPressed() {
  QString text = m_cmdLineEdit->text();
  if (!text.isEmpty() && text[0] == ':') {
    m_viEngine->processExCommand(text.mid(1));
  }
}

void MainWindow::cmdLineCursorPositionChanged(int oldPos, int newPos) {
  if (newPos == 0) {
    m_cmdLineEdit->setCursorPosition(1);
  }
}

void MainWindow::cmdLineTextChanged(const QString& text) {
  if (text.isEmpty() || text[0] != ':') {
    m_viEngine->setMode(Mode::CMD);
  }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_cmdLineEdit && event->type() == QEvent::KeyPress &&
      m_viEngine->mode() == Mode::CMDLINE) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      m_viEngine->setMode(Mode::CMD);
      return true;
    }
  }

  return false;
}
