/*
* Copyright (c) 2007-2008, Leandro Terra Cunha Melo
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the organization nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Leandro Terra Cunha Melo "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Leandro Terra Cunha Melo BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HASHCOL_OPEN_ADDRESSING_H
#define HASHCOL_OPEN_ADDRESSING_H

#include <utility>
#include <functional>
#include <vector>
#include <memory>
#include <cstddef>
#include <algorithm>

#include "config.h"
#include "hash_function.h"
#include "identity.h"
#include "constness_traits.h"


HASHCOL_BEGIN_NAMESPACE


/***********************************************************************************
NOTES:
  - This hash table is the underlying implementation of hash_map, hash_multimap,
  hash_set, and hash_multiset, which have interface similar to the corresponding 
  (non-standard) SGI implementations. However, both invariants of concept 
  Associative Container are not met. In a open addressing hash implementation I 
  do not think it is possible to provide contiguos storage - that is why expression
  equal_range is not implemented. Furthermore, I need assignment of pair with
  constant keys for immutability of keys (in the map types), which is not possible. 
  An alternative solution would be to construct a pair like interface class 
  with an assignment operator that meets this requirement. I have not done it yet.

  - It is possible to use two different strategies for colision resolution:
  linear probing and double hashing. This is represented by template argument
  increment_t. I provide both a hash function and a increment function (which
  is the second hash function in the case of double hashing) for integral types.
  One might need to write her own.

  - Elements of the hash table are never erased. Instead, they are just marked as
  unavailable. Actually removing the element is ok for linear probing (one just 
  needs to correct position of elements to the right of the erased element). 
  However, for double hashing there is no obvious equivalent implementation.

  - Functions are defined inside the class definition just for simplicity.

  - I compiled the code under MSVS 2008 (Express) and GCC 3.4.4.

***********************************************************************************/

template <
  class hash_container_t,
  class constness_traits_t>  
struct hash_table_iterator__
{
  typedef hash_table_iterator__<hash_container_t, constness_traits_t> Self;  
  typedef typename hash_container_t::size_type Position;  
  typedef typename hash_container_t::difference_type difference_type;

  typedef typename constness_traits_t::value_type value_type;
  typedef typename constness_traits_t::pointer pointer;
  typedef typename constness_traits_t::reference reference;

  hash_table_iterator__():
    container_(0),current_(-1){}
  hash_table_iterator__(const hash_container_t* t, Position c):
    container_(const_cast<hash_container_t*>(t)),current_(c){}
  hash_table_iterator__(const hash_table_iterator__< //A copy constructor for non-const
                                       hash_container_t,       //and conversion for const.
                                       non_const_traits<value_type> >& non_const):
    container_(non_const.container_),current_(non_const.current_){}

  hash_container_t* container_;  
  Position current_;

  void next_not_null()
  {
    while (true)
    {
      ++this->current_;
      if (this->current_ == this->container_->size() ||
         (!(*this->container_)[this->current_].is_null()) &&
         ((*this->container_)[this->current_].is_available())) break;
    }
  }

  reference operator*()const {return (*this->container_)[this->current_].value_;}
  pointer operator->()const {return &(operator*());}

  Self& operator++()
  {
    this->next_not_null();
    return *this;    
  }
  Self operator++(int)
  {
    Self t = *this;
    ++(*this);
    return t;
  }    
};
  
template <class C, class T>
inline bool 
operator==(const hash_table_iterator__<C, T>& l, 
           const hash_table_iterator__<C, T>& r)
{
  return (l.container_ == r.container_ && l.current_ == r.current_);
}

template <class C>
inline bool 
operator==(const hash_table_iterator__<C, const_traits<typename C::value_type::real_value_type> >& l, 
           const hash_table_iterator__<C, non_const_traits<typename C::value_type::real_value_type> >& r)
{
  return (l.container_ == r.container_ && l.current_ == r.current_);
}

template <class C, class T>
inline bool 
operator!=(const hash_table_iterator__<C, T>& l, 
           const hash_table_iterator__<C, T>& r)
{
  return !(l == r);
}

template <class C>
inline bool 
operator!=(const hash_table_iterator__<C, const_traits<typename C::value_type::real_value_type> >& l, 
           const hash_table_iterator__<C, non_const_traits<typename C::value_type::real_value_type> >& r)
{
  return !(l == r);
}


//Hash container.

template <
  class key_t, 
  class value_t, 
  class hash_fcn_t,
  class increment_t,
  class equal_key_t,
  class get_key_t,
  class alloc_t> 
class hash_table__
{
private:
  typedef hash_table__<
    key_t, 
    value_t, 
    hash_fcn_t, 
    increment_t,
    equal_key_t, 
    get_key_t,
    alloc_t> Self;  

  struct Element
  {
    enum {EMPTY = 0, FULL = 1, NOT_AVAILABLE = 2};
    typedef value_t real_value_type;
    value_t value_;
    int state_;
    bool is_null()const{return this->state_ == EMPTY;}
    bool is_available()const{return this->state_ != NOT_AVAILABLE;}
    void make_unavailable(){this->state_ = NOT_AVAILABLE;}
    Element():state_(EMPTY){}
    Element(const value_t& v):value_(v),state_(FULL){}
  };
  typedef typename alloc_t::template rebind<Element>::other ActualAlloc;
  typedef std::vector<Element, ActualAlloc> Container;

