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


#include <daw/csv_helper/data_cell.h>
#include <daw/csv_helper/data_table.h>
#include <daw/daw_algorithm.h>

#include "pump_data_analysis.h"

namespace daw {
	namespace pumpdataanalysis {

		namespace {
			size_t skip_hrs( size_t row, daw::data::DataTable::value_type const & ts_col, int32_t const hours ) {
				auto const time_start = ts_col[row++].timestamp( );
				for( ; row < ts_col.size( ); ++row ) {
					auto const time_now = ts_col[row].timestamp( );
					auto const duration = time_now - time_start;
					if( duration.hours( ) >= hours ) {
						break;
					}
				}
				return row;
			}

			template<typename T>
			T for_forward( const T& start, const T& end_exclusive, ::std::function<bool( size_t )> action ) {
				for( T n = start; n < end_exclusive; ++n ) {
					if( action( n ) ) {
						return n;
					}
				}
				return end_exclusive;
			}

			size_t row_from_date( boost::posix_time::ptime dte, const daw::data::DataTable::value_type& column_timestamp, size_t start_row = 0, size_t end_row = 0 ) {
				if( 0 == end_row ) {
					end_row = column_timestamp.size( );
				}
				auto action = [&dte, &column_timestamp]( size_t n ) -> bool {
					auto const& cur_cell = column_timestamp[n];
					if( cur_cell && cur_cell.timestamp( ) >= dte ) {
						return true;
					}
					return false;
				};
				return for_forward( start_row, end_row, action );
			}
			
			#if 0
			std::pair<size_t, size_t> rows_from_date_range( ::std::pair<boost::posix_time::ptime, boost::posix_time::ptime> date_range, const daw::data::DataTable::value_type& column_timestamp ) {
				size_t first = row_from_date( date_range.first, column_timestamp );
				size_t last = row_from_date( date_range.second, column_timestamp, first + 1 );
				if( column_timestamp.size( ) == last ) {
					--last;
				}
				if( first == last || column_timestamp.size( ) == first ) {
					throw ::std::runtime_error( ": Error getting converting timestamp to row." );
				}
				return{ first, last };
			}
			#endif

			bool should_stop_basal_test( const daw::data::DataTable& table, const size_t row ) {
				// Columns of impact
				auto const& col_bolus = table["Bolus Volume Delivered (U)"];
				auto const& col_carb = table["BWZ Carb Input (grams)"];
				auto const& col_raw_type = table["Raw-Type"];

				const bool has_manual_food = 0 == col_raw_type[row].string( ).compare( "JournalEntryMealMarker" );	// Has eaten
				const bool has_temp_basal = 0 == col_raw_type[row].string( ).compare( "ChangeTempBasalPercent" );	// Basal dose isn't normal
				const bool has_bolus_wizard_carb = !col_carb[row].empty( );	// Has eaten
				const bool has_bolus_dose = !col_bolus[row].empty( );	// Has taken bolus insulin
				return has_manual_food || has_temp_basal || has_bolus_wizard_carb || has_bolus_dose;
			}

			PumpDataAnalysis::basal_tests_t do_basal_test( daw::data::DataTable const & data_table ) {				
				using daw::algorithm::rbegin2;
				using ::std::begin;

				auto const& ts_col = data_table["Timestamp"];
				auto const& sensor_col = data_table["Sensor Glucose (mmol/L)"];
				const ::std::pair<size_t, size_t> minmax_rows = { 0, ts_col.size( ) - 1 };

				std::vector<std::pair<daw::data::timestamp_t, size_t>> current_values;
				PumpDataAnalysis::basal_tests_t basal_tests;
				size_t start_row = minmax_rows.first;
				start_row = skip_hrs( start_row, ts_col, 4 );	// We don't know if there is insulin/food just before start
				// TODO: backtrack if duration is changed and see if we can go back 4hrs without food/insulin
				// or start of file
				for( size_t row = start_row; row <= minmax_rows.second; ++row ) {
					if( sensor_col[row] ) {
						current_values.push_back( { ts_col[row].timestamp( ), row } );
					}

					if( should_stop_basal_test( data_table, row ) ) {
						if( 2 <= current_values.size( ) && rbegin2( current_values )->first != begin( current_values )->first ) {
							const boost::posix_time::time_duration duration = rbegin2( current_values )->first - begin( current_values )->first;
							if( duration.total_seconds( ) > 1800 ) {	// For now, keep a minimum duration of 1/2hr.  May not be needed TODO: test change without
								auto const minmax_vals = [&]( ) {
									std::pair<daw::data::real_t, daw::data::real_t> ret{ ::std::numeric_limits<daw::data::real_t>::max( ), ::std::numeric_limits<daw::data::real_t>::min( ) };
									for( auto const& pos : current_values ) {
										auto const&
											cell = sensor_col[pos.second];
										if( cell ) {
											auto const& val = cell.real( );
											if( ret.first > val ) {
												ret.first = val;
											}
											if( ret.second < val ) {
												ret.second = val;
											}
										}
									}
									return ret;
								}();
								auto const rise = (minmax_vals.second - minmax_vals.first) / (static_cast<daw::data::real_t>(duration.hours( )) + static_cast<daw::data::real_t>(duration.minutes( )) / 60.0);
								if( abs( rise ) <= 1.0 && (rbegin2( current_values )->first - begin( current_values )->first).hours( ) < 24 ) {	// For now, don't allow more than a 1mmol/L per hr rise or drop
									basal_tests.push_back( { begin( current_values )->second, rbegin2( current_values )->second } );
								}
							}
						}
						current_values.clear( );
						row = skip_hrs( row, ts_col, 4 );
					}
				}
				return basal_tests;
			}

		}	// namespace anonymous~

