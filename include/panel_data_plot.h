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

#include <functional>
#include <wx/wx.h>

#include <daw/csv_helper/data_common.h>

#include "panel_generic_plot.h"

//////////////////////////////////////////////////////////////////////////
/// <summary>Display a Basal Test graphically</summary>
//////////////////////////////////////////////////////////////////////////
class PanelDataPlot: public wxPanel {
	const daw::data::DataTable& m_data;
	const daw::data::DataTable::size_type m_data_first;
	const daw::data::DataTable::size_type m_data_last;
	const ::std::function<void( wxMenuBar* menu )> m_addmenu_cb;
public:
	PanelDataPlot( wxWindow *parent, ::std::function<void( wxMenuBar* menu )> addmenu_cb, const daw::data::DataTable& dt, const daw::data::DataTable::size_type first, const daw::data::DataTable::size_type last, wxPoint position = wxDefaultPosition, wxSize sz = wxDefaultSize );

private:	
	daw::pumpdataanalysis::PanelGenericPlotter m_gen_plot;
	void on_paint( wxPaintEvent& event );
	void plot( wxDC& dc, wxSize bounds );

	DECLARE_EVENT_TABLE( )
};

