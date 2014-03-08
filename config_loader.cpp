#include <cstdlib>
#include "config_loader.h"

// create a config line
CONFIG_LINE::CONFIG_LINE(char * s) : d_str(s), data_type(CONFIG_LINE::STRING) {}
CONFIG_LINE::CONFIG_LINE(std::string& s) : d_str(s), data_type(CONFIG_LINE::STRING) {}
CONFIG_LINE::CONFIG_LINE(int i) : d_integer(i), data_type(CONFIG_LINE::INTEGER) {}
CONFIG_LINE::CONFIG_LINE(double d) : d_double(d), data_type(CONFIG_LINE::DOUBLE) {}

// create a block
CONFIG_BLOCK::CONFIG_BLOCK(void) : pos(0) {}
CONFIG_BLOCK::CONFIG_BLOCK(std::string& name, std::string& type) : pos(0), name(name), type(type) {}

CONFIG_LINE::TYPE find_data(char* str, char*& ps)
{
    int i = 0;
    char t;
    while (str[i] != '\'' && str[i] != '"' && str[i] != ':') i++; // find start of data
    t = str[i];
    ps = &str[++i];
    while (str[i] != t && str[i]) i++; // find end of data
    str[i] = '\0';

    // return datatype
    if (t == '\'') return CONFIG_LINE::INTEGER;
    else if (t == ':') return CONFIG_LINE::DOUBLE;
    else return CONFIG_LINE::STRING;
}

const char * CONFIG_BLOCK::get_name(void)
{
    return name.c_str();
}

bool CONFIG_BLOCK::next(void)
{
    pos++;
    if (pos < data.size()) return true;
    else return false;
}

CONFIG_LINE::TYPE CONFIG_BLOCK::line_type(void)
{
    if (pos >= data.size()) return CONFIG_LINE::NONE;
    else return data[pos].data_type;
}

const char * CONFIG_BLOCK::read_string(void)
{
    return data[pos].d_str.c_str();
}

int CONFIG_BLOCK::read_integer(void)
{
    return data[pos].d_integer;
}

double CONFIG_BLOCK::read_double(void)
{
    return data[pos].d_double;
}

CONFIG_FILE::CONFIG_FILE(const char * file) : fp(file), filename(file), started(false)
{
    if (!fp.is_open()) printf("warning: missing config file %s!\n", file);
    else if (load_all()) printf("success: %s loaded\n", file);
    else printf("warning: problem loading %s\n", file);

    if (fp.is_open() && !size()) printf("warning: no data blocks found in %s\n", file);
}

const char * find_fchar(const char * s)
{
    while (isspace(*s)) s++;
    if (*s == '\n' || *s == '\0') return NULL;
    else return s;
}

void CONFIG_FILE::find_nonempty_line(void)
{
    const char * fchar;
    do {
        fp.getline(cur_line, 1024);
        fchar = find_fchar(cur_line);
    } while ((!fchar || *fchar == '#') && !fp.eof());

    this->line.str(std::string(cur_line)); // place into a stringstream
}

bool CONFIG_FILE::load_section(void)
{
    bool in_section = false;
    std::string name, type, id;
    char * data_str;
    CONFIG_LINE::TYPE dtype;

    while (1) {
        find_nonempty_line();
        if (fp.eof()) {
            if (in_section) printf("warning: incomplete '%s' config block\n", name.c_str());
            return false; // end of file
        }
        if (in_section && !line.str().compare(0, 3, "end", 0, 3)) break; // end of block

        // find block start
        if (!in_section) {
            line >> type;
            find_data(cur_line, data_str);
            name = data_str;
            id = type + "_" + name;
            // add new block
            blocks[id] = CONFIG_BLOCK(name, type);
            in_section = true;
        }
        // in block
        else {
            dtype = find_data(cur_line, data_str);
            switch (dtype) {
                case CONFIG_LINE::INTEGER :
                    blocks[id].data.push_back(CONFIG_LINE(atoi(data_str)));
                    break;
                case CONFIG_LINE::DOUBLE :
                    blocks[id].data.push_back(CONFIG_LINE(atof(data_str)));
                    break;
                case CONFIG_LINE::STRING :
                    blocks[id].data.push_back(CONFIG_LINE(data_str));
                    break;
            }
        }
    }

    return true;
}

bool CONFIG_FILE::load_all(void)
{
    if (!fp.is_open()) return false;

    while (load_section()) ; // load all blocks
    return true;
}

int CONFIG_FILE::size(void)
{
    return blocks.size();
}

CONFIG_BLOCK& CONFIG_FILE::get(std::string type, std::string name)
{
    std::string id = type + "_" + name;
    return blocks[id];
}

const char * CONFIG_FILE::get_filename(void)
{
    return filename.c_str();
}

CONFIG_BLOCK* CONFIG_FILE::start()
{
    started = true;
    pos = blocks.begin();
    return current();
}

CONFIG_BLOCK* CONFIG_FILE::current()
{
    if (pos != blocks.end()) return &pos->second;
    else return NULL;
}

CONFIG_BLOCK* CONFIG_FILE::next()
{
    if (started) {
        pos++;
        return current();
    } else return start();
}
