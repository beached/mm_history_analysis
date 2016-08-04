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

#include <limits>
#include <vector>

#include <daw/daw_exception.h>
#include <daw/daw_math.h>

namespace daw {
	template<typename T>
	class AggregateData {
	private:
		std::vector<T> values;
	public:
		T average;
		T high;
		T low;
		T std_dev;
		size_t count;
	
		AggregateData( ): 
				values{ }, 
				average{ 0.0 }, 
				high{ ::std::numeric_limits<T>::min( ) }, 
				low{ ::std::numeric_limits<T>::max( ) }, 
				std_dev{ 0 }, 
				count{ 0 } { } 


		void add_value( const T& value ) {
			average += value;
			++count;
			if( value < low ) {
				low = value;
			}
			if( value > high ) {
				high = value;
			}
			values.push_back( value );
		}

		void process_values( ) {
			if( 0 != values.size( ) ) {	// We can assume all values are all greater than zero
				average /= static_cast<T>(count);
			}
			for( auto const & value : values ) {
				std_dev += daw::math::sqr( average - value );
			}
			values.clear( );
			values.reserve( 0 );
			std_dev /= static_cast<T>(count);
			std_dev = sqrt( std_dev );
		}
	};


	template<typename T>
	class AggregateDataVector {
	private:
		std::vector<AggregateData<T>> m_aggregate_data_vector;
	public:
		AggregateDataVector( ): m_aggregate_data_vector( ) { }

		std::vector<AggregateData<T>>& get( ) {
			return m_aggregate_data_vector;
		}

		const ::std::vector<AggregateData<T>>& get( ) const {
			return m_aggregate_data_vector;
		}
	};

}

