//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

#ifndef SIZE_LRU_CACHE_HPP
#define SIZE_LRU_CACHE_HPP

#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <utility>

#include <boost/optional.hpp>

namespace size_lru_cache {
// a cache which evicts the least recently used item when it is full
template <class Key> class lru_cache {
public:
  typedef Key key_type;
  typedef size_t size_type;
  typedef std::list<key_type> list_type;
  typedef std::map<key_type, std::pair<size_type, typename list_type::iterator>>
      map_type;

  lru_cache(size_t capacity) : m_capacity(capacity) {}

  ~lru_cache() {}

  size_t size() const { return m_size_; }

  size_t capacity() const { return m_capacity; }

  bool empty() const { return m_map.empty(); }

  bool contains(const key_type &key) { return m_map.find(key) != m_map.end(); }

  void insert(const key_type &key, const size_type &size) {
    typename map_type::iterator i = m_map.find(key);
    if (i == m_map.end()) {
      // insert item into the cache, but first check if it is full
      m_size_ += size;
      while (m_size_ > m_capacity) {
        // cache is full, evict the least recently used item
        evict();
      }

      // insert the new item
      m_list.push_front(key);
      m_map[key] = std::make_pair(size, m_list.begin());
    }
  }

  boost::optional<size_type> get(const key_type &key) {
    // lookup value in the cache
    typename map_type::iterator i = m_map.find(key);
    if (i == m_map.end()) {
      // value not in cache
      return boost::none;
    }

    // return the value, but first update its place in the most
    // recently used list
    typename list_type::iterator j = i->second.second;
    if (j != m_list.begin()) {
      // move item to the front of the most recently used list
      m_list.erase(j);
      m_list.push_front(key);

      // update iterator in map
      j = m_list.begin();
      const size_type &value = i->second.first;
      m_map[key] = std::make_pair(value, j);

      // return the value
      return value;
    } else {
      // the item is already at the front of the most recently
      // used list so just return it
      return i->second.first;
    }
  }

  void clear() {
    m_map.clear();
    m_list.clear();
  }

private:
  void evict() {
    // evict item from the end of most recently used list
    typename list_type::iterator i = --m_list.end();
    // std::cout << "evicting " << *i << std::endl;

    auto size = m_map.find(*i)->second.first;
    m_size_ -= size;

    m_map.erase(*i);
    m_list.erase(i);
  }

private:
  map_type m_map;
  list_type m_list;
  size_t m_capacity;
  size_t m_size_ = 0;
};
} // namespace size_lru_cache

#endif // SIZE_LRU_CACHE_HPP
