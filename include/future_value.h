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

#include <chrono>
#include <memory>

#include "impl/future_value_impl.h"

namespace daw {
	template<typename T>
	class FutureValue {
	private:
		std::unique_ptr<daw::impl::FutureValueImpl<T>> m_impl;
	public:
		static const std::launch default_launch_policy = std::launch::async;

		FutureValue( ):
				m_impl{ std::make_unique<daw::impl::FutureValueImpl<T>>( ) } { }

		~FutureValue( ) = default;

		FutureValue( FutureValue const & ) = delete;
		FutureValue& operator=( FutureValue const & ) = delete;

		FutureValue( FutureValue && rhs ): 
				m_impl( std::move( rhs.m_impl ) ) { }

		FutureValue( ::std::future<T> && other ):
				m_impl( std::move( other ) ) { }

		FutureValue & operator=( FutureValue && rhs ) {
			if( this != &rhs ) {
				m_impl = std::move( rhs.m_impl );
			}
			return *this;
		}

		template<typename F>
		FutureValue( F && func, std::launch launch_policy = default_launch_policy ): 
			m_impl{ std::make_unique<daw::impl::FutureValueImpl<T>>( std::forward<F>( func ), launch_policy ) } { }

		template<typename F>
		void reset( F && func, std::launch launch_policy = default_launch_policy ) {
			m_impl->reset( std::forward<F>( func ), launch_policy );
		}

		T & get( ) {
			return m_impl->get( );
		}

		T const & get( ) const {
			return m_impl->get( );
		}

		bool valid( ) const {
			return m_impl->valid( );
		}

		void wait( ) const {
			return m_impl->wait( );
		}

		template<class Rep, class Period>
		std::future_status wait_for( const std::chrono::duration<Rep, Period>& timeout_duration ) const {
			return m_impl->wait_for( timeout_duration );
		}

		template<class Clock, class Duration>
		std::future_status wait_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) const {
			return m_impl->wait_until( timeout_time );
		}
	};
}

