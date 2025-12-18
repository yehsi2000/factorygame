#pragma once
#include <array>
#include "DataStruct/AnimationData.h"

class AnimationManager {
public:
    // preload animation data
    const AnimationSequence* GetAnimation(AnimationName type) const {
        return &mResources[static_cast<size_t>(type)];
    }
private:
    std::array<AnimationSequence, static_cast<size_t>(AnimationName::MAX_ANIMATIONS)> mResources;
};