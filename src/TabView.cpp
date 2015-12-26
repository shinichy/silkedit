﻿#include <memory>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStylePainter>

#include "TabView.h"
#include "TextEditView.h"
#include "KeymapManager.h"
#include "TabBar.h"
#include "Window.h"
#include "DraggingTabInfo.h"
#include "SilkApp.h"
#include "DocumentManager.h"
#include "PluginManager.h"
#include "core/Config.h"
#include "core/Theme.h"
#include "core/Constants.h"

using core::Document;
using core::Config;
using core::Theme;
using core::ColorSettings;
using core::Constants;

namespace {
constexpr const char* PREFIX    = "tabInformation";
constexpr const char* PATH_KEY  = "tab";

QString getFileNameFrom(const QString& path) {
  QFileInfo info(path);
  return info.fileName();
}

// http://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color/3943023#3943023
bool isLightColor(const QColor& color) {
  return (color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114) > 186;
}
}

TabView::TabView(QWidget* parent)
    : QTabWidget(parent),
      m_activeEditView(nullptr),
      m_tabBar(new TabBar(this)),
      m_tabDragging(false) {
  setTabBar(m_tabBar);
  setMovable(true);
  setDocumentMode(true);
  setTabsClosable(true);
  changeTabStyle(Config::singleton().theme());
  // Note: setDocumentMode also calls setDrawBase
  tabBar()->setDrawBase(false);

  connect(m_tabBar, &TabBar::onDetachTabStarted, this, &TabView::detachTabStarted);
  connect(m_tabBar, &TabBar::onDetachTabEntered, this, &TabView::detachTabEntered);
  connect(m_tabBar, &TabBar::onDetachTabFinished, this, &TabView::detachTabFinished);
  connect(this, &QTabWidget::tabBarClicked, this, &TabView::focusTabContent);
  connect(this, &QTabWidget::currentChanged, this, &TabView::changeActiveEditView);
  connect(this, &QTabWidget::tabCloseRequested, this, &TabView::removeTabAndWidget);
  connect(&Config::singleton(), &Config::themeChanged, this, &TabView::changeTabStyle);
}

TabView::~TabView() {
  qDebug("~TabView");
}

int TabView::addTab(QWidget* page, const QString& label) {
  return insertTab(-1, page, label);
}

int TabView::insertTab(int index, QWidget* w, const QString& label) {
  if (!w) {
    return -1;
  }

  w->setParent(this);
  TextEditView* editView = qobject_cast<TextEditView*>(w);
  if (editView) {
    connect(editView, &TextEditView::pathUpdated, this, &TabView::changeTabText);
    connect(editView, &TextEditView::modificationChanged, this, &TabView::updateTabTextBasedOn);
  } else {
    qDebug("inserted widget is not TextEditView");
  }

  int result = QTabWidget::insertTab(index, w, label);

  if (count() == 1 && result >= 0) {
    m_activeEditView = editView;
    if (editView) {
      editView->setFocus();
    }
  }
  return result;
}

int TabView::open(const QString& path) {
  qDebug("TabView::open(%s)", qPrintable(path));
  int index = indexOfPath(path);
  if (index >= 0) {
    setCurrentIndex(index);
    return index;
  }

  std::shared_ptr<Document> newDoc(Document::create(path));
  if (!newDoc) {
    return -1;
  }
  newDoc->setModified(false);

  if (count() == 1) {
    TextEditView* editView = qobject_cast<TextEditView*>(currentWidget());
    if (editView && !editView->document()->isModified() && editView->document()->isEmpty()) {
      qDebug("trying to replace am empty doc with a new one");
      editView->setDocument(std::move(newDoc));
      editView->setPath(path);
      return currentIndex();
    }
  }

  TextEditView* view = new TextEditView(this);
  view->setDocument(std::move(newDoc));
  return addTab(view, getFileNameFrom(path));
}

void TabView::addNew() {
  TextEditView* view = new TextEditView(this);
  std::shared_ptr<Document> newDoc(Document::createBlank());
  view->setDocument(std::move(newDoc));
  addTab(view, DocumentManager::DEFAULT_FILE_NAME);
}

void TabView::saveAllTabs() {
  for (int i = 0; i < count(); i++) {
    auto editView = qobject_cast<TextEditView*>(widget(i));
    if (editView) {
      editView->save();
    }
  }
}

void TabView::closeActiveTab() {
  closeTab(currentWidget());
}

bool TabView::closeAllTabs() {
  if (count() == 0) {
    emit allTabRemoved();
  } else {
    std::list<QWidget*> widgets;
    for (int i = 0; i < count(); i++) {
      widgets.push_back(widget(i));
      insertTabInformation(i);
    }

    for (auto w : widgets) {
      bool isSuccess = closeTab(w);
      if (!isSuccess)
        return false;
    }
  }

  return true;
}