  struct Unique{};
  struct Multi{};

public:
  typedef key_t key_type;
  typedef value_t value_type;
  typedef hash_fcn_t hasher;
  typedef increment_t incrementer;
  typedef equal_key_t key_equal;
  typedef get_key_t get_key;
  typedef alloc_t allocator;

  typedef typename Container::pointer pointer;
  typedef typename Container::reference reference;
  typedef typename Container::const_reference const_reference;
  typedef typename Container::size_type size_type;
  typedef typename Container::difference_type difference_type;
  
  //Iterator types.
  typedef hash_table_iterator__<Container, non_const_traits<value_type> > iterator;
  typedef hash_table_iterator__<Container, const_traits<value_type> > const_iterator;
  

private:
  //State.
  size_type TABLE_SIZE_;
  size_type NUM_ELEMENTS_;
  size_type NUM_VALID_ELEMENTS_;
  Container container_;

  //Interface.
  hasher hash_;
  incrementer increment_;
  key_equal key_equals_;
  get_key get_key_;

  template <class MultiOrUnique>
  void expand(MultiOrUnique);  
  
  size_type find_position(const key_type& k)
  {
    size_type hx = this->hash_(k) % this->TABLE_SIZE_;
    Element* current = &this->container_[hx];
    while (!current->is_null())
    {
      if (current->is_available() &&
          this->key_equals_(k, this->get_key_(current->value_)))
      {
        return hx;
      }
      hx = hx + this->increment_(k);
      current = &this->container_[hx];
    }
    return this->container_.size();
  }

  void reinit(size_type table_size)
  {
    this->NUM_ELEMENTS_ = 0;
    this->NUM_VALID_ELEMENTS_ = 0;
    this->TABLE_SIZE_ = table_size;
    this->container_ = Container(table_size);
  }

  void insert_by_type(const Element& element, Unique)
  {
    this->insert_unique(element.value_);
  }
  void insert_by_type(const Element& element, Multi)
  {
    this->insert_equal(element.value_);
  }
  
public:
  hash_table__(size_type max):
    TABLE_SIZE_(2*max),NUM_ELEMENTS_(0),NUM_VALID_ELEMENTS_(0),container_(2*max){}
  hash_table__(size_type max, const hasher& h):
    TABLE_SIZE_(2*max),NUM_ELEMENTS_(0),NUM_VALID_ELEMENTS_(0),container_(2*max),hash_(h){}
  hash_table__(size_type max, const hasher& h, const key_equal& eq):
    TABLE_SIZE_(2*max),NUM_ELEMENTS_(0),NUM_VALID_ELEMENTS_(0),container_(2*max),hash_(h),key_equals_(eq){}


  //Getters.
  hasher hash_funct()const{return this->hash_;}
  key_equal key_eq()const{return this->key_equals_;}
  
  void swap(Self& other)
  {
    std::swap(this->TABLE_SIZE_, other.TABLE_SIZE_);
    std::swap(this->NUM_ELEMENTS_, other.NUM_ELEMENTS_);
    std::swap(this->NUM_VALID_ELEMENTS_, other.NUM_VALID_ELEMENTS_);
    this->container_.swap(other.container_); //Constant for vector.
    std::swap(this->hash_, other.hash_);
    std::swap(this->increment_, other.increment_);
    std::swap(this->key_equals_, other.key_equals_);
    std::swap(this->get_key_, other.get_key_);
  }
  
  std::pair<iterator, bool> insert_unique(const value_type& x)
  {
    if (this->NUM_ELEMENTS_ > this->TABLE_SIZE_/2) this->expand(Unique());
    key_type xkey = this->get_key_(x);
    size_type hx = this->hash_(xkey) % this->TABLE_SIZE_;
    while (!this->container_[hx].is_null())
    {
      if (this->container_[hx].is_available() &&
          this->key_equals_(xkey, this->get_key_(this->container_[hx].value_)))
      {
        return std::make_pair(iterator(&this->container_, hx), false);
      }
      hx = (hx + this->increment_(xkey)) % this->TABLE_SIZE_; 
    }
    this->container_[hx] = x;
    ++this->NUM_ELEMENTS_;
    ++this->NUM_VALID_ELEMENTS_;
    return std::make_pair(iterator(&this->container_, hx), true);
  }
  iterator insert_equal(const value_type& x)
  {
    if (this->NUM_ELEMENTS_ > this->TABLE_SIZE_/2) this->expand(Multi());
    key_type xkey = this->get_key_(x);
    size_type hx = this->hash_(xkey) % this->TABLE_SIZE_;
    while (!this->container_[hx].is_null())
    {
      hx = (hx + this->increment_(xkey)) % this->TABLE_SIZE_; 
    }
    this->container_[hx] = x;
    ++this->NUM_ELEMENTS_;
    ++this->NUM_VALID_ELEMENTS_;
    return iterator(&this->container_, hx);
  }
  template <class input_iterator_t>
  void insert_unique(input_iterator_t b, input_iterator_t e)
  {
    for (; b != e; ++b) insert_unique(*b);
  }
  template <class input_iterator_t>
  void insert_equal(input_iterator_t b, input_iterator_t e)
  {
    for (; b != e; ++b) insert_equal(*b);
  }

