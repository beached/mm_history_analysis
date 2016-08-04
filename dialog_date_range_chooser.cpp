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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <wx/wx.h>

#include "defs.h"
#include "dialog_date_range_chooser.h"

namespace daw {
	namespace wx {
		using boost::posix_time::ptime;
		namespace {
			ptime to_ptime( const wxDateTime &dt ) {
				if( !dt.IsValid( ) ) {
					return boost::posix_time::not_a_date_time;
				} else {
					return boost::posix_time::from_time_t( dt.GetTicks( ) );
				}
			}
		}	// namespace anonymous

		DialogDateRangeChooser::DialogDateRangeChooser( wxWindow* parent, wxWindowID id, const wxString& title, const boost::posix_time::ptime& start_date, const boost::posix_time::ptime& end_date, const wxPoint& pos ): wxDialog( parent, id, title, pos, wxSize( 225, 165 ), wxDEFAULT_DIALOG_STYLE ), m_start_date( boost::posix_time::to_tm( start_date ) ), m_end_date( boost::posix_time::to_tm( end_date ) ), m_selected_start_date( m_start_date ), m_selected_end_date( m_end_date ), m_dp_start( nullptr ), m_dp_end( nullptr ), m_tp_start( nullptr ), m_tp_end( nullptr ) {
			m_dp_start = new wxDatePickerCtrl( this, wxID_ANY, m_start_date );
			m_dp_start->SetRange( m_start_date, m_end_date );
			m_tp_start = new wxTimePickerCtrl( this, wxID_ANY, m_start_date );
			auto vbox_start = new wxStaticBoxSizer( wxVERTICAL, this, "Start at" );
			vbox_start->Add( m_dp_start );
			vbox_start->AddSpacer( 5 );
			vbox_start->Add( m_tp_start );
			m_dp_end = new wxDatePickerCtrl( this, wxID_ANY, m_end_date );
			m_dp_end->SetRange( m_start_date, m_end_date );
			m_tp_end = new wxTimePickerCtrl( this, wxID_ANY, m_end_date );
			auto vbox_end = new wxStaticBoxSizer( wxVERTICAL, this, "End at" );
			vbox_end->Add( m_dp_end );
			vbox_end->AddSpacer( 5 );
			vbox_end->Add( m_tp_end );

			auto hbox_date = new wxStaticBoxSizer( wxHORIZONTAL, this, "Date Range" );
			hbox_date->Add( vbox_start );
			hbox_date->AddSpacer( 10 );
			hbox_date->Add( vbox_end );

			auto btn_ok = new wxButton( this, wxID_OK, wxT( "Ok" ), wxDefaultPosition, wxSize( 70, 30 ) );
			btn_ok->SetDefault( );
			auto btn_cancel = new wxButton( this, wxID_CANCEL, wxT( "Cancel" ), wxDefaultPosition, wxSize( 70, 30 ) );

			auto hbox = new wxStdDialogButtonSizer( );
			hbox->AddButton( btn_ok );
			hbox->AddButton( btn_cancel );
			hbox->Realize( );

			auto vbox = new wxBoxSizer( wxVERTICAL );
			vbox->Add( hbox_date, 2, wxRIGHT | wxLEFT | wxTOP | wxBOTTOM | wxCENTRE | wxALIGN_CENTRE_VERTICAL | wxEXPAND, 15 );
			vbox->Add( hbox, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxRIGHT | wxBOTTOM, 10 );
			vbox->SetSizeHints( this );
			SetSizer( vbox );

			Centre( );

			Bind( wxEVT_DATE_CHANGED, &daw::wx::DialogDateRangeChooser::on_date_time_range_updated, this, wxID_ANY );
			Bind( wxEVT_TIME_CHANGED, &daw::wx::DialogDateRangeChooser::on_date_time_range_updated, this, wxID_ANY );
			btn_ok->Bind( wxEVT_BUTTON, &daw::wx::DialogDateRangeChooser::on_ok, this, wxID_ANY );
			btn_cancel->Bind( wxEVT_BUTTON, &daw::wx::DialogDateRangeChooser::on_cancel, this, wxID_ANY );
		}

