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
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/datetime.h>
#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/timectrl.h>

namespace daw {
	namespace wx {
		class DialogDateRangeChooser final: public wxDialog {
			wxDateTime m_start_date;
			wxDateTime m_end_date;
			wxDateTime m_selected_start_date;
			wxDateTime m_selected_end_date;
			wxDatePickerCtrl * m_dp_start;
			wxDatePickerCtrl * m_dp_end;
			wxTimePickerCtrl * m_tp_start;
			wxTimePickerCtrl * m_tp_end;
		
			void on_ok( wxCommandEvent & event );
			void on_cancel( wxCommandEvent & event );
			/// <summary>Handles updating the valid ranges and times/dates in the control.  This allows for ranges that are both partial days and hours</summary>
			/// <param name="event"><c>wxDateEvent</c> that contains data such as source of event</param>
			void on_date_time_range_updated( wxDateEvent & event );

		public:
			/// <summary>Construct a Date Range Chooser Dialog.</summary>
			/// <param name="parent">Parent window</param>
			/// <param name="title">Title of dialog window</param>
			/// <param name="start_date">Start date/time of range chooser</param>
			/// <param name="end_date">End date/time of range chooser</param>
			/// <param name="pos">Position of dialog</param>
			/// <param name="size">Size of dialog</param>
			/// <param name="style">Dialog Style</param>
			DialogDateRangeChooser( wxWindow * parent, wxWindowID id, wxString const & title, boost::posix_time::ptime const & start_date, boost::posix_time::ptime const & end_date, wxPoint const & pos = wxDefaultPosition );			
			/// <summary>Returns a <c>stp::pair</c> with the start and end date/times</summary>
			std::pair<boost::posix_time::ptime, boost::posix_time::ptime> get_selected_range( ) const;
			
			DialogDateRangeChooser( ) = delete;
			DialogDateRangeChooser( DialogDateRangeChooser const & ) = delete;
			DialogDateRangeChooser( DialogDateRangeChooser && ) = default;
			DialogDateRangeChooser & operator=( DialogDateRangeChooser const & ) = delete;
			DialogDateRangeChooser & operator=( DialogDateRangeChooser && ) = default;
			~DialogDateRangeChooser( );
		};	// DialogDateRangeChooser
	}
}

