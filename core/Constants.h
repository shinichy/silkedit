﻿#pragma once

#include <node.h>
#include <QStringList>
#include <QObject>

#include "macros.h"
#include "Singleton.h"

namespace core {

class NODE_EXTERN Constants : public QObject, public core::Singleton<Constants> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Constants)

  Q_PROPERTY(QString userPackagesJsonPath READ userPackagesJsonPath CONSTANT)
  Q_PROPERTY(QString userPackagesNodeModulesPath READ userPackagesNodeModulesPath CONSTANT)

 public:
  ~Constants() = default;

#ifdef Q_OS_MAC
  static const QString defaultFontFamily;
#endif

#ifdef Q_OS_WIN
  static const QString defaultFontFamily;
#endif

  static const int defaultFontSize;

  QStringList configPaths();
  QStringList userKeymapPaths();
  QString userConfigPath();
  QString userKeymapPath();
  QString userPackagesRootDirPath() const;
  QString nodePath();
  QString npmCliPath();
  QString translationDirPath();
  QString jsLibDir();
  QString silkHomePath() const;
  QString recentOpenHistoryPath();
  QString tabViewInformationPath();
  QStringList themePaths();
  QStringList packagesPaths();
  QString userPackagesJsonPath() const;
  QString userPackagesNodeModulesPath() const;

 private:
  friend class core::Singleton<Constants>;
  Constants() = default;

  QStringList dataDirectoryPaths();
};

}  // namespace core
