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

#include <boost/date_time/posix_time/ptime.hpp>
#include <string>

#include <daw/daw_algorithm.h>
#include <daw/daw_math.h>
#include <daw/daw_string.h>

#include "frame_pump_data_analysis.h"
#include "panel_data_plot.h"

using namespace daw::data;
using namespace daw::math;
using daw::pumpdataanalysis::point_t;
using daw::pumpdataanalysis::PanelGenericPlotter;

namespace {
	auto get_epoch( ) {
		static auto const s_epoch = boost::posix_time::time_from_string( "1970-01-01 00:00:00.000" );
		return s_epoch;
	}

	void setup_graph( daw::pumpdataanalysis::PanelGenericPlotter& gen_plot, const daw::data::DataTable& data, size_t data_first, size_t data_last, daw::pumpdataanalysis::graph_config_t graph_config ) {
		// Setup plot
		gen_plot.coord_data( ).margins.set_all( 15 );

		auto const& bg_col = data["Sensor Glucose (mmol/L)"];
		auto const& ts_col = data["Timestamp"];

		//auto const timespan = ts_cozl[m_data_last].timestamp( ) - ts_col[m_data_first].timestamp( );

		// Draw graph
		auto const start = [&]( ) {
			auto result = data_first;
			while( bg_col[result].empty( ) ) {
				++result;
			}
			return result;
		}();

		auto finish = start;
		gen_plot.set_pen( graph_config.pen_line_average );
		{
			::std::vector<point_t> points;
			for( auto row = start; row <= data_last; ++row ) {
				if( bg_col[row] ) {
					auto const x( (ts_col[row].timestamp( ) - get_epoch( )).total_seconds( ) / 60 );
					auto const y( static_cast<int>(bg_col[row].real( )*10.0) );
					points.emplace_back( point_t{ x, y } );
					finish = row;
				}
			}
			if( points.size( ) < 2 ) {
				return;
			}
			gen_plot.draw_lines( ::std::move( points ) );
		}

		
		auto const min_point( gen_plot.coord_data( ).item_bounds.point1 );
		const point_t max_point{ value_or_min( gen_plot.coord_data( ).item_bounds.point2.pos( ).x, 100 ), value_or_min( gen_plot.coord_data( ).item_bounds.point2.pos( ).y, 120 ) };
		// Y-Axis
		graph_config.coord_data = gen_plot.coord_data( );
		
		draw_mmol_y_axis( gen_plot, graph_config );
		draw_ts_x_axis( gen_plot, ts_col, start, finish, graph_config );

		gen_plot.set_pen( wxNullPen );
	}


}	// namespace anonymous


PanelDataPlot::PanelDataPlot( wxWindow *parent, ::std::function<void( wxMenuBar* menu )> addmenu_cb, const daw::data::DataTable& dt, const daw::data::DataTable::size_type first, const daw::data::DataTable::size_type last, wxPoint position, wxSize sz ): wxPanel( parent, wxID_ANY, position, sz ), m_data( dt ), m_data_first( first ), m_data_last( last ), m_addmenu_cb( addmenu_cb ), m_gen_plot( ) {

	// create our menu bar: it will be shown instead of the main frame one when
	// we're active
	wxMenuBar *mbar = FramePumpDataAnalysis::create_menu_bar( );
	mbar->GetMenu( 0 )->Insert( 1, wxID_CLOSE, "&Close child\tCtrl-W", "Close this window" );

	// Associate the menu bar with the frame
	
	addmenu_cb( mbar );

	daw::pumpdataanalysis::graph_config_t graph_config;
	graph_config.axis_title_y = "mmol/L";

	setup_graph( m_gen_plot, m_data, m_data_first, m_data_last, ::std::move( graph_config ) );
	
}

void PanelDataPlot::on_paint( wxPaintEvent& ) {
	wxClientDC dc( this );
	wxSize area( GetClientSize( ) );
	if( area.GetWidth( ) > 0 && area.GetHeight( ) > 0 ) {		
		plot( dc, ::std::move( area ) );
	}
}

void PanelDataPlot::plot( wxDC& dc, wxSize bounds ) {
	m_gen_plot.plot( dc, ::std::move( bounds ) );
}

BEGIN_EVENT_TABLE( PanelDataPlot, wxPanel )
EVT_PAINT( PanelDataPlot::on_paint )
END_EVENT_TABLE( )

