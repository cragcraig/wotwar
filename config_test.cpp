#include "config_loader.h"

#include <iostream>

using namespace std;

int main(void)
{
    CONFIG_FILE cf("data/test.dat");
	
	CONFIG_BLOCK * block = cf.start();
	
	do {
	    cout << endl << block->get_name() << endl;
	    // load data
	    while (block->line_type()) {
	        switch (block->line_type()) {
                case CONFIG_LINE::INTEGER :
                    cout << block->read_integer() << endl;
                break;
                case CONFIG_LINE::STRING :
                    cout << block->read_string() << endl;
                break;
	        }
	        block->next();
	    }
	} while (block = cf.next());

    return 0;
}
