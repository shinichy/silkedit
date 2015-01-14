#pragma once

#include <QList>

#include "macros.h"

class TextEditView;
class MainWindow;
class TabView;
class TabViewGroup;

class API {
  DISABLE_COPY_AND_MOVE(API)

 public:
  static TextEditView* activeEditView();
  static TabView* activeTabView();
  static TabViewGroup* activeTabViewGroup();
  static MainWindow* activeWindow();
  static QList<MainWindow*> windows();

 private:
  API() = delete;
  ~API() = delete;
};
