/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CopyPopUntil.h
 *  @brief: Templated function to copy/pop from the front of a container until
 *          a predicate is true.
 */

#ifndef COPYPOPUNTIL_H
#define COPYPOPUNTIL_H
#include <algorithm>



/**
 * CopyPopUntil
 *    pushes back the front of a container to the back of another container
 *    popping until a predicate is true.
 *
 * @param c1 - container 1 must support front(), empty() and pop_front().
 * @param c2 - container 2 must support push_back and contain the same type
 *             of item as c1.
 * @param pred - Predicate called with a reference to the item contained
 *             returns true when done copying.
 */
template <class c1type, class c2type, class UnaryPredicate>
void
CopyPopUntil(c1type& c1, c2type& c2, UnaryPredicate& pred)
{
    
    auto s = c1.begin();
    auto e = s;
    while (e != c1.end()) {
        if (pred(*e)) break;
        e++;
    }
    // The range [s, e) must be moved...unless e is end
    
    c2.insert(c2.end(), s, e);
    c1.erase(s, e);
}

/**
 * CopyPopUntilR
 *    Same as CopyPopUntil but works from the back end of  the source
 *    until the predicate is not satisfied.
 *    parameters are all the same.
 *
 */
template <class c1type, class c2type, class UnaryPredicate>
void
CopyPopUntilR(c1type& c1, c2type& c2, UnaryPredicate& pred)
{
    
    auto s = c1.begin();
    auto re = c1.rbegin();
    while (re != c1.rend()) {
        if (!pred(*re)) break;
        re++;
    }
    
    auto e = re.base();
    
    // The range [s, e) must be moved...unless e is end
    
    c2.insert(c2.end(), s, e);
    c1.erase(s, e);
}
#endif