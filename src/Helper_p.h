﻿#pragma once

#include "Helper.h"
#include "core/IKeyEventFilter.h"

namespace node {
class ArrayBufferAllocator;
}

class HelperPrivate : public QObject, public core::IKeyEventFilter {
  Q_OBJECT

 public:
  Helper* q;

  explicit HelperPrivate(Helper* q_ptr);
  ~HelperPrivate();

  void init();

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;
  void startNodeEventLoop();
  void startNodeInstance(void* arg);
  void quitApplication();
  void cacheMethods(const QString& className, const QMetaObject* object);
  void cleanup();
  void emitSignal(QObject *obj, const QString& signal, QVariantList args);
  QVariant callFunc(const QString& funcName, QVariantList args = QVariantList());

  template <typename T>
  T callFunc(const QString& funcName, QVariantList args, T defaultValue);
};
