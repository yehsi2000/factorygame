#pragma once
 
#include "Components/AnimationComponent.h"
#include "DataStruct/AnimationData.h"


namespace util {

void SetAnimation(AnimationName name, AnimationComponent &animComp,
                  bool bPlayNow);

void AddAnimation(AnimationComponent &animComp, const AnimationName& animName,
                  SDL_Texture * texture, AnimationSequence&& animSequence);

} // namespace util
