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
#include <memory>
#include <string>
#include <vector>
#include <wx/wx.h>

#include <daw/csv_helper/data_table.h>

#include "panel_generic_plot.h"

namespace daw {
	namespace pumpdataanalysis {
		struct translation_t;

		class point_t final {
			wxPoint m_point;			
			union {
				struct {
					int8_t m_x_offset : 8;
					bool m_x_unmapped : 1;
					int8_t m_y_offset : 8;
					bool m_y_unmapped : 1;
					int : 0;
				};
				uintptr_t m_bitfield;
			};
		public:
			explicit point_t( int x = 0, int y = 0, int8_t off_x = 0, int8_t off_y = 0, bool x_unmapped = false, bool y_unmapped = false );
			point_t( point_t const & ) = default;
			point_t( point_t && ) = default;
			point_t & operator=( point_t const & ) = default;
			point_t & operator=( point_t && ) = default;
			~point_t( ) = default;

			wxPoint& pos( );
			wxPoint const & pos( ) const;

			wxPoint get_offset( ) const;
			void set_offset( wxPoint const & offset );

			bool get_unmapped_x( ) const;
			bool get_unmapped_y( ) const;

			void set_unmapped_x( bool unmapped );
			void set_unmapped_y( bool unmapped );

			wxPoint mapped_point( translation_t const & coord_data ) const;
			static wxPoint mapped_point( point_t const & point, translation_t const & coord_data );

			point_t& operator+=( point_t const & rhs);

		};	// point_t

		point_t operator+(point_t lhs, point_t const & rhs);

		struct box_t {
			point_t point1;
			point_t point2;
			box_t( );
			box_t( point_t point2 );
			box_t( point_t p1, point_t p2 );
			int width( ) const;
			int height( ) const;
		};
		
		struct translation_t {
			translation_t( );

			struct scale_t {
				float x;
				float y;
				scale_t( );
			} scale;

			box_t item_bounds;
			wxSize panel_bounds;			

			struct margin_t {
				size_t top;
				size_t bottom;
				size_t left;
				size_t right;

				margin_t( );
				void set_all( size_t sz );
				size_t width( ) const;
				size_t height( ) const;
			} margins;
		};

		struct graph_config_t {
			graph_config_t( );
			wxPen pen_line_average;
			wxPen pen_line_high;
			wxPen pen_line_low;
			wxPen pen_line_count;
			wxPen pen_area_std_dev;
			wxPen pen_axis_x;
			wxPen pen_axis_y;
			wxPen pen_axis_dotted;
			wxPen pen_axis_count;
			wxFont fnt_axis_title;
			wxFont fnt_axis_title_bold;
			wxBrush brush_area_std_dev;
			::std::string axis_title_x;
			::std::string axis_title_y;
			::std::string axis_title_count;
			translation_t coord_data;
		};

		namespace impl {
			struct PanelGenericPlotAction {
				using mapping_cb_t = ::std::function<int( int, translation_t const & )>;
			private:
				mapping_cb_t mf_map_x;
				mapping_cb_t mf_map_y;
			public:
				virtual ~PanelGenericPlotAction( );
				virtual void do_plot( wxDC &, translation_t & ) = 0;

				wxPoint map_point( point_t point, translation_t& translate_data ) const;
			};	// PanelGenericPlotAction

		}	// namespace impl
		/////////////////////////////////////////////////////////////////////////////////////////////
		/// <summary>Takes all the data and adjusts bounds and scaling based on the extents of the data</summary>
		/////////////////////////////////////////////////////////////////////////////////////////////
		struct PanelGenericPlotter {
			PanelGenericPlotter( );

			translation_t& coord_data( );
			const translation_t& coord_data( ) const;

			void set_pen( wxPen pen );
			void set_font( wxFont font );
			void set_brush( wxBrush brush );

			void draw_text( wxString text, point_t point );
			void draw_rotated_text( wxString text, point_t point, double angle );
			void draw_line( point_t p1, point_t p2 );
			void draw_lines( ::std::vector<point_t> points );
			void draw_polygon( ::std::vector<point_t> points );
			void update_scale( wxSize bounds );
			void plot( wxDC& dc, wxSize bounds );
			void clear( );
			int get_mapped_x( int x ) const;
			int get_mapped_y( int y ) const;
			point_t get_text_size( const wxString& text ) const;
			box_t get_rotated_text_size( const wxString& text, double angle ) const;			
			void check_minmax( point_t point );
			void check_minmax( box_t points );
		private:
			translation_t m_coord_data;
			::std::vector<::std::unique_ptr<impl::PanelGenericPlotAction>> m_actions;
			wxFont m_last_font;
		};
		void draw_mmol_y_axis( PanelGenericPlotter& gen_plot, graph_config_t graph_config, float at_least_y_values = 10.0f );
		void draw_ts_x_axis( PanelGenericPlotter& gen_plot, const daw::data::DataTable::value_type& ts_col, size_t start, size_t finish, graph_config_t graph_config, float at_least_y_values = 10.0f );
		void draw_24hr_x_axis( PanelGenericPlotter& gen_plot, int increment_size, graph_config_t graph_config, float at_least_y_values = 10.0f );
	}	// namespace pumpdataanalysis
} // namespace daw

