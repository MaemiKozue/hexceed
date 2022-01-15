#include <iostream>
#include "input.hpp"
#include "program.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    cout << "argc: " << argc << endl;
    for (size_t i = 0; i < argc; i++)
    {
        cout << "[" << i << "] " << argv[i] << endl;
    }
    
    Input in(MB_LEFT, 40, 80);
    Input second(KEY_ENTER);
    cout << in << endl;
    cout << second << endl;

    Program prog("hexceed");
    cout << prog.input(in) << endl;
    // prog.screenshot();
    prog.loop();
    Image im = prog.screenshot();
    return 0;
}
