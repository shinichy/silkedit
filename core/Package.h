#pragma once

#include <memory>
#include <tuple>
#include <QString>
#include <QJsonValue>

#include "macros.h"

// Package model class
namespace core {
struct Package {
  DEFAULT_COPY_AND_MOVE(Package)

  static const int ITEM_COUNT = 4;
  static Package fromJson(const QJsonValue& value) { return std::move(Package(value)); }

  QString name;
  QString version;
  QString description;
  QString repository;

  explicit Package(const QJsonValue& jsonValue);
  ~Package() = default;

  QStringList validate();
  QString tarballUrl();
};
}  // namespace core
