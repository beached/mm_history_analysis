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

#include <boost/utility/string_ref.hpp>
#include <functional>

#include <daw/daw_algorithm.h>
#include <daw/daw_range_algorithm.h>
#include <daw/daw_string.h>

#include "app_pump_data_analysis.h"
#include "csv_table.h"
#include "dialog_date_range_chooser.h"
#include "frame_pump_data_analysis.h"
#include "panel_average_basal.h"
#include "panel_average_basal_derivative.h"
#include "panel_data_plot.h"
#include "panel_pump_data_analysis.h"
#include "string_helpers.h"

// ---------------------------------------------------------------------------
// Pump Data Grid Child Window
// ---------------------------------------------------------------------------
namespace {
	#if 0
	using namespace daw::data;
	bool should_stop_basal_test( const DataTable& table, const size_t row ) {
		// Columns of impact
		const auto& col_bolus = table["Bolus Volume Delivered (U)"];
		const auto& col_carb = table["BWZ Carb Input (grams)"];
		const auto& col_raw_type = table["Raw-Type"];

		const bool has_manual_food = 0 == col_raw_type[row].string( ).compare( "JournalEntryMealMarker" );	// Has eaten
		const bool has_temp_basal = 0 == col_raw_type[row].string( ).compare( "ChangeTempBasalPercent" );	// Basal dose isn't normal
		const bool has_bolus_wizard_carb = !col_carb[row].empty( );	// Has eaten
		const bool has_bolus_dose = !col_bolus[row].empty( );	// Has taken bolus insulin
		return has_manual_food || has_temp_basal || has_bolus_wizard_carb || has_bolus_dose ;
	}
	#endif
}

enum {
	CWDATAGRID_BASALTESTS = 300
};

size_t PanelPumpDataAnalyis::ms_number_children = 0;

using namespace daw::data;

void PanelPumpDataAnalyis::add_basal_test_page( wxWindow * child, wxString const & title, bool const bring_to_front ) { 
	if( nullptr == m_notebook_basal_tests ) {
		GetBasalTestWindow( );
	}
	m_notebook_basal_tests->AddPage( child, title, bring_to_front );
}

void PanelPumpDataAnalyis::add_top_page( wxWindow * child, wxString const & title, bool const bring_to_front ) { 
	if( nullptr == m_notebook_main ) {
		GetTopPageWindow( );
	}
	m_notebook_main->AddPage( child, title, bring_to_front );
}

void PanelPumpDataAnalyis::add_menu_bar( wxMenuBar* menu ) {
	SetMenuBar( menu );
}

void PanelPumpDataAnalyis::update_status_callback( ::std::string status, wxApp * app ) {
	app->GetTopWindow( )->GetEventHandler( )->CallAfter( ::std::bind( &PanelPumpDataAnalyis::update_status, this, status ) );
}

void PanelPumpDataAnalyis::update_status( ::std::string status ) {
	wxLogStatus( wxString( status ) );
}
namespace {
	bool column_filter( std::string const & header ) {
		using daw::algorithm::contains;
		static const ::std::vector<std::string> disallowed_headers = { "Index", "Time", "Date", "Raw-ID", "Raw-Upload ID", "Raw-Seq Num", "Raw-Device Type" };
		const bool is_disallowed = contains( disallowed_headers, header );
		return !is_disallowed;
	}

}	// namespace anonymous

PanelPumpDataAnalyis::PanelPumpDataAnalyis( wxMDIParentFrame * parent, wxApp * app, ::std::string filename ):
		wxMDIChildFrame{ parent, wxID_ANY, wxString::Format( "Child %llu", ++ms_number_children ) },
		m_notebook_main{ nullptr }, 
		m_notebook_basal_tests{ nullptr },
		m_grid{ nullptr },
		m_app{ app }, 
		m_table_data{ },
		m_filename{ std::move( filename ) }, 
		m_backgroundthread{ } {

	update_status( "Loading CSV Data..." );
	daw::data::parse_csv_data_param params( m_filename, 11, column_filter, [&]( ::std::string status ) {
			update_status_callback( status, app );
	} );

	auto worker = [&, p=std::move( params ), app]( ) {
		m_table_data = CSVTable( p );

		auto handler = app->GetTopWindow( )->GetEventHandler( );

		if( !m_table_data.is_valid( )) {
			// Call Error Handler
			handler->CallAfter( std::bind( &PanelPumpDataAnalyis::on_finished_loading_csv_data_error, this ) );
			return;
		}
		handler->CallAfter(  std::bind( &PanelPumpDataAnalyis::on_finished_loading_csv_data, this ) );
	};
	m_backgroundthread = std::thread( worker );
	m_backgroundthread.detach( );

	const wxString title = "Data: " + m_filename;
	SetTitle( title );
	Show( false );
}

void PanelPumpDataAnalyis::on_finished_loading_csv_data_error( ) {
	update_status( "Error opening file" );
	Close( true );
}

