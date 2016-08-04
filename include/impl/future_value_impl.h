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

#include <future>
#include <chrono>
#include <memory>

#include <daw/scope_guard.h>

#include "multi_lock.h"

namespace daw {
	template<typename T> class FutureValue;

	namespace impl {
		template<typename T>
		class FutureValueImpl {
			mutable std::mutex m_value_mutex;
			mutable ::std::future<T> m_value_fut;
			mutable std::unique_ptr<T> m_value;
			mutable bool m_has_retrieved;

		public:
			FutureValueImpl( ): 
					m_value_mutex{ },
					m_value_fut{ }, 
					m_value{ }, 
					m_has_retrieved{ true } { }


			FutureValueImpl( FutureValueImpl && other ): 
					m_value_mutex{ },
					m_value_fut{ ::std::move( other.m_value_fut ) }, 
					m_value{ ::std::exchange( other.m_value, nullptr ) }, 
					m_has_retrieved{ std::exchange( other.m_has_retrieved, true ) } { }


			FutureValueImpl( FutureValueImpl const & ) = delete;
			FutureValueImpl & operator=( FutureValueImpl const & ) = delete;

			~FutureValueImpl( ) = default;

			FutureValueImpl & operator=( FutureValueImpl && rhs ) {
				if( this != &rhs ) {
					FutureValueImpl tmp{ std::move( rhs ) };
					using std::swap;
					swap( *this, tmp );
				}
				return *this;
			}

			friend void swap( FutureValueImpl & lhs, FutureValueImpl & rhs ) {
				daw::multi_lock_guard_t{ &lhs.m_value_mutex, &rhs.m_value_mutex };
				using std::swap;
				swap( lhs.m_has_retrieved, rhs.m_has_retrieved );
				swap( lhs.m_value, rhs.m_value );
				swap( lhs.m_value_fut, rhs.m_value_fut );
			}


			template<typename F>
			FutureValueImpl( F && func, std::launch launch_policy ):
					m_value_mutex{ },
					m_value_fut{ std::async( launch_policy, std::forward<F>( func ) ) },
					m_value{ }, 
					m_has_retrieved{ false } { }



			template<typename F>
			void reset( F && func, std::launch launch_policy ) {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				m_has_retrieved = false;
				m_value_fut = std::future<T>( std::async( launch_policy, std::forward<F>( func ) ) );
			}


			FutureValueImpl & operator=( ::std::future<T> && rhs) {
				FutureValueImpl tmp{ std::move( rhs ) };
				using std::swap;
				swap( *this, tmp );
				return *this;
			}

			T & get( ) {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				retrieve( );
				return *m_value;
			}

			T const & get( ) const {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				retrieve( );
				return *m_value;
			}

			bool valid( ) const {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				if( m_has_retrieved ) {
					return true;
				}
				return m_value_fut.valid( );
			}

			void wait( ) const {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				if( !m_has_retrieved ) {
					if( !m_value_fut.valid( ) ) {
						throw std::runtime_error( "Error retrieving future value" );
					}
					m_value_fut.wait( );
				}
			}

			template<class Rep, class Period>
			std::future_status wait_for( const std::chrono::duration<Rep, Period>& timeout_duration ) const {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				if( !m_has_retrieved ) {
					if( !m_value_fut.valid( ) ) {
						throw std::runtime_error( "Error retrieving future value" );
					}
					return m_value_fut.wait_for( timeout_duration );
				}
				return std::future_status::ready;
			}

			template<class Clock, class Duration>
			std::future_status wait_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) const {
				std::lock_guard<std::mutex> lock( m_value_mutex );
				if( !m_has_retrieved ) {
					if( !m_value_fut.valid( ) ) {
						throw std::runtime_error( "Error retrieving future value" );
					}
					return m_value_fut.wait_until( timeout_time );
				}
				return std::future_status::ready;
			}
		private:
			void retrieve( ) const {
				if( !m_has_retrieved ) {
					SCOPE_EXIT{
						m_has_retrieved = true;
					};
					m_value = std::make_unique<T>( std::move( m_value_fut.get( ) ) );
				}
			}
		};
	}
}

