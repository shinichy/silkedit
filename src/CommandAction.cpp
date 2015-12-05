﻿#include "CommandAction.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "core/PackageManager.h"
#include "core/Package.h"
#include "core/Config.h"
#include "core/Theme.h"

using core::Config;
using core::Theme;
using core::ColorSettings;
using core::PackageManager;
using core::Package;

void CommandAction::init(const QString& id) {
  setObjectName(id);

  // WidgetShortcut makes QAction's shortcut disabled but visible in a menu
  // Shortcut is handled by TextEditViewKeyHandler.
  setShortcutContext(Qt::WidgetShortcut);

  updateVisibilityAndShortcut();

  connect(this, &QAction::triggered, [this] { CommandManager::singleton().runCommand(m_cmdName); });
  connect(&KeymapManager::singleton(), &KeymapManager::shortcutUpdated, this,
          [=](const QString& cmdName, const QKeySequence& key) {
            if (cmdName == m_cmdName) {
              setShortcut(key);
            }
          });
}

CommandAction::CommandAction(const QString& id,
                             const QString& text,
                             const QString& cmdName,
                             QObject* parent,
                             boost::optional<core::AndConditionExpression> cond,
                             const QString& pkgName)
    : PackageAction(text, pkgName, parent, cond), m_cmdName(cmdName) {
  init(id);
}

CommandAction::CommandAction(const QString& id,
                             const QString& cmdName,
                             const QIcon& icon,
                             QObject* parent,
                             boost::optional<core::AndConditionExpression> cond,
                             const QString& pkgName)
    : PackageAction(icon, id, pkgName, parent, cond), m_cmdName(cmdName) {
  init(id);
}

void CommandAction::updateShortcut()
{
  QKeySequence key = KeymapManager::singleton().findShortcut(m_cmdName);
  setShortcut(key);
}

void CommandAction::updateVisibilityAndShortcut()
{
  PackageAction::updateVisibilityAndShortcut();
  updateShortcut();
}

CommandAction::CommandAction(const QString& id,
                             const QString& cmdName,
                             const QMap<QString, QString>& icons,
                             QObject* parent,
                             const QString& pkgName)
    : PackageAction(id, pkgName, parent), m_icons(icons), m_cmdName(cmdName) {
  init(id, cmdName);
  setTheme();
  connect(&Config::singleton(), &Config::themeChanged, this, &CommandAction::setTheme);
}

void CommandAction::setTheme() {
  if(Config::singleton().theme()->isDarkTheme()){
    setIcon(QIcon(m_icons.value("light", NULL)));
  } else {
    setIcon(QIcon(m_icons.value("dark", NULL)));
  }
}