void TabView::closeOtherTabs() {
  std::list<QWidget*> widgets;
  for (int i = 0; i < count(); i++) {
    if (i != currentIndex()) {
      widgets.push_back(widget(i));
    }
  }

  for (auto w : widgets) {
    closeTab(w);
  }
}

int TabView::indexOfPath(const QString& path) {
  //  qDebug("TabView::indexOfPath(%s)", qPrintable(path));
  for (int i = 0; i < count(); i++) {
    TextEditView* v = qobject_cast<TextEditView*>(widget(i));
    QString path2 = v->path();
    if (v && !path2.isEmpty() && path == path2) {
      return i;
    }
  }

  return -1;
}

void TabView::detachTabStarted(int index, const QPoint&) {
  qDebug("DetachTabStarted");
  m_tabDragging = true;
  DraggingTabInfo::setWidget(widget(index));
  DraggingTabInfo::setTabText(tabText(index));
  removeTab(index);
  Q_ASSERT(DraggingTabInfo::widget());
}

void TabView::detachTabEntered(const QPoint& enterPoint) {
  qDebug("DetachTabEntered");
  qDebug() << "tabBar()->mapToGlobal(QPoint(0, 0)):" << tabBar()->mapToGlobal(QPoint(0, 0));
  QPoint relativeEnterPos = enterPoint - tabBar()->mapToGlobal(QPoint(0, 0));
  int index = tabBar()->tabAt(relativeEnterPos);
  int newIndex = insertTab(index, DraggingTabInfo::widget(), DraggingTabInfo::tabText());
  DraggingTabInfo::setWidget(nullptr);
  m_tabDragging = false;
  tabRemoved(-1);
  QPoint tabCenterPos = tabBar()->tabRect(newIndex).center();

  qDebug() << "tabCenterPos:" << tabCenterPos << "enterPoint:" << enterPoint
           << "relativeEnterPos:" << relativeEnterPos;
  m_tabBar->startMovingTab(tabCenterPos);
}

void TabView::tabInserted(int index) {
  setCurrentIndex(index);
  QTabWidget::tabInserted(index);
}

void TabView::tabRemoved(int) {
  if (count() == 0 && !m_tabDragging) {
    emit allTabRemoved();
  }
}

void TabView::mouseReleaseEvent(QMouseEvent* event) {
  qDebug("mouseReleaseEvent in TabView");
  QTabWidget::mouseReleaseEvent(event);
}

void TabView::changeActiveEditView(int index) {
  // This lambda is called after m_tabbar is deleted when shutdown.
  if (index < 0)
    return;

  qDebug("currentChanged. index: %i, tab count: %i", index, count());
  if (auto w = widget(index)) {
    TextEditView* editView = qobject_cast<TextEditView*>(w);
    setActiveEditView(editView);
  } else {
    qDebug("active edit view is null");
    setActiveEditView(nullptr);
  }
}

void TabView::setActiveEditView(TextEditView* editView) {
  if (m_activeEditView != editView) {
    TextEditView* oldEditView = m_activeEditView;
    m_activeEditView = editView;
    emit activeTextEditViewChanged(oldEditView, editView);
  }
}

void TabView::changeTabText(const QString& path) {
  if (TextEditView* editView = qobject_cast<TextEditView*>(QObject::sender())) {
    setTabText(indexOf(editView), getFileNameFrom(path));
  }
}

void TabView::changeTabStyle(Theme* theme) {
  if (theme) {
    ColorSettings* settings = theme->scopeSettings.first()->colorSettings.get();
    if (settings->contains("background")) {
      QColor color = settings->value("background");
      bool isLight = isLightColor(color);
      QString selectedTabTextColor = isLight ? "gray" : "lightGray";
      tabBar()->setStyleSheet(QString("QTabBar::tab:selected { background-color: %1; color: %2; } ")
                                  .arg(color.name())
                                  .arg(selectedTabTextColor));
    }
  }
}

void TabView::removeTabAndWidget(int index) {
  if (auto w = widget(index)) {
    if (w == m_activeEditView) {
      m_activeEditView = nullptr;
    }
    w->deleteLater();
  }
  removeTab(index);
}