void PanelPumpDataAnalyis::on_finished_loading_csv_data( ) {	
	m_grid = new wxGrid( GetTopPageWindow( ), -1, wxPoint( 0, 0 ), GetClientSize( ) );
	add_top_page( m_grid, wxT( "Raw Data" ) );

	m_grid->SetTable( &m_table_data );	
	m_grid->EnableEditing( false );
	//m_grid->AutoSizeColumns( false );
	//SetIcon( wxICON( chart ) );		//TODO: Get icons

	// create our menu bar: it will be shown instead of the main frame one when
	// we're active
	auto mbar = FramePumpDataAnalysis::create_menu_bar( );
	mbar->GetMenu( 0 )->Insert( 1, wxID_CLOSE, "&Close child\tCtrl-W", "Close this window" );

	auto menuChild = new wxMenu( );

	menuChild->Append( CWDATAGRID_BASALTESTS, "Do Basal Tests" );

	mbar->Insert( 1, menuChild, "&Data Operations" );

	// Associate the menu bar with the frame
	SetMenuBar( mbar );
	update_status( ::std::string( "Loaded " + m_filename ) );
	Maximize( );
	Show( );
}

PanelPumpDataAnalyis::~PanelPumpDataAnalyis( ) {
	if( nullptr != m_grid ) {
		m_grid->SetTable( nullptr );
	}
	m_table_data.Clear( );
	ms_number_children--;
}

void PanelPumpDataAnalyis::on_close( wxCommandEvent& ) {
	m_grid->SetTable( nullptr );
	m_table_data.Clear( );	
	Close( true );
}

void PanelPumpDataAnalyis::on_refresh( wxCommandEvent& ) {
	if( m_notebook_main ) {
		m_notebook_main->Refresh( );
	}
}

void PanelPumpDataAnalyis::on_activate( wxActivateEvent& event ) {
	if( event.GetActive( ) && m_grid ) {
		m_notebook_main->SetFocus( );
	}
}

void PanelPumpDataAnalyis::on_move( wxMoveEvent& event ) {
	event.Skip( );
}

void PanelPumpDataAnalyis::on_size( wxSizeEvent& event ) {
	event.Skip( );
}

void PanelPumpDataAnalyis::on_close_window( wxCloseEvent& event ) {
	event.Skip( );
}

namespace {
	#if 0
	size_t skip_hrs( size_t row, const daw::data::DataTable::value_type& ts_col, const int32_t hours ) {
		const auto time_start = ts_col[row++].timestamp( );
		for( ; row < ts_col.size( ); ++row ) {
			const auto time_now = ts_col[row].timestamp( );
			const auto duration = time_now - time_start;
			if( duration.hours( ) >= hours ) {
				break;
			}
		}
		return row;
	}
	#endif

	template<typename T>
	T for_forward( const T& start, const T& end_exclusive, ::std::function<bool( size_t )> action )  {
		for( T n = start; n < end_exclusive; ++n ) {
			if( action( n ) ) {
				return n;
			}
		}
		return end_exclusive;
	}

