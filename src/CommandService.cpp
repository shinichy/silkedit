#include <QDebug>

#include "CommandService.h"

void CommandService::runCommand(const QString& name, const CommandArgument& args, int repeat) {
  if (m_commands.find(name) != m_commands.end()) {
    m_commands[name]->run(args, repeat);
  } else {
    qDebug() << "Can't find a command: " << name;
  }
}

void CommandService::addCommand(std::unique_ptr<ICommand> cmd) {
  m_commands[cmd->name()] = std::move(cmd);
}