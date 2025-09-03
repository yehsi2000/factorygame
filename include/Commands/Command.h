#ifndef CORE_COMMAND_
#define CORE_COMMAND_

class Registry;
class EventDispatcher;
class World;

/**
 * @brief Abstract base class for all commands. Commands are self-contained units of work that modify the game state.
 * @details The context (engine, registry, etc.) is passed to Execute to give the command the power to change the world state.
 */
class Command {
 public:
  virtual ~Command() = default;
  virtual void Execute(Registry *registry, EventDispatcher* eventDispatcher, World* world) = 0;
};

#endif /* CORE_COMMAND_ */
