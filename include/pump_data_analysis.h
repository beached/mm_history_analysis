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

#include <boost/date_time/posix_time/ptime.hpp>
#include <memory>
#include <string>

#include <daw/csv_helper/data_table.h>

#include "future_value.h"

namespace daw {
// 	namespace data {
// 		class DataTable;
// 	}

	namespace pumpdataanalysis {
		struct PumpDataAnalysis final {
			using basal_tests_t = std::vector<::std::pair<size_t, size_t>>;
		private:
			daw::data::DataTable parse_csv( daw::data::parse_csv_data_param const & param, ::std::function<void( )> on_completed );

			daw::FutureValue<daw::data::DataTable> m_data_table_fut;
			daw::FutureValue<basal_tests_t> m_basal_tests_fut;
		public:
			PumpDataAnalysis( daw::data::parse_csv_data_param const & param, ::std::function<void( )> on_completed );

			PumpDataAnalysis( ) = delete;
			~PumpDataAnalysis( ) = default;
			PumpDataAnalysis( PumpDataAnalysis const & ) = delete;
			PumpDataAnalysis( PumpDataAnalysis && other ) noexcept;

			PumpDataAnalysis & operator=( PumpDataAnalysis && rhs) noexcept;
			PumpDataAnalysis& operator=( PumpDataAnalysis const & ) = delete;

			friend void swap( PumpDataAnalysis & lhs, PumpDataAnalysis & rhs ) noexcept;

			daw::data::DataTable const & data_table( ) const;
			daw::data::DataTable & rw_data_table( );

			basal_tests_t const & basal_tests( ) const;
			basal_tests_t & basal_tests( );
			basal_tests_t basal_tests_in_range( ::std::pair<boost::posix_time::ptime, boost::posix_time::ptime> date_range );
		};	// PumpDataAnalysis
	}
}

