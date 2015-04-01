#include "PluginContext.h"
#include "PluginManager.h"

PluginContext::PluginContext(const QString& context) : m_key(context) {
}

bool PluginContext::isSatisfied(Operator op, const QString& operand) {
  return PluginManager::singleton().askExternalContext(m_key, op, operand);
}

QString PluginContext::key() {
  throw std::runtime_error("this method should not be called");
}