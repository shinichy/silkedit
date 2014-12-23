#include "CommandEvent.h"
#include "CommandService.h"

CommandEvent::CommandEvent(const QString& name) : CommandEvent(name, CommandArgument(), nullptr) {}

CommandEvent::CommandEvent(const QString& name, const CommandArgument& args)
    : CommandEvent(name, args, nullptr) {}

CommandEvent::CommandEvent(const QString& name, std::shared_ptr<IContext> context)
    : CommandEvent(name, CommandArgument(), context) {}

CommandEvent::CommandEvent(const QString& name,
                           const CommandArgument& args,
                           std::shared_ptr<IContext> context)
    : m_cmdName(name), m_args(std::move(args)), m_context(context) {}

bool CommandEvent::execute(int repeat) {
  if (!m_context || m_context->isSatisfied()) {
    CommandService::singleton().runCommand(m_cmdName, m_args, repeat);
    return true;
  }

  return false;
}

bool CommandEvent::hasContext() { return m_context != nullptr; }
