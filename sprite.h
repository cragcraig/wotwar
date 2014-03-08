#ifndef SPRITE_H
#define SPRITE_H

#include "wotwar_classes.h"

class SPRITE
{
    private:
        double animationCounter;
        int frame_number, prev_frame;
        bool is_first_display;
        ANIMATION_DATA * animationData;
		ANIMATION * currentAnimation;
		FRAME * currentFrame;
		ANIMATION_DATA::ANI_TYPE animationType;

    protected:
        BITMAP * updateAnimation(void);
        BITMAP * getFrame(void);
        int frame_num(void);
        bool is_first_frame_display(void);

    public:
        SPRITE(ANIMATION_DATA * ani_data);
        void setAnimation(ANIMATION_DATA::ANI_TYPE t, bool allowContinue=true);
};


#endif
