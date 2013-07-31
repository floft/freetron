/*
 * Data stuff that needs to be used most places
 */

#ifndef H_DATA
#define H_DATA

#include <iostream>
#include <vector>

struct Coord
{
    int x = 0;
    int y = 0;

    Coord() { }
    Coord(const int x, const int y) :x(x), y(y) { }

    Coord& operator+=(const Coord& c);
    Coord operator+(const Coord& c) const;
};

static const Coord default_coord(-1, -1);

// A function object (functor) for sorting points by X position
struct CoordXSort
{
    inline bool operator() (const Coord& p1, const Coord& p2)
    {
         return p1.x < p2.x;
    }
};

struct Data
{
    // The approximate width of the box
    int width = 0;

    // The diagonal of the first used box
    int diag  = 0;
};

// I have never seen one with more than 5 options
enum class Answer
{
    Blank = 0, A, B, C, D, E
};

// Return type for threads
struct Info
{
    int thread_id = 0;
    int id = 0;
    std::vector<Answer> answers;

    Info() { }
    Info(int t) :thread_id(t) { }
    Info(int t, int i, const std::vector<Answer>& answers)
        :thread_id(t), id(i), answers(answers) { }
};

std::ostream& operator<<(std::ostream& os, const Coord& c);
std::istream& operator>>(std::istream& is, Coord& c);
bool operator==(const Coord& c1, const Coord& c2);
bool operator!=(const Coord& c1, const Coord& c2);

// Less than: Is the y value less? If the same, is the x value less?
bool operator<(const Coord& c1, const Coord& c2);
bool operator>(const Coord& c1, const Coord& c2);

std::ostream& operator<<(std::ostream& os, const Answer& c);

#endif
