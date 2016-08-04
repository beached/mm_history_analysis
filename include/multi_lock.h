// The MIT License (MIT)
//
// Copyright (c) 2013-2015 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <mutex>
#include <vector>

#include <daw/daw_traits.h>

namespace daw {
	template<typename MutexType = std::mutex>
	struct multi_lock_guard {
		multi_lock_guard( ) = delete;
		multi_lock_guard( multi_lock_guard const & ) = delete;
		multi_lock_guard& operator=( multi_lock_guard const & ) = delete;
		multi_lock_guard( multi_lock_guard && ) = delete;
		multi_lock_guard& operator=( multi_lock_guard && ) = delete;

		bool operator==( multi_lock_guard const & ) const = delete;

		multi_lock_guard( std::initializer_list<MutexType*> mutexes ):
				m_mutexes{ mutexes } {
			
			std::sort( m_mutexes.begin( ), m_mutexes.end( ), []( MutexType const * const mutex_a, MutexType const * const mutex_b ) {
				return mutex_a < mutex_b;
			} );
			for( auto m : m_mutexes ) {
				m->lock( );
			}
		}

		~multi_lock_guard( ) {
			for( auto m = std::rbegin( m_mutexes ); m != std::rend( m_mutexes ); ++m ) {
				(*m)->unlock( );
			}
		}
	
	private:
		std::vector<MutexType*> m_mutexes;
	};

	using multi_lock_guard_t = multi_lock_guard<>;
}

