#include "hexgrid.hpp"

#include <vector>
#include <utility>

#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;


static int direction(int qa, int ra, int qb, int rb)
{
    int dir = -1;
    int dq = qb - qa;
    int dr = rb - ra;

    if (dq * dq > 1 || dr * dr > 1 || dq == dr) {
        dir = -1;
    }
    else if (dq ==  1) {
        dir = dr == 0 ? 1 : 0;
    }
    else if (dq ==  0) {
        dir = dr == 1 ? 2 : 5;
    }
    else if (dq == -1) {
        dir = dr == 0 ? 4 : 3;
    }
    else {
        // cerr << "Non standard direction (" << dq << ", " << dr << ")" << endl;
    }

    return dir;
}

static int direction(pair<int, int> a, pair<int, int> b)
{
    return direction(a.first, a.second, b.first, b.second);
}


/* --------- Cell --------- */

template<typename T>
Hexgrid<T>::Cell::Cell(int q, int r, T& data) :
    v{},
    q(q),
    r(r),
    data(data)
{
    ;;
}


template<typename T>
Hexgrid<T>::Cell::Cell()
{
    ;;
}


template<typename T>
void Hexgrid<T>::Cell::link(Hexgrid<T>::Cell& other)
{
    int dir = direction(this->q, this->r, other.q, other.r);
    // Error
    if (dir < 0) {
        return;
    }

    this->link(dir, &other);
}


template<typename T>
void Hexgrid<T>::Cell::link(int i, Hexgrid<T>::Cell *other)
{
    if (other != nullptr) {
        this->v[i] = other;
        other->v[(i+3)%6] = this;
    }
}


template<typename T>
void Hexgrid<T>::Cell::unlink(int i)
{
    if (this->v[i] != nullptr) {
        this->v[i].v[(i+3) % 6] = nullptr;
        this->v[i] = nullptr;
    }
}


template<typename T>
void Hexgrid<T>::Cell::unlink_all()
{
    for (size_t i = 0; i < 6; i ++) {
        this->unlink(i);
    }
}


/* --------- Hexgrid --------- */


/* Hexgrid Cells */

vector<pair<int, int>> neighbors(int q, int r)
{
    return {
        {q+1, r},
        {q-1, r},
        {q, r+1},
        {q, r-1},
        {q-1, r+1},
        {q+1, r-1}
    };
}


vector<pair<int, int>> neighbors(pair<int, int> p)
{
    return neighbors(p.first, p.second);
}


template<typename T>
void Hexgrid<T>::add(int q, int r, T& data)
{
    pair<int, int> pos{q, r};
    if (this->has(pos)) {
        this->grid[pos].data = data;
    }
    else {
        this->grid[pos] = Cell(q, r, data);
        Cell &cell = this->grid[pos];

        for (auto& k : neighbors(pos)) {
            if (this->has(k)) {
                cell.link(this->grid[k]);
            }
        }

    }
}


template<typename T>
void Hexgrid<T>::remove(int q, int r)
{
    if (!this->has(q, r)) {
        return;
    }
    else {
        Cell& cell this->grid[{q, r}];
        cell.unlink_all();
        this->grid.erase({q, r});
    }
}


template<typename T>
bool Hexgrid<T>::has(pair<int, int> p)
{
    return this->has(p.first, p.second);
}


template<typename T>
bool Hexgrid<T>::has(int q, int r)
{
    return this->grid.find({q, r}) != this->grid.end();
}



template<typename T>
T& Hexgrid<T>::get(int q, int r)
{
    if (this->has(q, r)) {
        return this->grid[{q, r}];
    }
    else {
        stringstream ss;
        ss << "Cell (" << q << ", " << r << ") does not exist";
        throw out_of_range(ss.str());
    }
}


template<typename T>
bool Hexgrid<T>::get(int q, int r, T& data)
{
    if (this->has(q, r)) {
        data = this->grid[{q, r}];
        return true
    }
    else {
        return false;
    }
}


template<typename T>
void Hexgrid<T>::set(int q, int r, T& data)
{
    if (this->has(q, r)) {
        this->grid[{q, r}] = data;
    }
    else {
        this->add(q, r, data);
    }
}


/* Hexgrid Walls */

// This makes sure coordinates of A is lexically lower than of B
static void normalize(int& qa, int& ra, int& qb, int& rb)
{
    using std::swap;

    if (qb > qa) {
        swap(qa, qb);
        swap(ra, rb);
    }
    else if (qa == qb) {
        if (rb > ra) {
            swap(ra, rb);
        }
    }
}


template<typename T>
void Hexgrid<T>::add_wall(int qa, int ra, int qb, int rb)
{
    normalize(qa, ra, qb, rb);
    this->walls.insert({{qa, ra}, {qb, rb}});
}


template<typename T>
bool Hexgrid<T>::has_wall(int qa, int ra, int qb, int rb)
{
    normalize(qa, ra, qb, rb);
    return this->walls.contains({{qa, ra}, {qb, rb}});
}


template<typename T>
void Hexgrid<T>::remove_wall(int qa, int ra, int qb, int rb)
{
    normalize(qa, ra, qb, rb);
    this->walls.erase({{qa, ra}, {qb, rb}});
}


/* Hexgrid drawing */


