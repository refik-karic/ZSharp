#include "ConsoleVariable.h"

namespace ZSharp {

HashTable<String, Delegate<const String&>>& GlobalConsoleCommands() {
  static HashTable<String, Delegate<const String&>> GlobalConsoleCommands;
  return GlobalConsoleCommands;
}

HashTable<String, Delegate<void>>& GlobalConsoleCommandsValueless() {
  static HashTable<String, Delegate<void>> GlobalConsoleCommandsValueless;
  return GlobalConsoleCommandsValueless;
}

}
