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

#ifndef HASHCOL_HASH_MAP_H
#define HASHCOL_HASH_MAP_H


#include "hash_table.h"
#include "increment.h"


HASHCOL_BEGIN_NAMESPACE


template <
  class key_t, 
  class value_t, 
  class hash_fcn_t = hash<key_t>, 
  class increment_t = unit_increment<key_t>,
  class equal_key_t = std::equal_to<key_t>, 
  class alloc_t = std::allocator<std::pair<key_t, value_t> > >
class hash_map 
{
private:
  typedef hash_map<key_t, value_t, hash_fcn_t, increment_t, equal_key_t, alloc_t> Self;

  //typedef std::pair<const key_t, value_t> Map_pair; 
  typedef std::pair<key_t, value_t> Map_pair;
  typedef hash_table__<
    key_t,
    Map_pair,
    hash_fcn_t,
    increment_t,
    equal_key_t,
    select1st<Map_pair>,
    alloc_t> HT; 

  HT underlying_;

public:
  typedef typename HT::key_type key_type;
  typedef value_t data_type;
  typedef typename HT::value_type value_type;
  typedef typename HT::size_type size_type;
  typedef typename HT::hasher hasher;
  typedef typename HT::key_equal key_equal;
  typedef typename HT::pointer pointer;
  typedef typename HT::reference reference;
  typedef typename HT::const_reference const_reference;
  typedef typename HT::allocator allocator;
  typedef typename HT::iterator iterator;
  typedef typename HT::const_iterator const_iterator;
  typedef typename HT::difference_type difference_type;
  
  hash_map(size_type max = 100):
    underlying_(max){}
  hash_map(size_type max, const hasher& h):
    underlying_(max, h){}
  hash_map(size_type max, const hasher& h, const key_equal& eq):
    underlying_(max, h, eq){}

  template <class input_iterator_t>
  hash_map(input_iterator_t b, input_iterator_t e, size_type max = 100):
    underlying_(max)
  {
    this->underlying_.insert_unique(b, e);
  }
  template <class input_iterator_t>
  hash_map(input_iterator_t b, input_iterator_t e, size_type max, const hasher& h):
    underlying_(max, h)
  {
    this->underlying_.insert_unique(b, e);
  }
  template <class input_iterator_t>
  hash_map(input_iterator_t b, input_iterator_t e, size_type max, const hasher& h, const key_equal& eq):
    underlying_(max, h, eq)
  {
    this->underlying_.insert_unique(b, e);
  }

  //Getters.
  hasher hash_funct()const{return this->underlying_.hash_funct();}
  key_equal key_eq()const{return this->underlying_.key_eq();}

  void swap(Self& other){this->underlying_.swap(other.underlying_);}

  std::pair<iterator, bool> insert(const value_type& x)
  {
    return this->underlying_.insert_unique(x);
  }  
  template <class iterator_t>
  void insert(iterator_t b, iterator_t e)
  {
    this->underlying_.insert_unique(b, e);
  }

  void erase(iterator it){this->underlying_.erase(it);}  
  void erase(iterator b, iterator e){this->underlying_.erase(b, e);}
  size_type erase(const key_type& k){return this->underlying_.erase(k);}

  iterator find(const key_type& k){return this->underlying_.find(k);}
  const_iterator find(const key_type& k)const{return this->underlying_.find(k);}

  size_type size()const{return this->underlying_.size();}
  size_type max_size()const{return this->underlying_.max_size();}
  size_type bucket_count()const{return this->underlying_.bucket_count();}
  bool empty()const{return this->underlying_.empty();}
  void resize(size_type n){this->underlying_.resize_unique(n);}
  void clear(){this->underlying_.clear();}
  size_type count(const key_type& k)const{return this->underlying_.count(k);}

  data_type& operator[](const key_type& k)
  {
    iterator it = this->underlying_.find(k);
    if (it == this->underlying_.end())
      return (*((this->insert(value_type(k, data_type()))).first)).second;
    return it->second;
  }

  iterator begin(){return this->underlying_.begin();}
  iterator end(){return this->underlying_.end();}
  const_iterator begin()const{return this->underlying_.begin();}
  const_iterator end()const{return this->underlying_.end();}

  template <class K, class V, class H, class I, class E, class A>
  friend bool
  operator==(const hash_map<K, V, H, I, E, A>& l, const hash_map<K, V, H, I, E, A>& r);

};

template <class K, class V, class H, class I, class E, class A>
bool
operator==(const hash_map<K, V, H, I, E, A>& l, const hash_map<K, V, H, I, E, A>& r)
{
  return l.underlying_ == r.underlying_;
}

HASHCOL_END_NAMESPACE

#endif //HASHCOL_HASH_MAP_H
