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

#include <string>
#include <utility>

#include <daw/daw_algorithm.h>
#include <daw/daw_string.h>

#include "aggregate_data.h"
#include "defs.h"
#include "frame_pump_data_analysis.h"
#include "panel_average_basal_derivative.h"
#include "panel_generic_plot.h"

using namespace daw::data;

namespace {
	void setup_graph( daw::pumpdataanalysis::PanelGenericPlotter& gen_plot, const ::std::vector<daw::AggregateData<daw::data::real_t>>& aggregate_data, daw::pumpdataanalysis::graph_config_t graph_config ) {
		using namespace daw::pumpdataanalysis;
		gen_plot.coord_data( ).margins.set_all( 15 );

		const size_t first_pos( ([&aggregate_data]( ) {
			size_t val = 0;
			while( 0 == aggregate_data[val].count ) {
				++val;
				if( aggregate_data.size( ) == val ) {
					throw ::std::runtime_error( ": Looks like the averages were not done.  There is no data within the array" );
				}
			}
			return val;
		})() );

		point_t last_point_avg{ };
		point_t last_point_low{ };
		point_t last_point_high{ };
		point_t last_point_count{ };

		daw::AggregateData<real_t> const* last_cell;
		int last_cell_time = 0;
		size_t start = first_pos;
		{
			auto const avg_cell_item_time = start * 5;
			auto const& avg_cell = aggregate_data[start];
			last_point_avg = point_t( avg_cell_item_time, static_cast<int>(avg_cell.average*10.0) );
			last_point_low = point_t( avg_cell_item_time, static_cast<int>(avg_cell.low*10.0) );
			last_point_high = point_t( avg_cell_item_time, static_cast<int>(avg_cell.high*10.0) );
			last_point_count = point_t( avg_cell_item_time, static_cast<int>(avg_cell.count*10.0) );
			last_cell = &avg_cell;
			last_cell_time = avg_cell_item_time;
			++start;
		}
		size_t prev_n = start;

		auto gen_poly = [&]( const daw::AggregateData<real_t>& last_agg, int last_time, const daw::AggregateData<real_t>& curr_agg, int curr_time ) {
			auto const std_dev_low_last = static_cast<int>((last_agg.average - last_agg.std_dev)*10.0);
			auto const std_dev_high_last = static_cast<int>((last_agg.average + last_agg.std_dev)*10.0);
			auto const std_dev_low_current = static_cast<int>((curr_agg.average - curr_agg.std_dev)*10.0);
			auto const std_dev_high_current = static_cast<int>((curr_agg.average + curr_agg.std_dev)*10.0);
			return ::std::move( ::std::vector<point_t>{ point_t( last_time, std_dev_low_last ), point_t( last_time, std_dev_high_last ), point_t( curr_time, std_dev_high_current ), point_t( curr_time, std_dev_low_current ), point_t( last_time, std_dev_low_last ) } );
		};

		gen_plot.set_brush( graph_config.brush_area_std_dev );
		for( size_t n = start; n < aggregate_data.size( ); ++n ) {
			auto const& avg_cell = aggregate_data[n];
			if( 0 <= avg_cell.count ) {
				const int avg_cell_item_time = n * 5;
				auto const p2_avg = point_t( avg_cell_item_time, static_cast<int>(avg_cell.average*10.0) );

				auto const p2_low = point_t( avg_cell_item_time, static_cast<int>(avg_cell.low*10.0) );
				auto const p2_high = point_t( avg_cell_item_time, static_cast<int>(avg_cell.high*10.0) );
				auto const p2_count = point_t( avg_cell_item_time, avg_cell.count );

				// Draw graph's in z-order from lowest to highest

				if( avg_cell.count > 0 && aggregate_data[prev_n].count > 0 ) {	// On purpose
					gen_plot.set_pen( graph_config.pen_area_std_dev );
					
					gen_plot.draw_polygon( gen_poly( *last_cell, last_cell_time, avg_cell, avg_cell_item_time ) );

					gen_plot.set_pen( graph_config.pen_line_high );
					gen_plot.draw_line( last_point_high, p2_high );

					gen_plot.set_pen( graph_config.pen_line_low );
					gen_plot.draw_line( last_point_low, p2_low );

					gen_plot.set_pen( graph_config.pen_line_average );
					gen_plot.draw_line( last_point_avg, p2_avg );
				}

				

				last_point_avg = p2_avg;
				last_point_high = p2_high;
				last_point_low = p2_low;
				last_point_count = p2_count;
				last_cell = &avg_cell;
				last_cell_time = avg_cell_item_time;
				prev_n = n;
			}
		}
		
		graph_config.coord_data = gen_plot.coord_data( );
		
		daw::pumpdataanalysis::draw_mmol_y_axis( gen_plot, graph_config, 2.0f );
		daw::pumpdataanalysis::draw_24hr_x_axis( gen_plot, 60, graph_config, 2.0f );
	}
}

