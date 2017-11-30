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

#ifndef HASHCOL_HASH_FUNCTION_H
#define HASHCOL_HASH_FUNCTION_H


HASHCOL_BEGIN_NAMESPACE

template <class key_t> 
struct hash{};

template <> 
struct hash<short>
{
  std::size_t operator()(short x){return 16161 * x;}
};

template <> 
struct hash<unsigned short>
{
  std::size_t operator()(unsigned short x){return 16161 * x;}
};

template <> 
struct hash<int>
{
  std::size_t operator()(int x){return 16161 * x;}
};

template <> 
struct hash<unsigned int>
{
  std::size_t operator()(unsigned int x){return 16161 * x;}
};

template <> 
struct hash<long>
{
  std::size_t operator()(long x){return 16161 * x;}
};

template <> 
struct hash<unsigned long>
{
  std::size_t operator()(unsigned long x){return 16161 * x;}
};

HASHCOL_END_NAMESPACE

#endif //HASHCOL_HASH_FUNCTION_H
