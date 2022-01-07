#include <iostream>
#include <string>
#include <cstdlib>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <filesystem>

namespace fs = std::filesystem;

int main (int argc, char* argv[]) {
    std::cout << "cwd: " << fs::current_path() << std::endl;
    std::string filename;
    if (argc == 2) {
        filename = argv[1];
        std::cout << "Name: " << filename << std::endl;
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);

    if (img.empty()) {
        std::cout << "Unable to read image: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    cv::imshow("View", img);

    int k = cv::waitKey(0);
    if (k == 's')
    {
        fs::path dir = fs::path("out/");
        fs::path file = fs::path(filename).filename();
        fs::path out = dir/file;
        std::string path = out.string();
        std::cout << "Saving in: " << path << std::endl;
        cv::imwrite(path, img);
    }

    return EXIT_SUCCESS;
}