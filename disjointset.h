/*
 * A disjoint set (or union find) data structure implementation
 *   See: https://en.wikipedia.org/wiki/Disjoint-set_data_structure
 *
 * Usage
 *   DisjointSet<int> ds(0); // Default, not-found element
 *   ds.add(5);
 *   ds.add(6);
 *   ds.add(7);
 *   ds.join(5, 6);
 *
 *   int rep = ds.find(6);
 *
 *   if (rep != ds.notfound())
 *       // 6 points to rep
 *
 *   DisjointSet<int> ds2(std::move(ds));
 *
 * Construct: pass in element to be returned when an element isn't found, add
 *            will throw an error if you try adding this item
 * Add:       insert a new item if it doesn't already exist
 * Join:      join the two sets that these two items belong to, they don't have
 *            to be the representative elements
 * Find:      find the representative element of the set containing this item,
 *            returns this.notfound() if not found
 *
 * Notes
 *   This is meant to be used with small elements since it'll return one of
 *   them as the representative element. If you have large items you're
 *   storing, you'll probably want a different implementation that has some
 *   sort of separate index for the items.
 *
 *   This requires that the elements be sortable. This is so that we can use
 *   std::set and its faster-than-linear-time find capabilities.
 *
 *   This is primarily designed for use in a connected-component labeling
 *   algorithm using something like short (small images) or int (larger images)
 *   as the data type.
 */

#ifndef H_DISJOINTSET
#define H_DISJOINTSET

#include <set>
#include <iostream>

// Thrown if default element is added
class ElementIsDefault { };

// Thrown if there are internal errors
class DisjointItemWrongParent { };
class DisjointSetParentNull { };
class DisjointItemNotEmpty { };
class DisjointItemNotInParent { };
class DisjointSetItemNotFound { };

// Each item stored not only contains the element itself but a reference to the
// representative element, and if it is the representative element, a list to
// all those that point to it
template<class Type>
struct DisjointSetItem
{
    typedef typename std::set<DisjointSetItem<Type>>::iterator set_iter;
    typedef typename std::set<DisjointSetItem<Type>>::const_iterator set_const_iter;
    typedef typename std::set<DisjointSetItem<Type>*>::iterator list_iter;
    typedef typename std::set<DisjointSetItem<Type>*>::const_iterator list_const_iter;

    // The item
    Type value;

    // The representative item for this set
    DisjointSetItem<Type>* rep;

    // If this is the representative item (rep == this), then we need to keep
    // track of all the items in this set. Otherwise, this will be empty.
    std::set<DisjointSetItem<Type>*> list;

    // New item in new set, representative is itself
    DisjointSetItem(Type value)
        : value(value), rep(this)
    { }

    // Allow moving, keep the rep pointing to this item if it previously was
    DisjointSetItem(DisjointSetItem<Type>&&);

    bool operator<(const DisjointSetItem<Type>& elem)  const { return value < elem.value; }
    bool operator>(const DisjointSetItem<Type>& elem)  const { return value > elem.value; }
    bool operator<=(const DisjointSetItem<Type>& elem) const { return value <= elem.value; }
    bool operator>=(const DisjointSetItem<Type>& elem) const { return value >= elem.value; }
    bool operator==(const DisjointSetItem<Type>& elem) const { return value == elem.value; }
    bool operator!=(const DisjointSetItem<Type>& elem) const { return value != elem.value; }

private:
    // Disallow copying (not yet implemented)
    DisjointSetItem(const DisjointSetItem<Type>&);
    DisjointSetItem<Type>& operator=(const DisjointSetItem<Type>&);
};

// The set of sets
template<class Type>
class DisjointSet
{
    typedef typename std::set<DisjointSetItem<Type>>::iterator set_iter;
    typedef typename std::set<DisjointSetItem<Type>>::const_iterator set_const_iter;
    typedef typename std::set<DisjointSetItem<Type>*>::iterator list_iter;
    typedef typename std::set<DisjointSetItem<Type>*>::const_iterator list_const_iter;

    // The set of all sets
    std::set<DisjointSetItem<Type>> sets;

    // Default, not found item. Public but constant.
    Type defaultElem;

public:
    DisjointSet(Type notfound)
        : defaultElem(notfound)
    { }

    // Allow moving, just std::move everything
    DisjointSet<Type>(DisjointSet<Type>&&);
    DisjointSet<Type>& operator=(DisjointSet<Type>&&);

    // Main operations
    void add(Type elem);
    void join(Type elem1, Type elem2);
    Type find(Type elem) const;
    Type notfound() const { return defaultElem; }

    // Debugging
    void selfcheck() const;

private:
    // Disallow copying (not yet implemented)
    DisjointSet<Type>(const DisjointSet<Type>&);
    DisjointSet<Type>& operator=(const DisjointSet<Type>&);

    template<class T>
    friend std::ostream& operator<<(std::ostream&, const DisjointSet<T>&);
    template<class T>
    friend std::ostream& operator<<(std::ostream&, const DisjointSetItem<T>&);
};

// For debugging
template<class Type>
std::ostream& operator<<(std::ostream&, const DisjointSet<Type>&);
template<class Type>
std::ostream& operator<<(std::ostream&, const DisjointSetItem<Type>&);