  void erase(iterator it)
  { 
    this->container_[it.current_].make_unavailable(); 
    --this->NUM_VALID_ELEMENTS_; 
  }
  void erase(iterator b, iterator e)
  {
    for (; b != e; ++b) this->erase(b);
  }
  size_type erase(const key_type& k) //Could implement "erase_unique"
    //with a break inside the loop for better performance in unique containers.
  {
    size_type erased = 0;
    size_type hx = this->hash_(k) % this->TABLE_SIZE_;
    Element * current = &this->container_[hx];
    while (!current->is_null())
    {
      if (current->is_available() &&
          this->key_equals_(k, this->get_key_(current->value_)))
      {
        current->make_unavailable();
        --this->NUM_VALID_ELEMENTS_;
        ++erased;
      }
      hx = (hx + this->increment_(k)) % this->TABLE_SIZE_;
      current = &this->container_[hx];
    }
    return erased;
  }

  iterator find(const key_type& k)
  {
    return iterator(&this->container_, this->find_position(k));
  }
  const_iterator find(const key_type& k)const
  {
    return const_iterator(&this->container_, this->find_position(k));
  }

  size_type size()const{return this->NUM_VALID_ELEMENTS_;}
  size_type max_size()const{return this->container_.max_size();}
  size_type bucket_count()const{return this->TABLE_SIZE_;}
  bool empty()const{return 0 == this->NUM_VALID_ELEMENTS_;}
  void resize_unique(size_type n){while (n > this->TABLE_SIZE_) this->expand(Unique());}
  void resize_equal(size_type n){while (n > this->TABLE_SIZE_) this->expand(Multi());}
  void clear(){this->reinit(this->TABLE_SIZE_);}

  size_type count(const key_type& k)const
  { 
    size_type num = 0;
    for (const_iterator it = begin(); it != end(); ++it)
      if (this->key_equals_(this->get_key_(*it), k)) ++num;
    return num;
  }

  iterator begin()
  {
    for (size_type i = 0; i < this->container_.size(); ++i)
      if (!this->container_[i].is_null() && this->container_[i].is_available())
        return iterator(&this->container_, i);
    return end();
  }
  iterator end()
  {
    return iterator(&this->container_, this->container_.size());
  }
  const_iterator begin()const
  {
    for (size_type i = 0; i < this->container_.size(); ++i)
      if (!this->container_[i].is_null() && this->container_[i].is_available())
        return const_iterator(&this->container_, i);
    return end();
  }
  const_iterator end()const
  {
    return const_iterator(&this->container_, this->container_.size());
  }

  template <class K, class V, class H, class I, class E, class G, class A>
  friend bool 
  operator==(const hash_table__<K, V, H, I, E, G, A>& l, 
             const hash_table__<K, V, H, I, E, G, A>& r);

};

template <
  class key_t, 
  class value_t, 
  class hash_fcn_t,
  class increment_t,
  class equal_key_t,
  class get_key_t,
  class alloc_t> 
  template <class MultiOrUnique>
void
hash_table__<
  key_t,
  value_t,
  hash_fcn_t,
  increment_t,
  equal_key_t,
  get_key_t,
  alloc_t>::
expand(MultiOrUnique type)
{
  #ifdef DEBUG
    std::cout << "\nEXPANDINDO TABELA DE HASH...";
  #endif 

  Container copy(this->container_);
  this->reinit(2 * this->TABLE_SIZE_);  
  for (typename Container::iterator it = copy.begin(); it != copy.end(); ++it)
    if (!(it->is_null()) && it->is_available())  insert_by_type(*it, type);
}

//If this is not the semantics you expect, feel free to re-write it.
template <class K, class V, class H, class I, class E, class G, class A>
inline bool 
operator==(const hash_table__<K, V, H, I, E, G, A>& l, 
           const hash_table__<K, V, H, I, E, G, A>& r)
{
  typedef typename hash_table__<K, V, H, I, E, G, A>::size_type size_type;

  if (l.TABLE_SIZE_ == r.TABLE_SIZE_ &&
      l.NUM_ELEMENTS_ == r.NUM_ELEMENTS_ &&
      l.NUM_VALID_ELEMENTS_ == r.NUM_VALID_ELEMENTS_ &&
      l.container_.size() == r.container_.size())
  {
    for (size_type i = 0; i < l.container_.size(); ++i)
      if (!((l.container_[i].is_available() && r.container_[i].is_available() &&
             l.container_[i].value_ == r.container_[i].value_) 
             ||
            (!l.container_[i].is_available() && r.container_[i].is_available())))
      {
        return false;
      }
    return true;
  }
  return false;  
}


HASHCOL_END_NAMESPACE
  
#endif //HASHCOL_OPEN_ADDRESSING_H
