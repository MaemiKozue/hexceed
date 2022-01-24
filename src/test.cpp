#include <iostream>
#include <string>
#include "input.hpp"
#include "program.hpp"


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define TARGET_FPS 144

using namespace std;
using namespace cv;


static void onMouse(int event, int x, int y, int , void *data)
{
    if (event == EVENT_LBUTTONDOWN) {
        Program& prog = *reinterpret_cast<Program*>(data);
        prog.input(Input(MB_LEFT, x, y));
    }
}


int main(int argc, char const *argv[])
{
    cout << "argc: " << argc << endl;
    for (size_t i = 0; i < argc; i++)
    {
        cout << "[" << i << "] " << argv[i] << endl;
    }

    if (argc <= 1) {
        cout << "Usage: " << argv[0] << " <window_name>" << endl;
        return EXIT_FAILURE;
    }

    const int target_fps = TARGET_FPS;
    int frame_delay = static_cast<int>(1000. / target_fps);
    string window_name = argv[1];

    Input in(MB_LEFT, 40, 80);
    Input second(KEY_ENTER);
    cout << in << endl;
    cout << second << endl;

    Program prog(window_name);
    cout << prog.input(in) << endl;

    string window_title = "View";
    namedWindow(window_title);
    setMouseCallback(window_title, onMouse, &prog);

    try
    {
        int k;
        bool quit = false;
        do {
            Mat frame = prog.screenshot();
            if (!frame.empty()) {
                imshow(window_title, frame);
                k = waitKey(frame_delay);
                quit = k == 'q';
            }
            else {
                waitKey(2000);
            }
        } while (!quit);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
