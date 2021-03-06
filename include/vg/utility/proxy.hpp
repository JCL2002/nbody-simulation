/*
vg -- random variate generator library 
Copyright (C) 2010-2011  Feng Wang (wanng.fenng@gmail.com) 

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by 
the Free Software Foundation, either version 3 of the License, or 
(at your option) any later version. 

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _PROXY_HPP_INCLUDED_SDOIFJ438OUAFSLSF988923PFUISLFKJEOIJFLKJDOIJFALIJEIJ
#define _PROXY_HPP_INCLUDED_SDOIFJ438OUAFSLSF988923PFUISLFKJEOIJFLKJDOIJFALIJEIJ

#include <cstddef>

namespace vg 
{

//template<typename T>
//struct proxy : T
//{};

template< typename T, std::size_t Level = 0 >
struct proxy;

template< typename T, std::size_t Level >
struct proxy : proxy< T, Level - 1 >
{};

template< typename T >
struct proxy<T, 0> : T
{};


}//namespace vg

#endif//_PROXY_HPP_INCLUDED_SDOIFJ438OUAFSLSF988923PFUISLFKJEOIJFLKJDOIJFALIJEIJ

