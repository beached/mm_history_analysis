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

#include <array>
#include <cinttypes>
#include <memory>
#include <utility>
#include <vector>
#include <wx/wx.h>

#include <daw/csv_helper/data_common.h>
#include <daw/daw_math.h>

#include "aggregate_data.h"
#include "panel_generic_plot.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>Display an aggregate of all basal test derivatives(slope over a 24hr period</summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class PanelAverageBasalDerivative: public wxPanel {
	const daw::data::DataTable& m_data;
	const ::std::vector<std::pair<size_t, size_t>> m_basal_positions;
	daw::data::real_t m_bg_min;
	daw::data::real_t m_bg_max;
	const int32_t m_points_x = 24 * 12;
	std::unique_ptr<daw::AggregateDataVector<daw::data::real_t>> m_aggregate_vec;
	const ::std::function<void( wxMenuBar* menu )> m_addmenu_cb;
public:
	PanelAverageBasalDerivative( wxWindow *parent, ::std::function<void( wxMenuBar* menu )> addmenu_cb, const daw::data::DataTable& dt, const ::std::vector<std::pair<daw::data::DataTable::size_type, daw::data::DataTable::size_type>> positions );

private:
	daw::pumpdataanalysis::PanelGenericPlotter m_gen_plot;
	void on_paint( wxPaintEvent& event );
	void plot( wxDC& dc, wxSize bounds );
	DECLARE_EVENT_TABLE( )
};

