﻿#pragma once

#include <unordered_map>
#include <memory>
#include <QString>
#include <QVariant>

#include "core/stlSpecialization.h"
#include "core/macros.h"
#include "Context.h"
#include "CommandArgument.h"

class CommandEvent {
  DISABLE_COPY(CommandEvent)
 public:
  explicit CommandEvent(const QString& name);
  CommandEvent(const QString& name, const CommandArgument& args);
  CommandEvent(const QString& name, std::shared_ptr<Context> context);
  CommandEvent(const QString& name, const CommandArgument& args, std::shared_ptr<Context> context);
  ~CommandEvent() = default;
  DEFAULT_MOVE(CommandEvent)

  QString cmdName() { return m_cmdName; }
  Context* context() { return m_context.get(); }

  bool execute(int repeat = 1);
  bool hasContext();

 private:
  QString m_cmdName;
  CommandArgument m_args;
  std::shared_ptr<Context> m_context;
};
