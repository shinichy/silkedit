#pragma once

#include <boost/optional.hpp>
#include <unordered_map>
#include <QVariant>

#include "macros.h"
#include "stlSpecialization.h"

class QString;

class CommandArgument {
  typedef std::unordered_map<QString, QVariant> ARGS;

 public:
  CommandArgument() = default;
  explicit CommandArgument(ARGS args);
  ~CommandArgument() = default;
  DEFAULT_COPY_AND_MOVE(CommandArgument)

  template <typename T>
  inline boost::optional<T> find(const QString& key) const {
    if (m_args.find(key) == m_args.end()) {
      return boost::none;
    }

    QVariant argVar = m_args.at(key);
    if (!argVar.canConvert<T>()) {
      return boost::none;
    }

    return argVar.value<T>();
  }

 private:
  ARGS m_args;
};
