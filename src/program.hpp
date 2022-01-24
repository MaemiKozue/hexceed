#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include "image.hpp"
#include "input.hpp"

#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>

typedef enum capture_method {
    CAPTURE_GDI,
    CAPTURE_CAMERA
} capture_method;


// Assuming a Windows program
// Assuming video input from default camera
class Program
{
private:
    std::string name;
    cv::VideoCapture capture;
    capture_method cap_method;

public:
    Program(std::string name);

    cv::Mat screenshot();
    bool input(Input in) const;
    void set_method(capture_method method);

    void loop() const;
};



#endif // PROGRAM_H