PanelAverageBasalDerivative::PanelAverageBasalDerivative( wxWindow *parent, ::std::function<void( wxMenuBar* menu )> addmenu_cb, const daw::data::DataTable& dt, const ::std::vector<std::pair<daw::data::DataTable::size_type, daw::data::DataTable::size_type>> positions ): wxPanel( parent, wxID_ANY ), m_data( dt ), m_basal_positions( positions ), m_bg_min( ::std::numeric_limits<real_t>::max( ) ), m_bg_max( ::std::numeric_limits<real_t>::min( ) ), m_aggregate_vec( new daw::AggregateDataVector<daw::data::real_t>( ) ), m_addmenu_cb( addmenu_cb ) {
	auto& m_aggregate = m_aggregate_vec->get( );
	m_aggregate.resize( m_points_x, daw::AggregateData<daw::data::real_t>( ) );

	// Find max and min
	auto const& bg_col = m_data["Sensor Glucose (mmol/L)"];
	auto const& ts_col = m_data["Timestamp"];

	auto const incs_per_hour = 1;	// must be 12(5min),6(10min),4(15min),3(20min),2(30min),1(60min)
	auto const incs_every_n_min = 60 / incs_per_hour;
	for( auto const position : m_basal_positions ) {
		for( auto row = position.first; row <= position.second; ++row ) {
			if( bg_col[row] ) {
				auto const five_minute_periods_per_day = (60 / 5) * 24;
				const size_t pos = [&]( ) {
					auto const ts_value = ts_col[row].timestamp( ).time_of_day( );
					auto ret = ts_value.hours( ) * 12 + daw::math::round_to_nearest( ts_value.minutes( ), static_cast<float>(incs_every_n_min) ) / 5;
					//auto ret = ts_value.hours( )*12;					
					if( five_minute_periods_per_day <= ret ) {	// Wrap back to midnight 0
						ret = 0;
					}
					return ret;
				}();				
				auto const bg_value = bg_col[row].real( );
				auto const prev_row = row > 0 ? row - 1 : five_minute_periods_per_day-1;
				auto const bg_prev = [prev_row, &bg_col]( ) {
					if( bg_col[prev_row] ) {
						return bg_col[prev_row].real( );
					}
					return static_cast<real_t>( 0 );
				}();

				if( 0 != bg_prev ) {
					for( int n = 0; n < (incs_every_n_min/5); ++n ) {
						m_aggregate[pos+n].add_value( (bg_value - bg_prev)*12.0 );	// mmol/L / hr instead of mmol/L / 5min
					}
				}
			}
		}
	}

	for( size_t n = 0; n < m_aggregate.size( ); ++n ) {
		auto& avg_item = m_aggregate[n];
		if( 0 != avg_item.average ) {
			avg_item.process_values( );

			if( avg_item.average < m_bg_min ) {
				m_bg_min = avg_item.average;
			}
			if( avg_item.average > m_bg_max ) {
				m_bg_max = avg_item.average;
			}
		}
	}
	// create our menu bar: it will be shown instead of the main frame one when
	// we're active
	wxMenuBar *mbar = FramePumpDataAnalysis::create_menu_bar( );
	mbar->GetMenu( 0 )->Insert( 1, wxID_CLOSE, "&Close child\tCtrl-W", "Close this window" );

	// Associate the menu bar with the frame
	addmenu_cb( mbar );

	daw::pumpdataanalysis::graph_config_t graph_config{ };
	graph_config.axis_title_y = "mmol/L per hour";

	setup_graph( m_gen_plot, m_aggregate_vec->get( ), ::std::move( graph_config ) );
}

void PanelAverageBasalDerivative::on_paint( wxPaintEvent& ) {
	wxClientDC dc( this );
	plot( dc, GetClientSize( ) );
}

void PanelAverageBasalDerivative::plot( wxDC& dc, wxSize bounds ) {
	m_gen_plot.plot( dc, ::std::move( bounds ) );
}

BEGIN_EVENT_TABLE( PanelAverageBasalDerivative, wxPanel )
EVT_PAINT( PanelAverageBasalDerivative::on_paint )
END_EVENT_TABLE( )
