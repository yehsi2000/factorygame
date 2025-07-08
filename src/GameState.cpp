#include "GameState.h"

#include <iostream>

// MainMenuState

void MainMenuState::Enter() { std::cout << "Entering Main Menu\n"; }

void MainMenuState::Exit() { std::cout << "Exiting Main Menu\n"; }

// PlayState

void PlayState::Enter() { std::cout << "Starting game!\n"; }

void PlayState::Exit() { std::cout << "Exiting game!\n"; }