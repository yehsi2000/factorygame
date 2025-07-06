#include "GameState.h"

#include <iostream>

#include "Engine.h"

// MainMenuState

void MainMenuState::Enter() { std::cout << "Entering Main Menu\n"; }

void MainMenuState::Exit() {}

// PlayState

void PlayState::Enter() { std::cout << "Starting game!\n"; }

void PlayState::Exit() {}