//
// Implementation
//
template<class Type>
DisjointSetItem<Type>::DisjointSetItem(DisjointSetItem<Type>&& other)
    : value(other.value), rep(other.rep), list(other.list)
{
    // If the other points to itself, it is the representative for its set, thus
    // point it to its new self. If it isn't, then keep pointing to the real
    // representative and in theory the list should be empty.
    if (other.rep == &other)
    {
        rep = this;

        // Update all items in the list to point to the new self
        for (DisjointSetItem<Type>* item : list)
        {
            item->rep = this;
        }
    }
    // Otherwise, update the pointer in this one's parent
    else
    {
        list_iter iter = rep->list.find(&other);

        if (iter != rep->list.end())
        {
            // Can't change item in set, so just re-add it
            rep->list.erase(iter);
            rep->list.insert(this);
        }
        else
        {
            throw DisjointItemNotInParent();
        }
    }
}

template<class Type>
DisjointSet<Type>::DisjointSet(DisjointSet<Type>&& other)
    : sets(std::move(other.sets)),
      defaultElem(std::move(other.defaultElem))
{
}

template<class Type>
DisjointSet<Type>& DisjointSet<Type>::operator=(DisjointSet<Type>&& other)
{
    sets = std::move(other.sets);
    defaultElem = std::move(other.defaultElem);

    return *this;
}

template<class Type>
void DisjointSet<Type>::add(Type elem)
{
    if (elem == defaultElem)
        throw ElementIsDefault();

    // Insert if it doesn't already exist (existence based solely on the value
    // of the item, not the representative or list)
    sets.emplace(DisjointSetItem<Type>(elem));
}

template<class Type>
void DisjointSet<Type>::join(Type elem1, Type elem2)
{
    set_iter rep1 = sets.find(DisjointSetItem<Type>(elem1));
    set_iter rep2 = sets.find(DisjointSetItem<Type>(elem2));

    // If one of them doesn't exist, then we don't have to join anything
    if (rep1 == sets.end() || rep2 == sets.end())
        return;

    // If they both are part of the same list
    if (rep1->rep == rep2->rep)
        return;

    // Determine which item is a part of a smaller list so we do less work
    DisjointSetItem<Type>* small;
    DisjointSetItem<Type>* large;

    if (rep1->rep->list.size() > rep2->rep->list.size())
    {
        large = rep1->rep;
        small = rep2->rep;
    }
    else
    {
        large = rep2->rep;
        small = rep1->rep;
    }

    // Change all of the small item's representatives to that of the larger
    // one and add the smaller items to the larger representative's list
    for (DisjointSetItem<Type>* item : small->list)
    {
        item->rep = large;
        large->list.insert(item);
    }

    // Finally, point the smaller representative to the representative of the
    // larger set, add this to the larger one's list, and clear the smaller
    // one's list
    large->list.insert(small);
    small->list.clear();
    small->rep = large;
}

template<class Type>
Type DisjointSet<Type>::find(Type elem) const
{
    // Search for this item by creating a new item. The representative doesn't
    // matter since equality is checked solely based on the element itself.
    DisjointSetItem<Type> item(elem);
    set_const_iter iter = sets.find(DisjointSetItem<Type>(elem));

    // Return representative element
    if (iter != sets.end())
        return iter->rep->value;

    return defaultElem;
}

template<class Type>
void DisjointSet<Type>::selfcheck() const
{
    for (const DisjointSetItem<Type>& item : sets)
    {
        if (item.rep == NULL)
        {
            throw DisjointSetParentNull();
        }
        else if (item.rep == &item)
        {
            for (const DisjointSetItem<Type>* ptr : item.list)
            {
                // This item points to the proper representative
                if (ptr->rep != &item)
                    throw DisjointItemWrongParent();

                // This item's list is empty
                if (!ptr->list.empty())
                    throw DisjointItemNotEmpty();
            }
        }
        else
        {
            // This item's list is empty
            if (!item.list.empty())
                throw DisjointItemNotEmpty();

            // This item is in its representative's list. We must cast this
            // since the list is of non-const pointers. However, we won't be
            // modifying anything here so we're still treating it as const.
            list_const_iter iter = item.rep->list.find(
                    const_cast<DisjointSetItem<Type>*>(&item));

            if (iter == item.rep->list.end())
                throw DisjointItemNotInParent();
        }

        // See if we can find this item
        Type found = find(item.value);

        if (found == defaultElem || found != item.rep->value)
            throw DisjointSetItemNotFound();
    }
}

template<class Type>
std::ostream& operator<<(std::ostream& os, const DisjointSet<Type>& set)
{
    // Print out all the representative items
    for (const DisjointSetItem<Type>& item : set.sets)
        if (item.rep == &item)
            os << item.value << " (" << item.rep->value << ") #"
               << item.rep->list.size() << ": " << item << std::endl;

    return os;
}

template<class Type>
std::ostream& operator<<(std::ostream& os, const DisjointSetItem<Type>& elem)
{
    for (DisjointSetItem<Type>* item : elem.list)
        os << item->value << " (" << item->rep->value << "), ";

    return os;
}

#endif
