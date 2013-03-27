/*
 * This is a class that will remember a certain number of items
 * and forget older ones as you remember new ones.
 *
 *   Forget<int> f(2);
 *   f.remember(1);
 *   f.remember(2);
 *   f.remember(3); // 1 is now gone
 *   
 *   for (int i : f)
 *       std::cout << i << std::endl;
 */

#ifndef H_FORGET
#define H_FORGET

#include <vector>

template<class Type>
class Forget
{
public:
    typedef typename std::vector<Type>::size_type size_type;
    typedef typename std::vector<Type>::const_iterator const_iterator;

private:
    int ct = 0;
    size_type sz = 0;
    std::vector<Type> items;

public:
    Forget(int sz, Type default_value = Type())
        :sz(sz), items(sz, default_value) { }

    // Standard functions
    size_type size() const { return sz; }
    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end();   }

    // Number of times we've remembered something
    int count() const { return ct; }

    // If we want to do anything with these
    std::vector<Type> dump() const { return items; }

    // Remember new item and forget oldest item
    void remember(const Type& item)
    {
        ++ct;

        // Shift all items back one
        for (size_type i = 1; i < sz; ++i)
            items[i-1] = items[i];
        
        // Put this item at the end
        items[sz-1] = item;
    }
};

#endif
