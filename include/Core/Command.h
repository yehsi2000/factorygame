#ifndef CORE_COMMAND_
#define CORE_COMMAND_

// Forward declarations to avoid circular dependencies
class GEngine;
class Registry;

// Abstract base class for all commands.
// Commands are self-contained units of work that modify the game state.
class Command {
 public:
  virtual ~Command() = default;
  // The context (engine, registry, etc.) is passed to Execute
  // to give the command the power to change the world state.
  virtual void Execute(GEngine& engine, Registry& registry) = 0;
};

#endif /* CORE_COMMAND_ */
