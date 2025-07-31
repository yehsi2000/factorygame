#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

class Registry;

class MovementSystem {
    Registry* registry;
public:
    MovementSystem(Registry* r);
    void Update(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
