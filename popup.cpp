#include "wotwar.h"
#include <cmath>

POPUP::POPUP(vector<BUILDABLE>* items, AGENT_HANDLER* ahandler, int player_id) : s(0), items(items), ah(ahandler), player_id(player_id)
{
    if (items && items->size() > MAX_BUILDS) {
        printf("error: limit of %d buildables in a building popup has been exceeded", MAX_BUILDS);
        panic("building with too many buildable units (limit is MAX_BUILDS in sizes.h)");
    }

    // check if already in process of creating a building
    GROUP* gp = ah->get_player_pointer(player_id)->get_selected_building();
    if (gp && gp->get_state() == GROUP::CREATING_BUILDING) items = NULL;
}

POPUP::~POPUP(void)
{

}

void POPUP::update_pos(int x, int y)
{
    if (!items) return;

    // grow popup
    s += 0.07;
    if (s > 1.) s = 1.;

    // calculate popup size
    double c = items->size() * POPUP_SIZE * s;
    double r = c / (2 * M_PI);
    if (items->size() == 2) r = POPUP_SIZE * s/2;
    if (!items->size()) r = 0.;

    // ensure pos vector is large enough
    if (items->size() != pos.size()) pos.resize(items->size());

    // draw popup
    if (items->size() == 1) {
         pos[0].x = x - (int)(POPUP_SIZE*s/2);
         pos[0].y = y - (int)(POPUP_SIZE*s/2);
         pos[0].depth = POPUP_SIZE * s;
    } else {
        for (int i = 0; i < items->size(); i++) {
            double ttheta = (double)i / items->size() * 2 * M_PI + 3./2.*M_PI + M_PI/2 * (1. - s);
            pos[i].x = x + (int)(r*cos(ttheta)) - (int)(POPUP_SIZE*s/2);
            pos[i].y = y + (int)(r*sin(ttheta)) - (int)(POPUP_SIZE*s/2);
            pos[i].depth = POPUP_SIZE * s;
        }
    }
}

void POPUP::draw(DISPLAY * disp, int x, int y)
{
    if (!items) return;

    // update popup
    update_pos(x, y);

    // check if mouseover
    int t = which_over(mouse_x, mouse_y);
    int price_draw = -1;

    // get build status
    PLAYER* pl = ah->get_player_pointer(player_id);
    GROUP* gp = pl->get_selected_building();
    TASK* tp = NULL;
    TASKLIST* tl = gp ? gp->get_tasklist() : NULL;
    if (tl) tl->count_builds(barray);

    // don't draw for in-build buildings
    if (tl->currentType() == TASK::CREATE_BUILDING) {
        items = NULL;
        return;
    }

    // draw popup
    for (int i = 0; i < items->size(); i++) {
        masked_stretch_blit((*items)[i].sprite, disp->buffer, (t == i) ? POPUP_SIZE : 0, 0, POPUP_SIZE, POPUP_SIZE, pos[i].x, pos[i].y, pos[i].depth, pos[i].depth);
        if (tl) {
            if (tl->currentType() == TASK::BUILD) tp = tl->current();
            else if (tl->current() && tl->current()->makeCurrent && tl->current()->pNext && tl->current()->pNext->type == TASK::BUILD) tp = tl->current()->pNext;
            // draw progress bar
            if (tp && tp->build_id == i) {
                int xt1, yt1, xt2, yt2;
                xt1 = pos[i].x + pos[i].depth * 0.25;
                yt1 = pos[i].y + pos[i].depth * 0.4;
                xt2 = pos[i].x + pos[i].depth * (0.25 + 0.5 * (double)tp->counter / tp->buildtime);
                yt2 = pos[i].y + pos[i].depth * 0.6;
                rectfill(disp->buffer, xt1, yt1, xt2, yt2, makecol(50,50,50));
                rect(disp->buffer, xt1, yt1, xt2, yt2, makecol(150,150,150));
            }
            if (barray[i]) textprintf_centre_ex(disp->buffer, game_font, pos[i].x + pos[i].depth/2, pos[i].y + pos[i].depth/2 - text_height(game_font)/2, makecol(255,255,255), -1, "%d", barray[i]);
        }
        // draw cost
        if (t == i) price_draw = i;
    }

    // draw cost
    if (price_draw > -1) {
        BITMAP* res_ico = ah->get_data()->ui_data.resource_icon;
        masked_blit(res_ico, disp->buffer, 0, 0, mouse_x + 18, mouse_y + 16, res_ico->w, res_ico->h);
        textprintf_ex(disp->buffer, game_font, mouse_x + res_ico->w + 21, mouse_y + 19, makecol(0,0,0), -1, "%d", (*items)[price_draw].cost);
        textprintf_ex(disp->buffer, game_font, mouse_x + res_ico->w + 20, mouse_y + 18, (*items)[price_draw].cost > pl->get_resources() ? makecol(172,40,40) : makecol(255,255,255), -1, "%d", (*items)[price_draw].cost);
    }
}

int POPUP::which_over(int x, int y)
{
    if (!items) return -1;

    for (int i = 0; i < pos.size(); i++) {
        if ((pos[i].x + pos[i].depth/2 - x)*(pos[i].x + pos[i].depth/2 - x)+(pos[i].y + pos[i].depth/2 - y)*(pos[i].y + pos[i].depth/2 - y) < pos[i].depth/2 * pos[i].depth/2) return i;
    }

    return -1;
}

void POPUP::click(int x, int y)
{
    if (!items) return;
    int i = which_over(x, y);
    if (i < 0) return;

    bool unbuild = (mouse_b & 2);

    // check resources

    int cost = (*items)[i].cost;
    PLAYER* pl = ah->get_player_pointer(player_id);

    // check and subtract resourcs
    if (!unbuild) {
        if (cost > pl->get_resources()) return;
        pl->subtract_resources(cost);
    }

    // add build task
    if (!(*items)[i].is_building) {
        // build units
        if (unbuild) ah->setTasktoSelected(new TASK(TASK::UNBUILD, player_id, i, 0, true));
        else ah->setTasktoSelected(new TASK(TASK::BUILD, player_id, i, (*items)[i].train_time));
    } else if (!unbuild) {
        // build building
        ah->setTasktoSelected(new TASK(TASK::BUILD_BUILDING, player_id, i, (*items)[i].train_time));
        items = NULL;
    }
}
