#ifndef COMPONENTS_ANIMATIONCOMPONENT_
#define COMPONENTS_ANIMATIONCOMPONENT_

#include <string>
#include <vector>
#include <map>

// 하나의 애니메이션 시퀀스에 대한 정보 (예: "walk", "idle", "attack")
struct AnimationSequence {
    int startIndex;    // 스프라이트 시트에서 이 애니메이션의 시작 프레임 인덱스
    int numFrames;     // 이 애니메이션을 구성하는 프레임의 총 개수
    float frameRate;   // 프레임 전환 속도 (초당 프레임 수, 예: 8.f)
    int frameWidth = 16; // 스프라이트 시트 내부 한 프레임의 이미지 너비(픽셀)
    int frameHeight = 16; // 스프라이트 시트 내부 한 프레임의 이미지 높이(픽셀)
    bool loop = true;  // 반복 여부
};

struct AnimationComponent {
    // 사용 가능한 모든 애니메이션 시퀀스를 저장 (예: "walk" -> AnimationSequence)
    std::map<std::string, AnimationSequence> animations;

    // 현재 재생 중인 애니메이션
    std::string currentAnimationName;
    int currentFrameIndex = 0; // 현재 시퀀스 내에서의 프레임 인덱스 (0부터 시작)
    float frameTimer = 0.f;   // 다음 프레임으로 넘어가기까지 남은 시간

    // 애니메이션 재생/정지 제어
    bool isPlaying = true;
};

#endif /* COMPONENTS_ANIMATIONCOMPONENT_ */
