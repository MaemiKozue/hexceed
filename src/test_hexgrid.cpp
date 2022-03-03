#include <iostream>
#include <random>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "hexgrid.hpp"

using namespace std;
using namespace cv;


void draw_hex(Mat& img, int x, int y, int r = 100)
{
    double sqrt_3 = sqrt(3);
    int c = (int) (r*sqrt_3/2.);
    int s = (int) (r/2.);
    Mat pts = (Mat_<int>(6, 2) <<
        x, y+r,
        x+c, y+s,
        x+c, y-s,
        x, y-r,
        x-c, y-s,
        x-c, y+s
    );
    polylines(img, pts, true, Scalar(0, 0, 255, 0));
}


void draw_hexgrid(Mat& img, int w, int h, int r = 100, int p = 0)
{
    if (r < 0 || p < 0) return;
    double sqrt_3 = sqrt(3);
    int xw = (int) ((r+p)*sqrt_3);
    int x = 0;
    int y = 0;
    bool even = true;
    while (y <= h)
    {
        while (x <= w)
        {
            draw_hex(img, x, y, r);

            x += xw;
        }
        even = !even;
        x = even ? 0 : (int) (xw / 2.);
        y += (int) ((r+p) / 2. * 3.);
    }
}


int main(int argc, char const *argv[])
{
    (void) argc;
    (void) argv;

    Hexgrid<int> grid;

    random_device rdev;
    default_random_engine reng(rdev());
    int range = 3;
    int value_range = 10;
    uniform_int_distribution<int> uniform(-value_range, value_range);
    normal_distribution<> normal(0, range/2.);
    for (int i = 0; i < 20; i++) {
        int x = static_cast<int>(round(normal(reng)));
        int y = static_cast<int>(round(normal(reng)));
        int v = uniform(reng);
        grid.add(x, y, v);
    }

    for (int i = 0; i < 6; i++) {
        int x1 = static_cast<int>(round(normal(reng)));
        int y1 = static_cast<int>(round(normal(reng)));
        int x2 = static_cast<int>(round(normal(reng)));
        int y2 = static_cast<int>(round(normal(reng)));
        grid.add_wall(x1, y1, x2, y2);
    }

    int w = 512;
    int h = 512;
    int k;
    int size = 50;
    int padding = 0;
    Mat bg(h, w, CV_8UC4, Scalar(255, 255, 255, 0));
    bool quit = false;
    bool restart = false;
    while (!quit) {
        k = waitKey(10);
        Mat grid_layer(h, w, CV_8UC4, Scalar(0, 0, 0, 255));
        Mat out(h, w, CV_8UC4);

        if (k == '+') {
            size++;
        }
        else if (k == '-') {
            size--;
            if (size < 1) size = 1;
        }
        else if (k == 'p') {
            padding++;
        }
        else if (k == 'm') {
            padding--;
            if (padding < 0) padding = 0;
        }
        else if (k == 'q') {
            quit = true;
        }
        else if (k == 'r') {
            quit = true;
            restart = true;
        }

        // draw_hexgrid(grid_layer, w, h, size, padding);
        grid.draw(grid_layer, Scalar(255, 0, 0, 0), size, padding);


        bg.copyTo(out);

        Mat grid_gray, mask;
        cvtColor(grid_layer, grid_gray, COLOR_BGR2GRAY);
        threshold(grid_gray, mask, 1, 255, THRESH_BINARY);
        grid_layer.copyTo(out, mask);

        // imshow("Grid", grid_layer);
        // imshow("Mask", mask);
        imshow("Hexgrid", out);
    }

    if (restart) {
        return main(argc, argv);
    }
    else {
        return EXIT_SUCCESS;
    }
}