		daw::data::DataTable PumpDataAnalysis::parse_csv( daw::data::parse_csv_data_param const & param, ::std::function<void( )> on_completed ) {
			auto && tbl = daw::data::parse_csv_data( ::std::move( param ) );
			if( !tbl.has_value( ) ) {
				::std::string msg = ": Error opening table\n";
				msg += tbl.get_exception_message( );
				throw ::std::runtime_error( msg );
			} else {
				daw::data::DataTable result = ::std::move( tbl.get( ) );
				// Cleanup
				{	// TODO: Move out to a callback
					daw::data::algorithm::erase_rows( result, []( auto const & row, auto const & tbl ) {
						for( size_t n = 1; n < tbl.size( ); ++n ) {
							if( tbl[n][row] ) {
								return false;	// Row has data, don't erase
							}
						}
						return true;	// Row is empty of our data
					} );
					// Convert Timestamp column from string to timestamp
					static auto const dteformat = "%d/%m/%y %H:%M:%S";
					for( auto & cell : result["Timestamp"] ) {
						auto cell_value = cell.string( );
						cell = daw::data::DataCell::from_time_string( cell_value, dteformat );
					}
				}
				on_completed( );
				m_basal_tests_fut = std::async( [res = std::cref( result )]( ) {
					return do_basal_test( res );
				} ).share( );
				return result;
			}
		}

		PumpDataAnalysis::PumpDataAnalysis( daw::data::parse_csv_data_param const & param, ::std::function<void( )> on_completed ):
				m_data_table_fut{ std::async( [&, param, on_completed]( ) {

			auto result = parse_csv( param, on_completed );
			return result;
		} ).share( ) } { }

		daw::data::DataTable const & PumpDataAnalysis::data_table( ) const {
			return m_data_table_fut.get( );
		}

		PumpDataAnalysis::basal_tests_t const & PumpDataAnalysis::basal_tests( ) const {
			return m_basal_tests_fut.get( );
		}

		PumpDataAnalysis::PumpDataAnalysis( PumpDataAnalysis&& other ) noexcept:
				m_data_table_fut{ ::std::move( other.m_data_table_fut ) },
				m_basal_tests_fut{ ::std::move( other.m_basal_tests_fut ) } { }
	
		PumpDataAnalysis & PumpDataAnalysis::operator=( PumpDataAnalysis && rhs) noexcept {
			if( this != &rhs ) {
				PumpDataAnalysis tmp{ ::std::move( rhs ) };
				using std::swap;
				swap( *this, tmp );
			}
			return *this;
		}

		void swap( PumpDataAnalysis & lhs, PumpDataAnalysis & rhs ) noexcept {
			using ::std::swap;
			swap( lhs.m_data_table_fut, rhs.m_data_table_fut );
			swap( lhs.m_basal_tests_fut, rhs.m_basal_tests_fut );
		}

		PumpDataAnalysis::basal_tests_t PumpDataAnalysis::basal_tests_in_range( ::std::pair<boost::posix_time::ptime, boost::posix_time::ptime> date_range ) {
			auto const& ts_col = data_table( )["Timestamp"];
			const size_t min_row = row_from_date( date_range.first, ts_col );
			const size_t max_row = row_from_date( date_range.second, ts_col, min_row + 1 );

			auto const first = [&]( ) {
				for( size_t test_no = 0; test_no <= basal_tests( ).size( ); ++test_no ) {
					if( basal_tests( )[test_no].first >= min_row ) {
						return test_no;
					}
				}
				throw daw::exception::AssertException( ": The date range specified was not valid and cannot be found in basal data" );
			}();

			auto const second = [&]( ) {
				for( size_t test_no = basal_tests( ).size( ) - 1; test_no > first; --test_no ) {
					if( basal_tests( )[test_no].second <= max_row ) {
						return test_no;
					}
				}
				throw daw::exception::AssertException( ": The date range specified was not valid and cannot be found in basal data" );
			}();

			return basal_tests_t{ begin( basal_tests( ) ) + first, begin( basal_tests( ) ) + second };
		}

	}	// namespace pumpdataanalysis
}	// namespace daw
