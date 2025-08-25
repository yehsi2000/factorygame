#ifndef UTIL_ANIMUTIL_
#define UTIL_ANIMUTIL_

#include "Components/AnimationComponent.h"


namespace util {

void SetAnimation(AnimationName name, AnimationComponent &animComp,
                  bool bPlay);

void AddAnimation(AnimationComponent &animComp, const AnimationName& animName,
                  SDL_Texture * texture, AnimationSequence&& animSequence);

} // namespace util

#endif /* UTIL_ANIMUTIL_ */
