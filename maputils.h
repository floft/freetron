/*
 * Utilities for using maps
 */

#ifndef H_MAPUTILS
#define H_MAPUTILS

#include <map>

#include "data.h"

// Compare map keys
template<class T>
class mapKeyCompare
{
public:
    bool operator()(const T& a, const T& b)
    {
        return (a.first < b.first);
    }
};

// Compare map values
template<class T>
class mapValueCompare
{
public:
    bool operator()(const T& a, const T& b)
    {
        return (a.second < b.second);
    }
};

// Return the value of the minimum key in a map
//  Coord c = mapMinValue<Coord, std::pair<double, Coord>>(m.begin(), m.end())
template<class Result, class Type, class Iter>
inline Result mapMinValue(Iter start, Iter end)
{
    Iter min = std::min_element(start, end, mapKeyCompare<Type>());

    if (min != end)
        return min->second;
    else
        return Result();
}

// Return the key of the max value in a map
//  Direction d = mapMaxValueKey<Direction, std::pair<Direction, int>>(m.begin(), m.end())
template<class Result, class Type, class Iter>
inline Result mapMaxValueKey(Iter start, Iter end)
{
    Iter max = std::max_element(start, end, mapValueCompare<Type>());

    if (max != end)
        return max->first;
    else
        return Result();
}

// Count the number of times a value is in a map
template<class Type, class Value> int mapCountValue(Type m, Value val)
{
    typedef typename Type::iterator iter;

    int number = 0;

    for (iter i = m.begin(); i != m.end(); ++i)
        if (i->second == val)
            ++number;
    
    return number;
}

// A map value const_iterator, so we can easily do the following without messing
// with map->first or map->second.
//   for (const CoordPair& p : blobs) ...
//
// Based on: http://stackoverflow.com/a/7667618
template<class Type> class MapValueIterator 
{
    typedef typename Type::mapped_type Value;
    
    typename Type::const_iterator iter;

public:
    //typedef Value value_type;
    //typedef Value* pointer;
    //typedef Value& reference;
    //typedef MapValueIterator<Type> difference_type;
    //typedef std::forward_iterator_tag iterator_category;

    MapValueIterator(typename Type::const_iterator iter) :iter(iter) { }
    MapValueIterator(const MapValueIterator<Type>& i) :iter(i.iter) { }

    const Value& operator*() const
    {
        return iter->second;
    }
    
    const Value* const operator->() const
    {
        return &iter->second;
    }

    MapValueIterator& operator++()
    {
        ++iter;

        return *this;
    }

    MapValueIterator& operator=(const MapValueIterator& i)
    {
        iter = i.iter;

        return *this;
    }
    
    bool operator==(const MapValueIterator& i) const
    {
        return iter == i.iter;
    }

    bool operator!=(const MapValueIterator& i) const
    {
        return iter != i.iter;
    }
};

#endif
