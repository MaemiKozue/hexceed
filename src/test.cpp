#include <iostream>
#include "input.hpp"
#include "program.hpp"


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define TARGET_FPS 144

using namespace std;

int main(int argc, char const *argv[])
{
    cout << "argc: " << argc << endl;
    for (size_t i = 0; i < argc; i++)
    {
        cout << "[" << i << "] " << argv[i] << endl;
    }

    const int target_fps = TARGET_FPS;
    int frame_delay = static_cast<int>(1000. / target_fps);
    
    Input in(MB_LEFT, 40, 80);
    Input second(KEY_ENTER);
    cout << in << endl;
    cout << second << endl;

    Program prog("hexceed");
    cout << prog.input(in) << endl;
    // prog.screenshot();
    // prog.loop();
    try
    {
        int k;
        do {
            Image ss = prog.screenshot();
            if (ss.get() != NULL) {
                cv::Mat frame(ss.height, ss.width, CV_8UC4, ss.get());
                cv::imshow("View", frame);
            }
        } while ((k = cv::waitKey(frame_delay)) != 'q');
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    

    return 0;
}
