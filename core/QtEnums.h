#pragma once
#include <QObject>

namespace core {

// enums in this class are not registered in Qt side
class QtEnums : public QObject {
  Q_OBJECT

 public:
  enum KeyboardModifier {
    NoModifier = 0x00000000,
    ShiftModifier = 0x02000000,
    ControlModifier = 0x04000000,
    AltModifier = 0x08000000,
    MetaModifier = 0x10000000,
    KeypadModifier = 0x20000000,
    GroupSwitchModifier = 0x40000000,
    // Do not extend the mask to include 0x01000000
    KeyboardModifierMask = 0xfe000000
  };
  Q_ENUM(KeyboardModifier)

  enum EventPriority { HighEventPriority = 1, NormalEventPriority = 0, LowEventPriority = -1 };
  Q_ENUM(EventPriority)
};

}  // namespace core
