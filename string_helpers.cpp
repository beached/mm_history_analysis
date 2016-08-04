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

#include <memory>

#include <daw/daw_string.h>

#include "string_helpers.h"

namespace daw {
	namespace string {
		std::string ptime_to_string( const boost::posix_time::ptime& value, const std::string& format, const std::string locale_str ) {
			try {
				static std::stringstream ss( "" );
				static std::unique_ptr<boost::posix_time::time_facet> facet;
				if( !facet ) {
					facet = std::make_unique<boost::posix_time::time_facet>( 1 );
					ss.imbue( std::locale( std::locale( locale_str ), facet.get( ) ) );
				}
				clear( ss );
				facet->format( format.c_str( ) );
				ss << value;
				return ss.str( );
			} catch( const std::exception& ex ) {
				throw ex;
			}
		}

		std::string ptime_to_string( const boost::posix_time::time_duration& value, bool show_seconds ) {
			try {
				static std::stringstream ss( "" );
				clear( ss );
				ss << std::setw( 2 ) << std::setfill( '0' ) << value.hours( );
				ss << ":";
				ss << std::setw( 2 ) << std::setfill( '0' ) << value.minutes( );
				if( show_seconds ) {
					ss << ":";
					ss << std::setw( 2 ) << std::setfill( '0' ) << value.seconds( );
				}
				return ss.str( );
			} catch( const std::exception& ex ) {
				throw ex;
			}
		}
	}	// namespace string
}	// namespace daw
