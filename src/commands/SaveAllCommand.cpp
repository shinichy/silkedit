#include <QDebug>

#include "SaveAllCommand.h"
#include "API.h"
#include "MainWindow.h"

const QString SaveAllCommand::name = "save_all";

SaveAllCommand::SaveAllCommand() : ICommand(SaveAllCommand::name) {
}

void SaveAllCommand::doRun(const CommandArgument&, int) {
  API::activeWindow()->saveAllTabs();
}
