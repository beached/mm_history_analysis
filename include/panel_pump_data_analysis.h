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

#include <thread>
#include <vector>
#include <wx/notebook.h>
#include <wx/wx.h>

#include <daw/csv_helper/data_common.h>

#include "csv_table.h"

//////////////////////////////////////////////////////////////////////////
/// <summary>Pump Data Grid Child Window</summary>
//////////////////////////////////////////////////////////////////////////
struct PanelPumpDataAnalyis: public wxMDIChildFrame {

	PanelPumpDataAnalyis( wxMDIParentFrame * parent, wxApp * app, ::std::string filename );
	virtual ~PanelPumpDataAnalyis( );

	static size_t GetChildrenCount( ) { return ms_number_children; }

	void add_basal_test_page( wxWindow * child, wxString const & title, bool const bring_to_front = false );
	void add_top_page( wxWindow * child, wxString const & title, bool const bring_to_front = false );
	void add_menu_bar( wxMenuBar* menu );
	
	inline wxWindow* GetBasalTestWindow( ) {
		if( nullptr == m_notebook_basal_tests ) {
			m_notebook_basal_tests = new wxNotebook( GetTopPageWindow( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM );
			add_top_page( m_notebook_basal_tests, wxT( "Basal Tests" ) );
		}
		return m_notebook_basal_tests;
	}
	
	inline wxWindow* GetTopPageWindow( ) {
		if( nullptr == m_notebook_main ) {
			m_notebook_main = new wxNotebook( this, wxID_ANY, wxPoint( 0, 0 ), GetClientSize( ), wxNB_TOP );
		}
		return m_notebook_main;
	}

	void on_activate( wxActivateEvent& event );
	void on_refresh( wxCommandEvent& event );
	void on_update_refresh( wxUpdateUIEvent& event );
	void on_change_title( wxCommandEvent& event );
	void on_close( wxCommandEvent& event );
	void on_size( wxSizeEvent& event );
	void on_move( wxMoveEvent& event );
	void on_close_window( wxCloseEvent& event );
	void on_do_basal_tests( wxCommandEvent& event );
	void on_do_correction_tests( wxCommandEvent& event );
	void on_finished_loading_csv_data_error( );
	void on_finished_loading_csv_data( );
	void on_finished_do_basal_tests( const ::std::vector<std::pair<daw::data::DataTable::size_type, daw::data::DataTable::size_type>> positions, const ::std::pair<size_t, size_t> date_range );
	void on_finished_do_correction_tests( const ::std::vector<std::pair<daw::data::DataTable::size_type, daw::data::DataTable::size_type>> positions, const ::std::pair<size_t, size_t> date_range );
	void update_status( ::std::string status ) const;
	void update_status_callback( ::std::string status, wxApp* app );

private:
	static size_t ms_number_children;
	wxNotebook * m_notebook_main;
	wxNotebook * m_notebook_basal_tests;
	wxGrid * m_grid;
	wxApp * m_app;
	daw::data::CSVTable m_table_data;
	::std::string const m_filename;
	std::thread m_backgroundthread;

	DECLARE_EVENT_TABLE( )
};