		std::pair<boost::posix_time::ptime, boost::posix_time::ptime> DialogDateRangeChooser::get_selected_range( ) const {
			return{ to_ptime( m_selected_start_date ), to_ptime( m_selected_end_date ) };
		}

		void DialogDateRangeChooser::on_date_time_range_updated( wxDateEvent& event ) {
			const auto ptr_origin = reinterpret_cast<uintptr_t>(event.GetEventObject( ));
			const auto ptr_dp_start = reinterpret_cast<uintptr_t>(m_dp_start);
			const auto ptr_dp_end = reinterpret_cast<uintptr_t>(m_dp_end);
			const auto ptr_tp_start = reinterpret_cast<uintptr_t>(m_tp_start);
			const auto ptr_tp_end = reinterpret_cast<uintptr_t>(m_tp_end);

			if( ptr_origin == ptr_dp_start ) {
				const wxDateTime selected_start_date( m_dp_start->GetValue( ) );
				m_selected_start_date.SetYear( selected_start_date.GetYear( ) );
				m_selected_start_date.SetMonth( selected_start_date.GetMonth( ) );
				m_selected_start_date.SetDay( selected_start_date.GetDay( ) );				
			} else if( ptr_origin == ptr_dp_end ) {
				const wxDateTime selected_end_date( m_dp_end->GetValue( ) );
				m_selected_end_date.SetYear( selected_end_date.GetYear( ) );
				m_selected_end_date.SetMonth( selected_end_date.GetMonth( ) );
				m_selected_end_date.SetDay( selected_end_date.GetDay( ) );
			} else if( ptr_origin == ptr_tp_start ) {
				const wxDateTime selected_start_time( m_tp_start->GetValue( ) );
				m_selected_start_date.SetHour( selected_start_time.GetHour( ) );
				m_selected_start_date.SetMinute( selected_start_time.GetMinute( ) );
				m_selected_start_date.SetSecond( 0 );
				m_tp_start->SetValue( m_selected_start_date );
			} else if( ptr_origin == ptr_tp_end ) {				
				const wxDateTime selected_end_time( m_tp_end->GetValue( ) );
				m_selected_end_date.SetHour( selected_end_time.GetHour( ) );
				m_selected_end_date.SetMinute( selected_end_time.GetMinute( ) );				
				m_selected_end_date.SetSecond( 0 );
				m_tp_end->SetValue( m_selected_end_date );
			} else {
				throw ::std::runtime_error( ": Unexpected event object" );
			}			
			if( m_selected_start_date >= m_selected_end_date ) {
				m_selected_start_date = m_selected_end_date - wxTimeSpan( 0, 1 );
				m_tp_start->SetValue( m_selected_start_date );
			}
			if( m_selected_start_date < m_start_date ) {
				m_selected_start_date = m_start_date;
				m_tp_start->SetValue( m_selected_start_date );
			}
			if( m_selected_end_date > m_end_date ) {
				m_selected_end_date = m_end_date;
				m_tp_end->SetValue( m_selected_end_date );				
			}
			m_dp_start->Refresh( );
			m_dp_end->Refresh( );
			m_tp_start->Refresh( );
			m_tp_end->Refresh( );
		}

		void DialogDateRangeChooser::on_ok( wxCommandEvent& ) {
			if( IsModal( ) ) {
				EndModal( wxOK ); // If modal
			} else {
				SetReturnCode( wxOK );
				Show( false ); // If modeless
			}
		}

		void DialogDateRangeChooser::on_cancel( wxCommandEvent& ) {
			if( IsModal( ) ) {				
				EndModal( wxCANCEL ); // If modal
			} else {
				SetReturnCode( wxCANCEL );
				Show( false ); // If modeless
			}
		}

	}	// Namespace wx
}	// Namespace daw
