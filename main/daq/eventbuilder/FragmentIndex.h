// FragmentIndex.h
//
// Author : Jeromy Tompkins
// Date   : 3/2014
//

#ifndef FRAGMENTINDEX_H
#define FRAGMENTINDEX_H

#include <vector>
#include <stdint.h>
#include <cstdlib>

/**! A convenient, copiable piece formatting of 
*   a fragment. Similar to a EVB::FlatFragment but
*   it can be copied safely. Similar to a EVB::Fragment
*   but you don't need to typecast the body to use it.
*/
struct FragmentInfo // A structure containting basic info from the Fragment header --JP
{
  uint64_t s_timestamp;
  uint32_t s_sourceId;
  uint32_t s_size;
  uint32_t s_barrier;
  uint16_t* s_itemhdr;
  uint16_t* s_itembody;

  FragmentInfo() : 
      s_timestamp(0), s_sourceId(0), s_size(0), s_barrier(0), 
      s_itemhdr(0), s_itembody(0) {}
};


/**! FragmentIndex
*
* A class that finds the start of every fragment in a built ring item.
*
*/
class FragmentIndex
{
  private:
  typedef std::vector<FragmentInfo> Container;

  public:
  typedef Container::iterator iterator;
  typedef Container::const_iterator const_iterator;

  private:
  Container m_frags; ///< The list of fragment locations

  public:

  /**! Default constructor
   * Does nothing besides initialize the empty list of fragments 
   */
  FragmentIndex();

  /**! Given the body pointer, index all of the fragmentsa
   * @param a pointer to the first word in the body (this is b/4 the first fragment)
   *
   */
  FragmentIndex(uint16_t* data);

  /**! Get a fragment
   * Checks whether the index provided is valid. If index is out of range, 
   * returns a null pointer. Otherwise, it returns the ith pointer.
   *
   * @param i is the index of the fragment in the body (starting at 0)
   * 
   * @return the pointer to the ith fragment or null if ith index doesn't exist
   */  
  FragmentInfo getFragment(size_t i) 
  {
    if (i<m_frags.size() && i>=0) {
      return m_frags.at(i); //pointer to ith fragment in the m_frags list --JP
    } else {
      FragmentInfo null;
      null.s_itembody = static_cast<uint16_t*>(0);
      return null;
    }
  }
 
  size_t getNumberFragments() const { return m_frags.size(); }

  /**! The indexing algorithm 
  * This will traverse the range of data in addresses [begin, end]
  * and find all of the complete fragments. Every fragment found will be appended to the 
  * list of fragments m_frags. This is very similar to the constructor but differs in the arguments.
  * The constructor expects that the first word of the body is passed, this expects that the
  * pointer to the first fragment is passed. There should be a difference of 32-bits between
  * the two arguments.
  *
  * @param begin a pointer to the first fragment
  * @param end pointer just beyond the last fragment 
  */
  void indexFragments(uint16_t* begin, uint16_t* end);

  /**! The indexing algorithm 
  * @param data a pointer to the first fragment
  * @param nbytes the number of bytes from start of first fragment to end of the body
  */
  void indexFragments(uint16_t* data, size_t max_bytes) {
    indexFragments(data, data+max_bytes/sizeof(uint16_t) );
  }

  private:
  size_t computeWordsToNextFragment(uint16_t* data);


  public:
  // Implement an iterator interface
  iterator begin() { return m_frags.begin(); }
  const_iterator begin() const { return m_frags.begin(); }

  iterator end() { return m_frags.end(); }
  const_iterator end() const { return m_frags.end(); }
};

#endif