bool TabView::closeTab(QWidget* w) {
  TextEditView* editView = qobject_cast<TextEditView*>(w);
  if (editView && editView->document()->isModified()) {
    QMessageBox msgBox;
    msgBox.setText(tr("Do you want to save the changes made to the document %1?")
                       .arg(getFileNameFrom(editView->path())));
    msgBox.setInformativeText(tr("Your changes will be lost if you don’t save them."));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIconPixmap(QIcon(":/app_icon_064.png").pixmap(64, 64));
    int ret = msgBox.exec();
    switch (ret) {
      case QMessageBox::Save:
        editView->save();
        break;
      case QMessageBox::Discard:
        break;
      case QMessageBox::Cancel:
        return false;
      default:
        qWarning("ret is invalid");
        return false;
    }
  } else {
    qDebug("widget is neither TextEditView nor modified");
  }

  removeTabAndWidget(indexOf(w));

  // Focus to the current widget after closing a tab
  if (QWidget* w = currentWidget()) {
    w->setFocus();
  }
  return true;
}

void TabView::focusTabContent(int index) {
  if (QWidget* w = widget(index)) {
    w->setFocus();
  }
}

void TabView::updateTabTextBasedOn(bool changed) {
  qDebug() << "updateTabTextBasedOn. changed:" << changed;
  if (QWidget* w = qobject_cast<QWidget*>(QObject::sender())) {
    int index = indexOf(w);
    QString text = tabText(index);
    if (changed) {
      setTabText(index, text + "*");
    } else if (text.endsWith('*')) {
      text.chop(1);
      setTabText(index, text);
    }
  } else {
    qDebug("sender is null or not QWidget");
  }
}

void TabView::detachTabFinished(const QPoint& newWindowPos, bool isFloating) {
  qDebug() << "DetachTab."
           << "newWindowPos:" << newWindowPos << "isFloating:" << isFloating;

  if (isFloating) {
    Window* newWindow = Window::create();
    newWindow->move(newWindowPos);
    newWindow->show();
    if (DraggingTabInfo::widget()) {
      newWindow->activeTabView()->addTab(DraggingTabInfo::widget(), DraggingTabInfo::tabText());
      DraggingTabInfo::setWidget(nullptr);
    } else {
      qWarning("dragging widget is null");
    }
  }

  m_tabDragging = false;
  // call tabRemoved to emit allTabRemoved if this had only one tab before drag (it's empty now)
  tabRemoved(-1);
}

void TabView::request(TabView* view,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object&) {
  if (method == "count") {
    PluginManager::singleton().sendResponse(view->count(), msgpack::type::nil(), msgId);
  } else if (method == "currentIndex") {
    PluginManager::singleton().sendResponse(view->currentIndex(), msgpack::type::nil(), msgId);
  } else {
    qDebug("method: %s not found", qPrintable(method));
  }
}

void TabView::notify(TabView* view, const QString& method, const msgpack::object& obj) {
  int numArgs = obj.via.array.size;
  if (method == "closeAllTabs") {
    view->closeAllTabs();
  } else if (method == "closeActiveTab") {
    view->closeActiveTab();
  } else if (method == "closeOtherTabs") {
    view->closeOtherTabs();
  } else if (method == "addNew") {
    view->addNew();
  } else if (method == "setCurrentIndex") {
    if (numArgs == 2) {
      std::tuple<int, int> params;
      obj.convert(&params);
      int index = std::get<1>(params);
      view->setCurrentIndex(index);
    } else {
      qWarning("invalid numArgs: %d", numArgs);
    }
  }
}
int TabView::insertTabInformation( const int index ){
  TextEditView* v = qobject_cast<TextEditView*>(widget(index));
  if (!v) {
    return false;
  }
  QString path    = v->path();

  // Declaration variables to insert tab information.
  QSettings tabViewHistoryTable(Constants::tabViewInformationPath(),
                                QSettings::IniFormat);

  // set tab information to array.
  tabViewHistoryTable.beginWriteArray(PREFIX);
  tabViewHistoryTable.setArrayIndex(index);
  tabViewHistoryTable.setValue(PATH_KEY, path.toStdString().c_str());
  tabViewHistoryTable.endArray();

  return index;

}
bool TabView::createWithSavedTabs( void ){
  // declaration variables to insert tab information.
  QSettings tabViewHistoryTable(Constants::tabViewInformationPath(),
                                QSettings::IniFormat);

  // get array size.
  int size = tabViewHistoryTable.beginReadArray(PREFIX);

  // if array size is 0, return false
  if( !size ){
    return false;
  }

  // restore tab information.
  for (int i = 0; i < size; i++) {
    tabViewHistoryTable.setArrayIndex(i);
    const QVariant& value = tabViewHistoryTable.value(PATH_KEY);
    // if value is empty,creat new window.
    if (value.toString().isEmpty()){
       addNew();
    }
    // if value convert to QString, open file.
    if (value.canConvert<QString>()) {
       open(value.toString());
    }
  }

  tabViewHistoryTable.endArray();
  
  return true;
}
