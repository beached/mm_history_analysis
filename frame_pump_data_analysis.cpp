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

#include <array>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <wx/wx.h>

#include <daw/daw_algorithm.h>

#include "defs.h"
#include "dialog_date_range_chooser.h"
#include "frame_pump_data_analysis.h"
#include "panel_pump_data_analysis.h"

// ---------------------------------------------------------------------------
// PumpDataAnalysisFrame
// ---------------------------------------------------------------------------

FramePumpDataAnalysis::FramePumpDataAnalysis( wxApp * app ): 
		wxMDIParentFrame( nullptr, wxID_ANY, "Pump Data Analysis (c) 2014 Darrell Wright - ", wxDefaultPosition, wxSize( 800, 600 ) ),
		m_wxapp( app ) {
//	SetIcon( wxICON( sample ) );
	SetMenuBar( create_menu_bar( ) );

	auto windowMenu = GetWindowMenu( );
	if( windowMenu ) {
		windowMenu->SetLabel( wxID_MDI_WINDOW_TILE_HORZ, "&Tile horizontally\tCtrl-Shift-H" );
		windowMenu->SetLabel( wxID_MDI_WINDOW_TILE_VERT, "&Tile vertically\tCtrl-Shift-V" );
		windowMenu->SetHelpString( wxID_MDI_WINDOW_CASCADE, "Arrange windows in cascade" );
		windowMenu->Delete( wxID_MDI_WINDOW_ARRANGE_ICONS );
		windowMenu->AppendSeparator( );
		windowMenu->Append( wxID_CLOSE_ALL, "&Close all windows\tCtrl-Shift-C", "Close all open windows" );
		SetWindowMenu( windowMenu );
	}

	CreateStatusBar( );
	
	::std::array<wxAcceleratorEntry,3> entries;
	entries[0].Set( wxACCEL_CTRL, (int) 'O', wxID_OPEN );
	entries[1].Set( wxACCEL_CTRL, (int) 'X', wxID_EXIT );
	entries[2].Set( wxACCEL_CTRL, (int) 'A', wxID_ABOUT );
	wxAcceleratorTable accel( entries.size( ), entries.data( ) );
	SetAcceleratorTable( accel );
}

wxMenuBar * FramePumpDataAnalysis::create_menu_bar( ) {
	auto menuFile = new wxMenu( );
	menuFile->Append( wxID_OPEN, "&Open\tCtrl-O", "Open datafile" );
	menuFile->Append( wxID_EXIT, "&Exit\tAlt-X", "Quit the program" );

	auto menuHelp = new wxMenu( );
	menuHelp->Append( wxID_ABOUT, "&About\tF1" );

	auto mbar = new wxMenuBar( );
	mbar->Append( menuFile, "&File" );
	mbar->Append( menuHelp, "&Help" );

	return mbar;
}

void FramePumpDataAnalysis::on_close( wxCloseEvent & event ) {
	auto numChildren = PanelPumpDataAnalyis::GetChildrenCount( );
	if( event.CanVeto( ) && (numChildren > 0) ) {
		wxString msg;
		msg.Printf( "%d windows still open, close anyhow?", numChildren );
		if( wxMessageBox( msg, "Please confirm", wxICON_QUESTION | wxYES_NO ) != wxYES ) {
			event.Veto( );
			return;
		}
	}
	event.Skip( );
}

void FramePumpDataAnalysis::on_quit( wxCommandEvent & ) {
	Close( );
}

void FramePumpDataAnalysis::on_about( wxCommandEvent & ) {
	wxMessageBox( "Pump Data Analysis\nAuthor: Darrell Wright (c) 2014\n", "About Pump Data Analysis" );
}

void FramePumpDataAnalysis::on_file_open( wxCommandEvent & ) {
	wxFileDialog openFileDialog{ this, _( "Open Pump Data File" ), "", "", "Medtronic Carelink CSV Export files (*.csv)|*.csv", wxFD_OPEN | wxFD_FILE_MUST_EXIST };
	if( wxID_CANCEL == openFileDialog.ShowModal( ) ) {
		return;
	}
	auto fpath = openFileDialog.GetPath( ).ToStdString( );
	auto ppda = new PanelPumpDataAnalyis( this, m_wxapp, fpath );
	auto subframe = daw::exception::dbg_throw_on_null_or_return( ppda, ": Error creating new PanelPumpDataAnalysis" );
	Unused( subframe );
}

void FramePumpDataAnalysis::on_close_all( wxCommandEvent & ) {
	for( auto & child: GetChildren( ) ) {
		if( wxDynamicCast( child, wxMDIChildFrame ) ) {
			child->Close( );
		}
	}
}

void FramePumpDataAnalysis::on_size( wxSizeEvent& ) { }

// ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------
BEGIN_EVENT_TABLE( FramePumpDataAnalysis, wxMDIParentFrame )
	EVT_MENU( wxID_ABOUT, FramePumpDataAnalysis::on_about )
	EVT_MENU( wxID_OPEN, FramePumpDataAnalysis::on_file_open )
	EVT_MENU( wxID_EXIT, FramePumpDataAnalysis::on_quit )
	EVT_MENU( wxID_CLOSE_ALL, FramePumpDataAnalysis::on_close_all )
	EVT_CLOSE( FramePumpDataAnalysis::on_close )
END_EVENT_TABLE( )

