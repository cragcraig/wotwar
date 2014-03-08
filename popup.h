#ifndef POPUP_H
#define POPUP_H

#include "wotwar_classes.h"
#include <vector>

class POPUP
{
    private:
        double s;
        int player_id;
        AGENT_HANDLER* ah;
        vector<BUILDABLE>* items;
        vector<COORDINATE> pos;
        void update_pos(int x, int y);
        int barray[MAX_BUILDS];

    public:
        POPUP(vector<BUILDABLE>* items, AGENT_HANDLER* ahandler, int player_id);
        ~POPUP(void);

        void draw(DISPLAY * disp, int x, int y);
        void check_click(int x, int y);

        int which_over(int x, int y);
        void click(int x, int y);
};






#endif
