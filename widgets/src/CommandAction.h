#pragma once

#include <QAction>

#include "core/macros.h"
#include "core/PackageAction.h"
#include "core/CommandArgument.h"

namespace core {
class Theme;
}

class CommandAction : public core::PackageAction {
  Q_OBJECT
 public:
  CommandAction(const QString& id,
                const QString& text,
                const QString& cmdName,
                QObject* parent = nullptr,
                const CommandArgument& args = CommandArgument(),
                boost::optional<core::AndConditionExpression> cond = boost::none,
                const QString& pkgName = "");

  CommandAction(const QString& id,
                const QString& cmdName,
                const QIcon& icon,
                QObject* parent = nullptr,
                const CommandArgument& args = CommandArgument(),
                boost::optional<core::AndConditionExpression> cond = boost::none,
                const QString& pkgName = "");

  CommandAction(const QString& id,
                const QString& cmdName,
                const QMap<QString, QString>& icons,
                QObject* parent = nullptr,
                const CommandArgument& args = CommandArgument(),
                boost::optional<core::AndConditionExpression> cond = boost::none,
                const QString& pkgName = "");

  ~CommandAction() = default;
  DEFAULT_COPY_AND_MOVE(CommandAction)

  void updateVisibilityAndShortcut() override;

 private:
  QMap<QString, QString> m_icons;
  QString m_cmdName;
  CommandArgument m_args;

  void init(const QString& id);
  void updateShortcut();
  void setTheme(const core::Theme* theme);
};
