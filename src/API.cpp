#include <string>
#include <QApplication>
#include <QMessageBox>

#include "API.h"
#include "MainWindow.h"
#include "CommandManager.h"
#include "commands/PluginCommand.h"
#include "PluginManager.h"
#include "TextEditView.h"
#include "SilkApp.h"
#include "TabView.h"

std::unordered_map<std::string, std::function<void(msgpack::object)>> API::notifyFunctions;
std::unordered_map<std::string, std::function<void(msgpack::rpc::msgid_t, msgpack::object)>>
    API::requestFunctions;

void API::init() {
  notifyFunctions.insert(std::make_pair("alert", &alert));
  notifyFunctions.insert(std::make_pair("load_menu", &loadMenu));
  notifyFunctions.insert(std::make_pair("register_commands", &registerCommands));

  requestFunctions.insert(std::make_pair("active_view", &activeView));
  requestFunctions.insert(std::make_pair("active_tab_view", &activeTabView));
  requestFunctions.insert(std::make_pair("active_window", &activeWindow));
}

void API::hideActiveFindReplacePanel() {
  if (MainWindow* window = SilkApp::activeWindow()) {
    window->hideFindReplacePanel();
  }
}

void API::call(const std::string& method, const msgpack::object& obj) {
  if (notifyFunctions.count(method) != 0) {
    notifyFunctions.at(method)(obj);
  } else {
    qWarning("%s is not supported", method.c_str());
  }
}

void API::call(const std::string& method, msgpack::rpc::msgid_t msgId, const msgpack::object& obj) {
  if (requestFunctions.count(method) != 0) {
    requestFunctions.at(method)(msgId, obj);
  } else {
    qWarning("%s is not supported", method.c_str());
  }
}

void API::alert(msgpack::object obj) {
  msgpack::type::tuple<std::string> params;
  obj.convert(&params);
  std::string msg = std::get<0>(params);
  QMessageBox msgBox;
  msgBox.setText(QString::fromUtf8(msg.c_str()));
  msgBox.exec();
}

void API::loadMenu(msgpack::object obj) {
  msgpack::type::tuple<std::string> params;
  obj.convert(&params);
  std::string ymlPath = std::get<0>(params);
  MainWindow::loadMenu(ymlPath);
}

void API::registerCommands(msgpack::object obj) {
  msgpack::type::tuple<std::vector<std::string>> params;
  obj.convert(&params);
  std::vector<std::string> commands = std::get<0>(params);
  for (std::string& cmd : commands) {
    qDebug("command: %s", cmd.c_str());
    CommandManager::add(
        std::unique_ptr<ICommand>(new PluginCommand(QString::fromUtf8(cmd.c_str()))));
  }
}

void API::activeView(msgpack::rpc::msgid_t msgId, msgpack::object) {
  TextEditView* editView = SilkApp::activeEditView();
  if (editView) {
    PluginManager::singleton().sendResponse(editView->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::activeTabView(msgpack::rpc::msgid_t msgId, msgpack::object) {
  TabView* tabView = SilkApp::activeTabView();
  if (tabView) {
    PluginManager::singleton().sendResponse(tabView->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void API::activeWindow(msgpack::rpc::msgid_t msgId, msgpack::object) {
  MainWindow* window = SilkApp::activeWindow();
  if (window) {
    PluginManager::singleton().sendResponse(window->id(), msgpack::type::nil(), msgId);
  } else {
    PluginManager::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}