	size_t row_from_date( boost::posix_time::ptime dte, const DataTable::value_type& column_timestamp, size_t start_row = 0, size_t end_row = 0 ) {
		if( 0 == end_row ) {
			end_row = column_timestamp.size( );
		}
		auto action = [&dte, &column_timestamp]( size_t n ) -> bool {
			const auto& cur_cell = column_timestamp[n];
			if( cur_cell && cur_cell.timestamp( ) >= dte ) {
				return true;
			}
			return false;
		};
		return for_forward( start_row, end_row, action );
	}

	
	std::pair<size_t, size_t> rows_from_date_range( ::std::pair<boost::posix_time::ptime, boost::posix_time::ptime> date_range, const DataTable::value_type& column_timestamp ) {		
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
}

// void PanelPumpDataAnalyis::on_do_basal_tests( wxCommandEvent& ) {
// 	using daw::algorithm::rbegin2;
// 	using ::std::begin;
// 	daw::wx::DialogDateRangeChooser date_range_selector( this, wxID_ANY, "Look for Basal tests", begin( m_table_data.data( )["Timestamp"] )->timestamp( ), rbegin2( m_table_data.data( )["Timestamp"] )->timestamp( ) );
// 	if( wxOK == date_range_selector.ShowModal( ) ) {
// 		const auto selected_date_range = date_range_selector.get_selected_range( );
// 		const auto minmax_rows = rows_from_date_range( selected_date_range, m_table_data.data( )["Timestamp"] );
// 		m_backgroundthread = ::std::thread( [&, minmax_rows]( ) {
// 			const auto basal_tests = m_table_data.data_analysis( ).basal_tests_in_range( date_range_selector.get_selected_range( ) );
// 			m_app->GetTopWindow( )->GetEventHandler( )->CallAfter( ::std::bind( &PanelPumpDataAnalyis::on_finished_do_basal_tests, this, basal_tests, minmax_rows ) );
// 		} );
// 		m_backgroundthread.detach( );
// 	} else {
// 		std::cout << "";
// 	}
// }

void PanelPumpDataAnalyis::on_do_basal_tests( wxCommandEvent& ) {
	using daw::algorithm::rbegin2;
	using ::std::begin;
	daw::wx::DialogDateRangeChooser date_range_selector( this, wxID_ANY, "Look for Basal tests", begin( m_table_data.data( )["Timestamp"] )->timestamp( ), rbegin2( m_table_data.data( )["Timestamp"] )->timestamp( ) );
	if( wxOK == date_range_selector.ShowModal( ) ) {
		const auto selected_date_range = date_range_selector.get_selected_range( );
		const auto date_range = rows_from_date_range( selected_date_range, m_table_data.data( )["Timestamp"] );
		const auto basal_tests = m_table_data.data_analysis( ).basal_tests_in_range( date_range_selector.get_selected_range( ) );
		const auto cb = ::std::bind( &PanelPumpDataAnalyis::add_menu_bar, this, ::std::placeholders::_1 );
		for( const auto& period : basal_tests ) {
			auto plot = new PanelDataPlot( GetBasalTestWindow( ), cb, m_table_data.data( ), period.first, period.second, wxDefaultPosition, GetBasalTestWindow( )->GetClientSize( ) );
			const auto& col_ts = m_table_data.data( )["Timestamp"];
			std::string title = daw::string::ptime_to_string( col_ts[period.first].timestamp( ), "%Y-%m-%d %H:%M" ) + " -> " + daw::string::ptime_to_string( col_ts[period.second].timestamp( ), "%Y-%m-%d %H:%M" );
			add_basal_test_page( plot, title );
		}
		auto avgBasal = new PanelAverageBasal( GetTopPageWindow( ), cb, m_table_data.data( ), basal_tests );
		add_top_page( avgBasal, wxT( "Aggregate Basal Day" ) );
		auto avgDay = new PanelAverageBasal( GetTopPageWindow( ), cb, m_table_data.data( ), { { date_range.first, date_range.second } } );
		add_top_page( avgDay, wxT( "Average Day in Range" ) );
		auto avgBasalDeriv = new PanelAverageBasalDerivative( GetTopPageWindow( ), cb, m_table_data.data( ), basal_tests );
		add_top_page( avgBasalDeriv, wxT( "Average Basal Change" ), true );
	} else {
		std::cout << "";
	}
}

// void PanelPumpDataAnalyis::on_finished_do_basal_tests( const ::std::vector<std::pair<daw::data::DataTable::size_type, daw::data::DataTable::size_type>> positions, const ::std::pair<size_t, size_t> date_range ) {
// 	const auto cb = ::std::bind( &PanelPumpDataAnalyis::add_menu_bar, this, ::std::placeholders::_1 );
// 	for( const auto& period : positions ) {		
// 		auto plot = new PanelDataPlot( GetBasalTestWindow( ), cb, m_table_data.data( ), period.first, period.second, wxDefaultPosition, GetBasalTestWindow( )->GetClientSize( ) );
// 		const auto& col_ts = m_table_data.data( )["Timestamp"];
// 		std::string title = daw::string::ptime_to_string( col_ts[period.first].timestamp( ), "%Y-%m-%d %H:%M" ) + " -> " + daw::string::ptime_to_string( col_ts[period.second].timestamp( ), "%Y-%m-%d %H:%M" );
// 		add_basal_test_page( plot, title );
// 	}
// 	auto avgBasal = new PanelAverageBasal( GetTopPageWindow( ), cb, m_table_data.data( ), positions );
// 	add_top_page( avgBasal, wxT( "Aggregate Basal Day" ) );
// 	auto avgDay = new PanelAverageBasal( GetTopPageWindow( ), cb, m_table_data.data( ), { { date_range.first, date_range.second } } );
// 	add_top_page( avgDay, wxT( "Average Day in Range" ) );
// 	auto avgBasalDeriv = new PanelAverageBasalDerivative( GetTopPageWindow( ), cb, m_table_data.data( ), positions );
// 	add_top_page( avgBasalDeriv, wxT( "Average Basal Change" ), true );
// }

BEGIN_EVENT_TABLE( PanelPumpDataAnalyis, wxMDIChildFrame )
	EVT_MENU( wxID_CLOSE, PanelPumpDataAnalyis::on_close )
	EVT_MENU( MDI_REFRESH, PanelPumpDataAnalyis::on_refresh )
	EVT_MENU( CWDATAGRID_BASALTESTS, PanelPumpDataAnalyis::on_do_basal_tests )
	EVT_SIZE( PanelPumpDataAnalyis::on_size )
	EVT_MOVE( PanelPumpDataAnalyis::on_move )
	EVT_CLOSE( PanelPumpDataAnalyis::on_close_window )
END_EVENT_TABLE( )
