#ifndef CORE_COMMAND_
#define CORE_COMMAND_

class GEngine;
class Registry;

/**
 * @brief Abstract base class for all commands. Commands are self-contained units of work that modify the game state.
 * @details The context (engine, registry, etc.) is passed to Execute to give the command the power to change the world state.
 */
class Command {
 public:
  virtual ~Command() = default;
  virtual void Execute(GEngine& engine, Registry& registry) = 0;
};

#endif /* CORE_COMMAND_ */