static void draw_centered_text(Mat& out, string s, int x, int y, Scalar color, double scale, int font = FONT_HERSHEY_SIMPLEX)
{
    Size size = getTextSize(s, font, scale, 1, nullptr);
    Point pos = Point((int) (x - size.width / 2.),
                      (int) (y + size.height/ 2.));
    putText(out, s, pos, font, scale, color);
}


template<typename T>
static void draw_neighbors(Mat& out, int x, int y, T& c, Scalar color, int r)
{
    double a = 1. / 2.;
    double b = sqrt(3) / 2.;
    double px =  a*b*r*.8;
    double py = -b*b*r*.8;
    // double rotate[][] = {
    //     {a, -b},
    //     {b,  a}
    // };
    for (size_t i = 0; i < 6; i++)
    {
        stringstream ss;
        if (c.v[i] != nullptr) {
            ss << c.v[i]->q << "," << c.v[i]->r;
        }
        else {
            ss << ".";
        }
        string text = ss.str();
        int text_x = (int) (x+px);
        int text_y = (int) (y+py);
        draw_centered_text(out, text, text_x, text_y, color, 0.005*r);

        double next_px, next_py;
        next_px = a * px - b * py;
        next_py = b * px + a * py;
        px = next_px;
        py = next_py;
    }
}


static void draw_hex(Mat& out, int x, int y, Scalar color = Scalar::all(0), int r = 100)
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
    polylines(out, pts, true, color);
}


template<typename T>
static void draw_data(Mat& out, int x, int y, T data, Scalar color = Scalar::all(0), double scale = 1)
{
    stringstream ss;
    ss << data;
    string label = ss.str();
    Size label_size = getTextSize(label, FONT_HERSHEY_SIMPLEX, scale, 1, nullptr);
    Point label_pos = Point((int) (x - label_size.width / 2.),
                            (int) (y + label_size.height/ 2.));
    putText(out, label, label_pos, FONT_HERSHEY_SIMPLEX, scale, color);
}


static void draw_label(Mat& out, int x, int y, int q, int r, Scalar color, double scale)
{
    stringstream ss;
    ss << "(" << q << ", " << r << ")";
    string label = ss.str();
    Size label_size = getTextSize(label, FONT_HERSHEY_SIMPLEX, scale, 1, nullptr);
    Point label_pos = Point((int) (x - label_size.width / 2.),
                            (int) (y + 2.875 * label_size.height));
    putText(out, label, label_pos, FONT_HERSHEY_SIMPLEX, scale, color);
}


static void draw_wall(Mat& out, int x, int y, int dir, int size, Scalar color, int thickness)
{
    double vx, vy;
    switch (dir % 3)
    {
    case 0: {
        vx = sqrt(3) / 2.0;
        vy = 0.5;
        break;
    }
    case 1: {
        vx = 0;
        vy = 1;
        break;
    }
    case 2: {
        vx = sqrt(3) / 2.0;
        vy = - 0.5;
        break;
    }

    default:
        vx = 0;
        vy = 0;
        break;
    }


    Point a = Point((int) (x - vx * size / 2.0), (int) (y - vy * size / 2.0));
    Point b = Point((int) (x + vx * size / 2.0), (int) (y + vy * size / 2.0));
    // circle(out, Point(x, y), 5, color);
    line(out, a, b, color, thickness);
}


template<typename T>
void Hexgrid<T>::draw(Mat& out, Scalar color, int size, int padding)
{
    int l = size;
    int p = padding;
    int wall_thickness = 3;
    if (l < 0 || p < 0) return;
    double sqrt_3 = sqrt(3);
    int ox = (int) (out.cols / 2.);
    int oy = (int) (out.rows / 2.);
    int w = (int) (sqrt_3 * (l+p));
    int w2 = (int) (sqrt_3 * (l+p) / 2.);
    int h = (int) ((l+p) * 3. / 2.);

    // Cell drawing
    for (auto& pr : this->grid) {
        auto& pos = pr.first;
        Cell& cell = pr.second;

        int q = pos.first == cell.q ? pos.first : -1;
        int r = pos.second == cell.r ? pos.second : -1;

        int x = ox + w * (q + r/2) + (r%2 * w2); // on purpose using r%2 == -1/0/1
        int y = oy + h * r;

        circle(out, Point(x, y), 1, color, FILLED);
        draw_hex(out, x, y, color, l);
        draw_data(out, x, y, cell.data, color, l/50.);
        draw_neighbors(out, x, y, cell, color, l);
        draw_label(out, x, y, q, r, color, 0.375*l/50.);
    }

    // Wall drawing
    for (auto& wall : this->walls) {
        auto& pa = wall.first;
        auto& pb = wall.second;

        int qa = pa.first;
        int ra = pa.second;
        int qb = pb.first;
        int rb = pb.second;

        int xa = ox + w * (qa + ra/2) + (ra%2 * w2);
        int ya = oy + h * ra;
        int xb = ox + w * (qb + rb/2) + (rb%2 * w2);
        int yb = oy + h * rb;
        int x = (xa + xb) / 2;
        int y = (ya + yb) / 2;

        // circle(out, Point(xa, ya), 2, Scalar(0, 255, 0), FILLED);
        // circle(out, Point(xb, yb), 2, Scalar(0, 255, 0), FILLED);
        // line(out, Point(xa, ya), Point(xb, yb), Scalar(0, 255, 0));

        int dir = direction(pa, pb);
        draw_wall(out, x, y, dir, l, Scalar(0, 0, 255), wall_thickness);
    }
}

