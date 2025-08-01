#ifndef COMMANDS_PERFORMINTERACTIONCOMMAND_
#define COMMANDS_PERFORMINTERACTIONCOMMAND_

#include <iostream>

#include "Core/Command.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "SDL.h"

// A command to execute a generic interaction.
// In a real game, this would likely take an entity ID for the instigator
// and the target of the interaction.
class PerformInteractionCommand : public Command {
 public:
  PerformInteractionCommand() {}

  void Execute(GEngine& engine, Registry& registry) override {
    // For now, we'll just print a message to confirm the command is working.
    // In a real implementation, this would contain the logic for what
    // happens during an interaction, e.g., opening a chest, talking to an NPC
    // ,etc
    std::cout << "Interaction Command Executed! Time:" << SDL_GetTicks()
              << std::endl;
  }
};

#endif /* COMMANDS_PERFORMINTERACTIONCOMMAND_ */
