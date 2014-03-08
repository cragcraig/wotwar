#include "wotwar.h"

ATTACK_DATA::ATTACK_DATA(const char * filepath)
{
    load_attacks(filepath);
}

const ATTACK_TYPE * ATTACK_DATA::get_type(const char * type)
{
    // create particle
    ATTACK_TYPE& dat = attack_lib[type];

    if (!dat.is_inited()) { // error, particle type does not exist or was not fully initialized
        printf("error: attack type '%s' is not defined\n", type);
        panic("attempted to use attack of an undefined type");
    }

    return &dat;
}

void ATTACK_DATA::load_attacks(const char * filepath)
{
    string path(filepath);
    path += "attacks.dat";
    CONFIG_FILE df(path.c_str());

    CONFIG_BLOCK * block;

    while (block = df.next()) {
        attack_lib[block->get_name()] = ATTACK_TYPE();
        printf(" attack: '%s'\n", block->get_name());
        // load data
        while (block->line_type()) {
            switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    attack_lib[block->get_name()].set_data(block->read_integer());
                break;
                case CONFIG_LINE::STRING :
                    attack_lib[block->get_name()].set_data(block->read_string());
                break;
            }
            block->next();
        }
    }
}
