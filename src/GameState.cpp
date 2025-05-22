#include "World.h"
#include "GameState.h"

#include <iostream>

//MainMenuState

void MainMenuState::Enter() {
    std::cout<<"Entering Main Menu\n";
}

void MainMenuState::Exit(){

}

void MainMenuState::Update(){
    std::cout << "In Main Menu... Press 1 to start game\n";
    int input;
    std::cin>>input;
    if(input==1) World::Instance().ChangeState(std::make_unique<PlayState>());
}

//PlayState

void PlayState::Enter(){
    std::cout << "Starting game!\n";
}

void PlayState::Exit(){
    
}

void PlayState::Update(){
    std::cout<<"Playing... Press 2 to quit\n";
    int input;
    std::cin>>input;
    if(input==2) World::Instance().ChangeState(std::make_unique<MainMenuState>());
}
