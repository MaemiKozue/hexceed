#ifndef HEXGRID_H
#define HEXGRID_H

#include <unordered_map>
#include <unordered_set>

#include <opencv2/core/mat.hpp>

using namespace std;
using namespace cv;


class wall_hash {
private:
public:
    size_t operator()(pair<pair<int, int>, pair<int, int>> p) const;
};


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


    class Wall
    {
    private:
    public:
        pair<int, int> a;
        pair<int, int> b;

        Wall(int qa, int ra, int qb, int rb);
    };

    unordered_map<pair<int, int>, Cell> grid;
    unordered_set<pair<pair<int, int>, pair<int, int>>, wall_hash> walls;

public:
    // hexgrid();

    // Cells
    void add(int q, int r, T& data);
    void remove(int q, int r);
    bool has(int q, int r);
    bool has(pair<int, int> p);


    T& get(int q, int r);
    bool get(int q, int r, T& data);
    void set(int q, int r, T& data);


    // Walls
    void add_wall(int qa, int ra, int qb, int rb);
    bool has_wall(int qa, int ra, int qb, int rb);
    void remove_wall(int qa, int ra, int qb, int rb);


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


size_t wall_hash::operator()(pair<pair<int, int>, pair<int, int>> p) const {
    size_t a = hash<pair<int, int>>()(p.first);
    size_t b = hash<pair<int, int>>()(p.second);
    size_t h = (a+b) * (a+b+1) / 2 + b;
    return h;
}


#include "hexgrid_impl.hpp"

#endif // HEXGRID
