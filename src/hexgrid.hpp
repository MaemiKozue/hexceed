#ifndef HEXGRID_H
#define HEXGRID_H

#include <unordered_map>

#include <opencv2/core/mat.hpp>

using namespace std;
using namespace cv;

template<typename T>
class Hexgrid
{
private:
    class Position
    {
    private:

    public:
        int q, r, s;

        Position(int q, int r): q(q), r(r), s(-q-r) {;;}
        Position(int q, int r, int s): q(q), r(r), s(s) {;;}
        bool valid() const { return q+r+s == 0; }
    };

    class Cell
    {
    private:
    public:
        T data;
        int q;
        int r;
        Cell* v[6];


        Cell(int q, int r, T& data);
        Cell();

        void link(Hexgrid<T>::Cell& other);
        void link(int i, Cell *other);
        void unlink(int i);
        void unlink_all();
    };

    unordered_map<std::pair<int, int>, Cell> grid;

public:
    // hexgrid();
    void add(int q, int r, T& data);
    void remove(int q, int r);
    bool has(int q, int r);
    bool has(pair<int, int> p);


    T& get(int q, int r);
    bool get(int q, int r, T& data);
    void set(int q, int r, T& data);

    void Hexgrid<T>::draw(Mat& out, Scalar color = Scalar(255, 0, 0, 0), int size = 50, int padding = 0);
};


namespace std {
    template<>
    class hash<std::pair<int, int>>
    {
    private:

    public:
        size_t operator()(pair<int, int> p) const {
            size_t a = hash<int>()(p.first);
            size_t b = hash<int>()(p.second);
            // Cantor pairing
            size_t h = (a+b) * (a+b+1) / 2 + b;
            return h;
        }
    };
}

#include "hexgrid_impl.hpp"

#endif // HEXGRID
