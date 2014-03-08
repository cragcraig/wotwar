#include "wotwar.h"

BUILDING_DATA::BUILDING_DATA(void) : sprite(NULL), field(0), loading_flag(0), data(NULL), is_generic(false), is_buildsite(false), resources_per_sec(-1)
{

}

BUILDING_DATA::~BUILDING_DATA(void)
{
    //if (sprite) destroy_bitmap(sprite);

    for (int i = 0; i < buildables.size(); i++) {
        if (buildables[i].sprite) destroy_bitmap(buildables[i].sprite);
    }
}

void BUILDING_DATA::give_dataobj(DATA * data)
{
    this->data = data;
}

void BUILDING_DATA::set_sprite(const char * s)
{
    if (sprite) return;
    //sprite = load_bitmap(s, NULL);

    // use stand animations for buildings
    ANIMATION_DATA * tani = data->getAnimationDataPointer(s);
    if (!tani) panic("building has no defined animation!");
    sprite = tani->getAnimation(ANIMATION_DATA::STAND)->pFirst->img;

    if (!sprite) {
        printf("BUILDING_DATA: bitmap %s does not exist\n", s);
        panic("missing building sprite");
    }
}

bool BUILDING_DATA::set_data(int d)
{
    // get resources per second
    if (resources_per_sec < 0) {
        resources_per_sec = abs(d);
        return true;
    }

	switch (loading_flag) {
        case 1: buildables.back().quantity = d;
                buildables.back().is_building = buildables.back().quantity ? false : true;
            break;
        case 2: buildables.back().cost = d;
            break;
        case 3: buildables.back().train_time = d;
            break;
        default:
                printf("BUILDING_DATA: error in list of buildables\n");
                panic("corrupt building data");
            break;
    }
    loading_flag++;

	field++;
	return true;
}

bool BUILDING_DATA::set_data(const char * s)
{
    // no resources per second
    if (resources_per_sec < 0) {
        resources_per_sec = 0;
    }

	switch (field) {
	    case 0 :
            strncpy(name, s, 256);
            name[255] = '\0';
            printf(" building: '%s'\n", name);
            set_sprite(name);
            break;
		default: buildables.push_back(BUILDABLE()); // create buildable
            strncpy(buildables.back().name, s, 256);
            // get sprite
            buildables.back().sprite = generate_sprite_buildable(s);
            printf("  (%s)\n", s);
            // get build info now
            loading_flag = 1;
            break;
	}
	field++;
	return true;
}

BITMAP * BUILDING_DATA::generate_sprite_buildable(const char * type)
{
    ANIMATION_DATA * tani;
    BITMAP * tbmp, * rbmp = create_bitmap(2*POPUP_SIZE, POPUP_SIZE);

    // get unit animation
    tani = data->getAnimationDataPointer(type);
    if (!tani) panic("buildable unit has no animation!");
    tbmp = tani->getAnimation(ANIMATION_DATA::STAND)->pFirst->img;
    if (!tbmp) panic("buildable unit has no stand animation!");

    // generate build UI image
    clear_to_color(rbmp, makecol(135, 126, 118));
    if (tbmp->w < POPUP_SIZE && tbmp->h < POPUP_SIZE) masked_blit(tbmp, rbmp, 0, 0, POPUP_SIZE/2 - tbmp->w/2, POPUP_SIZE/2 - tbmp->h/2, POPUP_SIZE, POPUP_SIZE);
    else if (tbmp->w > tbmp->h) masked_stretch_blit(tbmp, rbmp, 0, 0, tbmp->w, tbmp->h, 0, POPUP_SIZE/2 - POPUP_SIZE/2 * tbmp->h / tbmp->w, POPUP_SIZE, POPUP_SIZE * tbmp->h / tbmp->w);
    else masked_stretch_blit(tbmp, rbmp, 0, 0, tbmp->w, tbmp->h, POPUP_SIZE/2 - POPUP_SIZE/2 * tbmp->w / tbmp->h, 0, POPUP_SIZE * tbmp->w / tbmp->h, POPUP_SIZE);
    blit(rbmp, rbmp, 0, 0, POPUP_SIZE, 0, POPUP_SIZE, POPUP_SIZE);
    masked_blit(data->ui_data.build_button, rbmp, 0, 0, 0, 0, POPUP_SIZE, POPUP_SIZE);
    masked_blit(data->ui_data.highlight_build_button, rbmp, 0, 0, POPUP_SIZE, 0, POPUP_SIZE, POPUP_SIZE);
    floodfill(rbmp, 0, 0, makecol(255, 0, 255));

    return rbmp;
}
