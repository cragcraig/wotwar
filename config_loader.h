#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include <fstream>
#include <string>
#include <sstream>

#include "sizes.h"

class CONFIG_LINE
{
    public:
        enum TYPE {NONE=0, STRING, INTEGER, DOUBLE};
        CONFIG_LINE::TYPE data_type;

        // data
        std::string d_str;
        int d_integer;
        double d_double;

        // constructors
        CONFIG_LINE(char * s);
        CONFIG_LINE(std::string& s);
        CONFIG_LINE(int i);
        CONFIG_LINE(double d);
};

class CONFIG_BLOCK
{
    private:
        int pos;
    public:
        std::string name, type;
        std::vector<CONFIG_LINE> data;
        const char * get_name(void);
        CONFIG_BLOCK(void);
        CONFIG_BLOCK(std::string& name, std::string& type);

        bool next(void);
        CONFIG_LINE::TYPE line_type(void);
        const char * read_string(void);
        int read_integer(void);
        double read_double(void);
};

class CONFIG_FILE
{
    private:
        std::ifstream fp;
        std::stringstream line;
        std::string filename;
        char cur_line[1024];
        bool started;

        void find_nonempty_line(void);
        bool load_section(void);
        bool load_all(void);

        std::map<std::string, CONFIG_BLOCK, cfg_cmp>::iterator pos;
        std::map<std::string, CONFIG_BLOCK, cfg_cmp> blocks;

    public:
        CONFIG_FILE(const char * file);
        const char * get_filename(void);
        int size(void);

        CONFIG_BLOCK* start();
        CONFIG_BLOCK* current();
        CONFIG_BLOCK* next();
        CONFIG_BLOCK& get(std::string type, std::string name);
};

CONFIG_LINE::TYPE find_data(char* str, char*& ps);
const char * find_fchar(const char * s);

#endif
