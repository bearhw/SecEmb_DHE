/*
*    ZeroTrace: Oblivious Memory Primitives from Intel SGX 
*    Copyright (C) 2018  Sajin (sshsshy)
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, version 3 of the License.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * RandomRequestSource.hpp
 *
 *  Created on: Mar 26, 2017
 *      Author: root
 */

#ifndef APP_RANDOMREQUESTSOURCE_HPP_
#define APP_RANDOMREQUESTSOURCE_HPP_

class RandomRequestSource
{
public:
	RandomRequestSource(){};
	RandomRequestSource(int length) {};
	int* GenerateRandomSequence(int length, int max_capacity);
	int* GenerateSequential(int length, int max_capacity);
};

#endif
