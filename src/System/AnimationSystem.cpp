#include "System/AnimationSystem.h"

#include "Components/AnimationComponent.h"
#include "Components/SpriteComponent.h"
#include "Entity.h"

AnimationSystem::AnimationSystem(Registry* r) { registry = r; }

void AnimationSystem::Update(float deltaTime) {
  // AnimationComponent와 SpriteComponent를 모두 가진 엔티티를 순회
  for (EntityID entity :
       registry->view<AnimationComponent, SpriteComponent>()) {
    auto& anim = registry->GetComponent<AnimationComponent>(entity);
    auto& sprite = registry->GetComponent<SpriteComponent>(entity);

    // 애니메이션이 재생 중이 아닐 경우 아무것도 하지 않음
    if (!anim.isPlaying) {
      continue;
    }

    // 현재 재생 중인 애니메이션 시퀀스 정보를 가져옴
    const auto& sequence = anim.animations.at(anim.currentAnimationName);

    // 타이머 업데이트
    anim.frameTimer += deltaTime;

    // 다음 프레임으로 넘어갈 시간이 되었는지 확인
    // frameRate가 8이라면, 한 프레임당 시간은 1.0 / 8.0 초
    if (anim.frameTimer >= (1.f / sequence.frameRate)) {
      anim.frameTimer = 0.f;  // 타이머 리셋

      // 프레임 인덱스를 1 증가시킴
      anim.currentFrameIndex++;

      // 마지막 프레임에 도달했는지 확인
      if (anim.currentFrameIndex >= sequence.numFrames) {
        if (sequence.loop) {
          // 루프가 설정되어 있으면 처음 프레임으로 돌아감
          anim.currentFrameIndex = 0;
        } else {
          // 루프가 아니면 마지막 프레임에서 멈추고 재생 중단
          anim.currentFrameIndex = sequence.numFrames - 1;
          anim.isPlaying = false;
        }
      }
    }

    // --- SpriteComponent의 srcRect 업데이트 ---
    // 스프라이트 시트의 전체 너비/높이를 알아야 함
    int sheetWidth, sheetHeight;
    SDL_QueryTexture(sprite.texture, NULL, NULL, &sheetWidth, &sheetHeight);

    int framesPerRow = sheetWidth / sequence.frameWidth;

    // 최종 프레임 인덱스 계산 (시퀀스의 시작 인덱스 + 현재 프레임 인덱스)
    int globalFrameIndex = sequence.startIndex + anim.currentFrameIndex;

    // srcRect의 x, y 위치 계산
    sprite.srcRect.x = (globalFrameIndex % framesPerRow) * sequence.frameWidth;
    sprite.srcRect.y = (globalFrameIndex / framesPerRow) * sequence.frameHeight;
    sprite.srcRect.w = sequence.frameWidth;
    sprite.srcRect.h = sequence.frameHeight;
  }
}
