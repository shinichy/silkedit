#pragma once

#include <unordered_map>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "macros.h"

class ICommand {
  DISABLE_COPY_AND_MOVE(ICommand)
 public:
  explicit ICommand(QString name) : m_name(name) {}
  virtual ~ICommand() = default;

  inline void run(const std::unordered_map<QString, QVariant>& args) {
    qDebug() << "Start command: " << m_name;
    doRun(args);
    qDebug() << "End command: " << m_name;
  }

  inline QString name() { return m_name; }

 private:
  virtual void doRun(const std::unordered_map<QString, QVariant>& args) = 0;

  QString m_name;
};