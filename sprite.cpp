#include "wotwar.h"

SPRITE::SPRITE(ANIMATION_DATA * ani_data) : animationData(ani_data), frame_number(0), is_first_display(true)
{
    if (animationData) setAnimation(ANIMATION_DATA::STAND, false);
}

void SPRITE::setAnimation(ANIMATION_DATA::ANI_TYPE t, bool allowContinue)
{
	if ((animationType == t || (animationType == ANIMATION_DATA::IDLE && t == ANIMATION_DATA::STAND)) && allowContinue) return; // just return if the animation can continue and is being set to the same type
	else {
		ANIMATION * oldAniPointer = currentAnimation;
		currentAnimation = animationData->getAnimation(t);
		if (currentAnimation != oldAniPointer || !allowContinue) {
			animationCounter = 0.0;
			frame_number = 0;
			currentFrame = currentAnimation->pFirst;
			if (animationType != t) { // randomly offset walk and stand animations
				if (t == ANIMATION_DATA::MOVE) {
					int k = rand() % currentAnimation->length;
					for (int i = 0; i < k; i++) currentFrame = currentFrame->pNext;
				}
				else if (t == ANIMATION_DATA::STAND) {
					animationCounter = (float)(rand() % 500) / 1000.0;
				}
			}
			animationType = t;
		}
	}
}

BITMAP * SPRITE::updateAnimation(void)
{
	if ((animationCounter += currentAnimation->speed) > 1.0) {
		animationCounter -= 1.0;
		is_first_display = true;
		frame_number++;
		currentFrame = currentFrame->pNext;
		if (!currentFrame) {
		    frame_number = 0;
			if (animationType != ANIMATION_DATA::IDLE) setAnimation(animationType, false);
			else setAnimation(ANIMATION_DATA::STAND, false);
		}
		if (animationType == ANIMATION_DATA::STAND && !(rand()%3)) setAnimation(ANIMATION_DATA::IDLE, false); // throw in an idle animation
	} else is_first_display = false;
	return currentFrame->img;
}

BITMAP * SPRITE::getFrame(void)
{
    return currentFrame->img;
}

int SPRITE::frame_num(void)
{
    return frame_number;
}

bool SPRITE::is_first_frame_display(void)
{
    return is_first_display;
}
