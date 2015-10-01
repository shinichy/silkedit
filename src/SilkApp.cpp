#include <QWidget>

#include "SilkApp.h"
#include "TabViewGroup.h"
#include "DocumentManager.h"
#include "TextEditView.h"
#include "TabView.h"
#include "Window.h"
#include "PluginManager.h"
#include "version.h"

namespace {
template <typename T>
T findParent(QWidget* widget) {
  if (!widget)
    return nullptr;

  T desiredWidget = qobject_cast<T>(widget->parentWidget());
  if (desiredWidget)
    return desiredWidget;
  return findParent<T>(widget->parentWidget());
}
}

TabBar* SilkApp::tabBarAt(int x, int y) {
  foreach (Window* window, Window::windows()) {
    if (TabBar* tabBar = window->tabViewGroup()->tabBarAt(x, y)) {
      return tabBar;
    }
  }

  return nullptr;
}

SilkApp::SilkApp(int& argc, char** argv) : QApplication(argc, argv) {
  setApplicationVersion(VERSION);

  QFile file(":/stylesheet.css");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStyleSheet(file.readAll());
    file.close();
  }

  // Track active TabView
  QObject::connect(this, &QApplication::focusChanged, [this](QWidget*, QWidget* focusedWidget) {
    //    qDebug("focusChanged");
    if (TextEditView* editView = qobject_cast<TextEditView*>(focusedWidget)) {
      if (TabView* tabView = findParent<TabView*>(editView)) {
        if (TabViewGroup* tabViewGroup = findParent<TabViewGroup*>(tabView)) {
          tabViewGroup->setActiveTab(tabView);

          // send focusChangedEvent to plugin runner
          PluginManager::singleton().sendFocusChangedEvent("TextEditView");
        } else {
          qDebug("unable to find the parent TabViewGroup");
        }
      } else {
        qDebug("can't find TabView in ancestor");
      }
    } else {
      qDebug("focused widget is not TextEditView");
    }
  });
}

bool SilkApp::event(QEvent* event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      qDebug("FileOpen event");
      DocumentManager::open(static_cast<QFileOpenEvent*>(event)->file());
      return true;
    default:
      return QApplication::event(event);
  }
}

TextEditView* SilkApp::activeEditView() {
  TabView* tabView = activeTabView();
  if (tabView) {
    return tabView->activeEditView();
  } else {
    qDebug("active tab view is null");
    return nullptr;
  }
}

TabView* SilkApp::activeTabView() {
  TabViewGroup* tabViewGroup = activeTabViewGroup();
  if (tabViewGroup) {
    return tabViewGroup->activeTab();
  } else {
    qDebug("active tab view group is null");
    return nullptr;
  }
}

TabViewGroup* SilkApp::activeTabViewGroup() {
  Window* window = activeWindow();
  if (window) {
    return window->tabViewGroup();
  } else {
    qDebug("active window is null");
    return nullptr;
  }
}

Window* SilkApp::activeWindow() {
  return qobject_cast<Window*>(QApplication::activeWindow());
}
