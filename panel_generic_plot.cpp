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

#include <daw/csv_helper/data_table.h>
#include <daw/csv_helper/data_types.h>
#include <daw/daw_algorithm.h>
#include <daw/daw_exception.h>
#include <daw/daw_math.h>
#include <daw/daw_string.h>

#include "defs.h"
#include "panel_generic_plot.h"
#include "string_helpers.h"

namespace daw {
	namespace pumpdataanalysis {
		using namespace daw::algorithm;
		using namespace daw::exception;

		graph_config_t::graph_config_t( ): 
				pen_line_average{ *wxBLUE, 3, wxSOLID }, 
				pen_line_high{ { 255, 140, 0 }/*orange*/, 3, wxSOLID }, 
				pen_line_low{ *wxGREEN, 3, wxSOLID }, 
				pen_line_count{ { 49, 0, 98 }/*purple*/, 2, wxSOLID }, 
				pen_area_std_dev{ { 192, 192, 255 }/*light blue*/, 1, wxPENSTYLE_SOLID }, 
				pen_axis_x{ *wxRED, 2, wxSOLID }, 
				pen_axis_y{ *wxRED, 2, wxSOLID }, 
				pen_axis_dotted{ *wxRED, 1, wxDOT }, 
				pen_axis_count{ *wxBLUE, 2, wxSOLID }, 
				fnt_axis_title{ 10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL }, 
				fnt_axis_title_bold{ 10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD }, 
				brush_area_std_dev{ { 192, 192, 255 }/*light blue*/, wxSOLID }, 
				axis_title_x( ), 
				axis_title_y( ), 
				axis_title_count( ), 
				coord_data( ) { }

		namespace impl {
			namespace {
				auto get_mapped_x = []( int x, const translation_t& coord_data ) {
					const auto& min_point = coord_data.item_bounds.point1;
					const auto& scale = coord_data.scale;
					return static_cast<int>((x - min_point.pos( ).x)*scale.x + coord_data.margins.left);
				};

				auto get_unmapped_x = []( int x, const translation_t& coord_data ) {
					const auto& min_point = coord_data.item_bounds.point1;
					const auto& scale = coord_data.scale;
					return static_cast<int>( static_cast<daw::data::real_t>(x - coord_data.margins.left) / scale.x) + min_point.pos( ).x;
				};

				auto get_mapped_y = []( int y, const translation_t& coord_data ) {
					const auto& min_point = coord_data.item_bounds.point1;
					const auto& scale = coord_data.scale;
					return coord_data.panel_bounds.GetHeight( ) - static_cast<int>((y - min_point.pos( ).y)*scale.y + coord_data.margins.bottom);
				};

				auto get_unmapped_y = []( int y, const translation_t& coord_data ) {
					const auto& min_point = coord_data.item_bounds.point1;
					const auto& scale = coord_data.scale;
					return static_cast<int>(static_cast<daw::data::real_t>((coord_data.panel_bounds.GetHeight( )-y) - coord_data.margins.bottom) / scale.y) + min_point.pos( ).y;
				};

			}	//namespace anonymous
		}	//namespace impl

		namespace {
			const int s_min_int = ::std::numeric_limits<int>::min( );
			const int s_max_int = ::std::numeric_limits<int>::max( );

			box_t rotate_by( box_t points, double angle ) {
				const auto L( points.point2.pos( ).x - points.point1.pos( ).x );
				if( 0 == L ) {
					return points;	// Cannot rotate about a zero length box_t 
				}
				const auto H( points.point2.pos( ).y - points.point1.pos( ).y );

				const auto D( sqrt( L*L + H*H ) );
				const auto c( angle + atan2( H, L ) );
				points.point2.pos( ).x = points.point1.pos( ).x + D*cos( c );
				points.point2.pos( ).y = points.point1.pos( ).y + D*sin( c );
				return ::std::move( points );
			}

			const auto s_epoch = boost::posix_time::time_from_string( "1970-01-01 00:00:00.000" );

		}	// namespace anonymous
		point_t::point_t( int x, int y, int8_t off_x, int8_t off_y, bool x_unmapped, bool y_unmapped ):
				m_point( ::std::move( x ), ::std::move( y ) ),
				m_x_offset( off_x ),
				m_x_unmapped( x_unmapped ),
				m_y_offset( off_y ),
				m_y_unmapped( y_unmapped ) { }

		wxPoint& point_t::pos( ) {
			return m_point;
		}

		const wxPoint& point_t::pos( ) const {
			return m_point;
		}

		wxPoint point_t::get_offset( ) const {
			return{ m_x_offset, m_y_offset };
		}

		wxPoint point_t::mapped_point( const point_t& point, const translation_t& coord_data ) {
			wxPoint result( point.pos( ) );
			if( !point.get_unmapped_x( ) ) {
				result.x = impl::get_mapped_x( result.x, coord_data );
			}
			if( !point.get_unmapped_y( ) ) {
				result.y = impl::get_mapped_y( result.y, coord_data );
			}
			const auto& offset = point.get_offset( );
			result.x += offset.x;
			result.y -= offset.y;	// Panel y-axis is inverted
			return ::std::move( result );
		}

		wxPoint point_t::mapped_point( const translation_t& coord_data ) const {
			return mapped_point( *this, coord_data );
		}

		void point_t::set_offset( const wxPoint& offset ) {
			dbg_throw_on_false( offset.x <= ::std::numeric_limits<decltype(m_x_offset)>::max( ) && offset.x >= ::std::numeric_limits<decltype(m_x_offset)>::min( ), ": X Offset out of range [-128,128)" );
			dbg_throw_on_false( offset.y <= ::std::numeric_limits<decltype(m_y_offset)>::max( ) && offset.y >= ::std::numeric_limits<decltype(m_y_offset)>::min( ), ": Y Offset out of range [-128,128)" );
			m_x_offset = offset.x;
			m_y_offset = offset.y;
		}

		bool point_t::get_unmapped_x( ) const {
			return m_x_unmapped;
		}

		bool point_t::get_unmapped_y( ) const {
			return m_y_unmapped;
		}

		void point_t::set_unmapped_x( bool unmapped ) {
			m_x_unmapped = unmapped;
		}

		void point_t::set_unmapped_y( bool unmapped ) {
			m_y_unmapped = unmapped;
		}

		point_t& point_t::operator+=(const point_t& rhs) {
			m_point += rhs.m_point;
			m_x_offset += rhs.m_x_offset;
			m_y_offset += rhs.m_y_offset;
			m_x_unmapped |= rhs.m_x_unmapped;
			m_y_unmapped |= rhs.m_y_unmapped;
			return *this;
		}

		point_t operator+(point_t lhs, const point_t& rhs) {
			lhs += rhs;
			return ::std::move( lhs );
		}

		namespace impl {
			PanelGenericPlotAction::~PanelGenericPlotAction( ) { }

			class PanelGenericPlotActionPen: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionPen( wxPen pen ):PanelGenericPlotAction( ), m_pen( ::std::move( pen ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& ) {
					dc.SetPen( m_pen );
				}
			private:
				const wxPen m_pen;
			};

			class PanelGenericPlotActionFont: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionFont( wxFont font ):PanelGenericPlotAction( ), m_font( ::std::move( font ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& ) {
					dc.SetFont( m_font );
				}
			private:
				const wxFont m_font;
			};

			class PanelGenericPlotActionBrush: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionBrush( wxBrush brush ):PanelGenericPlotAction( ), m_brush( ::std::move( brush ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& ) {
					dc.SetBrush( m_brush );
				}
			private:
				const wxBrush m_brush;
			};

			class PanelGenericPlotActionDrawText: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionDrawText( wxString text, point_t point ):PanelGenericPlotAction( ), m_text( ::std::move( text ) ), m_point( ::std::move( point ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& coord_data ) {
					dc.DrawText( m_text, m_point.mapped_point( coord_data ) );
				}
			private:
				const wxString m_text;
				const point_t m_point;
			};

			class PanelGenericPlotActionDrawRotatedText: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionDrawRotatedText( wxString text, point_t point, double angle ): PanelGenericPlotAction( ), m_text( ::std::move( text ) ), m_point( ::std::move( point ) ), m_angle( ::std::move( angle ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& coord_data ) {
					dc.DrawRotatedText( m_text, m_point.mapped_point( coord_data ), m_angle );
				}
			private:
				const wxString m_text;
				const point_t m_point;
				const double m_angle;
			};

			class PanelGenericPlotActionDrawLine: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionDrawLine( point_t p1, point_t p2 ): PanelGenericPlotAction( ), m_point1( ::std::move( p1 ) ), m_point2( ::std::move( p2 ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& coord_data ) {
					dc.DrawLine( m_point1.mapped_point( coord_data ), m_point2.mapped_point( coord_data ) );
				}
			private:
				const point_t m_point1;
				const point_t m_point2;
			};

			class PanelGenericPlotActionDrawLines: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionDrawLines( ::std::vector<point_t> points ):PanelGenericPlotAction( ), m_points( ::std::move( points ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& coord_data ) {
					::std::vector<wxPoint> points;
					points.reserve( m_points.size( ) );
					for( const auto& point : m_points ) {
						points.emplace_back( point.mapped_point( coord_data ) );
					}
					dc.DrawLines( points.size( ), daw::algorithm::to_array( points ) );
				}
			private:
				const ::std::vector<point_t> m_points;
			};

			class PanelGenericPlotActionDrawPolygon: public PanelGenericPlotAction {
			public:
				PanelGenericPlotActionDrawPolygon( ::std::vector<point_t> points ):PanelGenericPlotAction( ), m_points( ::std::move( points ) ) { }

				virtual void do_plot( wxDC& dc, translation_t& coord_data ) {
					::std::vector<wxPoint> points;
					points.reserve( m_points.size( ) );
					for( const auto& point : m_points ) {
						points.emplace_back( point.mapped_point( coord_data ) );
					}
					dc.DrawPolygon( points.size( ), daw::algorithm::to_array( points ) );
				}
			private:
				const ::std::vector<point_t> m_points;
			};
		}	// namespace impl

		PanelGenericPlotter::PanelGenericPlotter( ): m_coord_data( ), m_actions( ) { }

		translation_t& PanelGenericPlotter::coord_data( ) {
			return m_coord_data;
		}

		const translation_t& PanelGenericPlotter::coord_data( ) const {
			return m_coord_data;
		}

		void PanelGenericPlotter::set_pen( wxPen pen ) {
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionPen( ::std::move( pen ) ) ) );
		}

		void PanelGenericPlotter::set_font( wxFont font ) {
			m_last_font = font;
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionFont( ::std::move( font ) ) ) );
		}

		void PanelGenericPlotter::set_brush( wxBrush brush ) {
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionBrush( ::std::move( brush ) ) ) );
		}

		point_t PanelGenericPlotter::get_text_size( const wxString& text ) const {
			point_t bounds;
			wxMemoryDC dc;
			wxCoord zero{ 0 };
			dc.GetTextExtent( text, &bounds.pos( ).x, &bounds.pos( ).y, &zero, &zero, &m_last_font );
			return ::std::move( bounds );
		}

		box_t PanelGenericPlotter::get_rotated_text_size( const wxString& text, double angle ) const {
			point_t point2;
			wxMemoryDC dc;
			wxCoord zero{ 0 };
			dc.GetTextExtent( text, &point2.pos( ).x, &point2.pos( ).y, &zero, &zero, &m_last_font );
			return rotate_by( box_t{ point2 }, angle );
		}

		void PanelGenericPlotter::draw_text( wxString text, point_t point ) {
			//check_minmax( { point, get_text_size( text ) + point } );			
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionDrawText( ::std::move( text ), { ::std::move( point ) } ) ) );
		}

		void PanelGenericPlotter::draw_rotated_text( wxString text, point_t point, double angle ) {
			//check_minmax( get_rotated_text_size( text, angle ) );
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionDrawRotatedText( ::std::move( text ), ::std::move( point ), ::std::move( angle ) ) ) );
		}

		void PanelGenericPlotter::draw_line( point_t p1, point_t p2 ) {
			check_minmax( { p1, p2 } );
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionDrawLine( ::std::move( p1 ), ::std::move( p2 ) ) ) );
		}

		void PanelGenericPlotter::draw_lines( ::std::vector<point_t> points ) {
			daw::exception::dbg_throw_on_false( 2 <= points.size( ), ": Must specify at least two points" );
			for( const auto& point : points ) {
				check_minmax( point );
			}
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionDrawLines( ::std::move( points ) ) ) );
		}

		void PanelGenericPlotter::draw_polygon( ::std::vector<point_t> points ) {
			for( const auto& point : points ) {
				check_minmax( point );
			}
			m_actions.emplace_back( ::std::unique_ptr<impl::PanelGenericPlotAction>( new impl::PanelGenericPlotActionDrawPolygon( ::std::move( points ) ) ) );
		}

		void PanelGenericPlotter::check_minmax( point_t pt ) {
			const auto& point = pt.pos( );
			auto& point1 = m_coord_data.item_bounds.point1.pos( );
			auto& point2 = m_coord_data.item_bounds.point2.pos( );
			if( point.x < point1.x ) {
				point1.x = point.x;
			}
			if( point.x > point2.x ) {
				point2.x = point.x;
			}
			if( point.y < point1.y ) {
				point1.y = point.y;
			}
			if( point.y > point2.y ) {
				point2.y = point.y;
			}
		}

		void PanelGenericPlotter::check_minmax( box_t points ) {
			check_minmax( points.point1 );
			check_minmax( points.point2 );
		}

		void PanelGenericPlotter::update_scale( const wxSize& bounds ) {
			m_coord_data.panel_bounds = bounds;
			m_coord_data.scale.x = static_cast<float>(bounds.GetWidth( ) - coord_data( ).margins.width( )) / static_cast<float>(m_coord_data.item_bounds.width( ));
			m_coord_data.scale.y = static_cast<float>(bounds.GetHeight( ) - coord_data( ).margins.height( )) / static_cast<float>(m_coord_data.item_bounds.height( ));
		}

		void PanelGenericPlotter::plot( wxDC& dc, wxSize bounds ) {
			dc.SetAxisOrientation( true, false );
			dc.SetLogicalOrigin( 0, 0 );
			dc.SetMapMode( wxMM_TEXT );
			dc.SetBackgroundMode( wxTRANSPARENT );
			update_scale( bounds );
			for( auto& action : m_actions ) {
				action->do_plot( dc, m_coord_data );
			}
			dc.SetPen( wxNullPen );
			dc.SetBrush( wxNullBrush );
			dc.SetFont( wxNullFont );
		}

		void PanelGenericPlotter::clear( ) {
			m_actions.clear( );
		}

		void draw_mmol_y_axis( PanelGenericPlotter& gen_plot, graph_config_t graph_config, float at_least_y_values ) {
			// Y-Axis
			const auto& min_point( graph_config.coord_data.item_bounds.point1 );
			const point_t max_point{ daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).x, 100 ), daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).y, static_cast<int>(at_least_y_values*10.0) ) };
			const auto min_y( static_cast<int>(daw::math::floor_by( min_point.pos( ).y - 10, 10.0 )) );
			const auto max_y( static_cast<int>(daw::math::ceil_by( max_point.pos( ).y + 10, 10.0 )) );
			const auto& min_x = min_point.pos( ).x;
			const auto& max_x = max_point.pos( ).x;
			
			gen_plot.set_pen( graph_config.pen_axis_y );
			gen_plot.draw_line( point_t( min_x, min_y ), point_t( min_x, max_y ) );	// Y-axis
			{
				for( auto cur_y = min_y + 5; cur_y <= max_y; cur_y += 5 ) {
					wxString cur_label( std::to_string( static_cast<daw::data::real_t>(cur_y) / 10.0 ) );
					if( 0 == cur_y % 10 ) {
						gen_plot.set_font( graph_config.fnt_axis_title_bold );
						gen_plot.set_pen( graph_config.pen_axis_dotted );
						gen_plot.draw_line( point_t( min_x, cur_y ), point_t( max_x, cur_y ) );
						cur_label += ".0";		//Hack, too lazy to use other padding.  It works, leave me alone
					} else {
						gen_plot.set_font( graph_config.fnt_axis_title );
					}
					gen_plot.set_pen( graph_config.pen_axis_y );
					const point_t p_left( min_x, cur_y, -2 );
					const point_t p_right( min_x, cur_y, 2 );
					gen_plot.draw_line( p_left, p_right );

					if( cur_y >= max_y ) {
						if( graph_config.axis_title_y.empty( ) ) {
							cur_label += " mmol/L";
						} else {
							cur_label += " " + graph_config.axis_title_y;
						}
					}
					const point_t p_text( min_x, cur_y, 4, gen_plot.get_text_size( cur_label ).pos( ).y / 2 );
					gen_plot.draw_text( cur_label, p_text );
				}
			}
		}

		void draw_24hr_x_axis( PanelGenericPlotter& gen_plot, int increment_size, graph_config_t graph_config, float at_least_y_values ) {
//			increment_size;
			const auto& min_point( graph_config.coord_data.item_bounds.point1 );
			const point_t max_point{ daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).x, 100 ), daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).y, static_cast<int>(at_least_y_values*10.0) ) };
			const auto min_y( static_cast<int>(daw::math::floor_by( min_point.pos( ).y - 10, 10.0 )) );
			const auto max_y( static_cast<int>(daw::math::ceil_by( max_point.pos( ).y + 10, 10.0 )) );
			const auto& min_x = min_point.pos( ).x;
			const auto& max_x = max_point.pos( ).x;

			gen_plot.set_pen( graph_config.pen_axis_y );
			gen_plot.draw_line( point_t( min_x, min_y ), point_t( max_x, min_y ) );	// X-axis
			
			if( !graph_config.axis_title_x.empty( ) ) {
				const auto off = gen_plot.get_text_size( graph_config.axis_title_x ).pos( );
				gen_plot.coord_data( ).margins.right += off.x;
				gen_plot.draw_text( wxString( graph_config.axis_title_x ), point_t( max_x, min_y, 2, off.y / 2 ) );
			}

			const auto total_increments = 24 * (60 / increment_size);
			for( auto n = 1; n < total_increments; ++n ) {
				const int x = n * increment_size;
				const point_t p_mid( x, min_y );
				const point_t p_low( x, min_y, 0, -2 );
				const point_t p_high( x, min_y, 0, 2 );

				gen_plot.set_pen( graph_config.pen_axis_y );
				gen_plot.draw_line( p_low, p_high );

				boost::posix_time::ptime ts( boost::posix_time::second_clock::local_time( ).date( ) );
				ts += boost::posix_time::minutes( x );

				if( 0 == ts.time_of_day( ).minutes( ) ) {
					gen_plot.set_pen( graph_config.pen_axis_dotted );
					const point_t p_high_dot( x, max_y );
					gen_plot.draw_line( p_high, p_high_dot );
					gen_plot.set_font( graph_config.fnt_axis_title_bold );
				} else {
					gen_plot.set_font( graph_config.fnt_axis_title );
				}
				const ::std::string cur_label( daw::string::ptime_to_string( ts, "%H:%M" ) );
				const auto y_off = gen_plot.get_text_size( cur_label ).pos( ).y;
				const point_t p_text( x, min_y, 0 - (y_off / 2), 6 + y_off / 2 );

				gen_plot.draw_rotated_text( wxString( cur_label ), p_text, 45.0 );
			}
		}

		void draw_ts_x_axis( PanelGenericPlotter& gen_plot, const daw::data::DataTable::value_type& ts_col, size_t start, size_t finish, graph_config_t graph_config, float at_least_y_values ) {
			const auto& min_point( graph_config.coord_data.item_bounds.point1 );
			const point_t max_point{ daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).x, 100 ), daw::math::value_or_min( graph_config.coord_data.item_bounds.point2.pos( ).y, static_cast<int>(at_least_y_values*10.0) ) };
			const auto min_y( static_cast<int>(daw::math::floor_by( min_point.pos( ).y - 10, 10.0 )) );
			const auto max_y( static_cast<int>(daw::math::ceil_by( max_point.pos( ).y + 10, 10.0 )) );
			const auto& min_x = min_point.pos( ).x;
			const auto& max_x = max_point.pos( ).x;


			gen_plot.set_pen( graph_config.pen_axis_x );
			gen_plot.draw_line( point_t( min_x, min_y ), point_t( max_x, min_y ) );	// X-axis
			if( !graph_config.axis_title_x.empty( ) ) {
				const auto off = gen_plot.get_text_size( graph_config.axis_title_x ).pos( );
				gen_plot.coord_data( ).margins.right += off.x;
				gen_plot.draw_text( wxString( graph_config.axis_title_x ), point_t( max_x, min_y, 2, off.y / 2 ) );
			}

			{
				gen_plot.set_font( graph_config.fnt_axis_title );
				bool is_first = true;
				for( size_t n = start; n <= finish; ++n ) {
					const auto& ts( ts_col[n].timestamp( ) );
					const auto x( (ts - s_epoch).total_seconds( ) / 60 );
					::std::string cur_label;

					if( 0 == ts.time_of_day( ).hours( ) && 0 == ts.time_of_day( ).minutes( ) ) {
						is_first = false;
						gen_plot.set_font( graph_config.fnt_axis_title_bold );
						cur_label = daw::string::ptime_to_string( ts, "%H:%M %b %d" );
					}
					if( 0 == x % 60 ) {						
						gen_plot.set_pen( graph_config.pen_axis_dotted );
						gen_plot.draw_line( point_t( x, min_y ), point_t( x, max_y ) );
						gen_plot.set_pen( graph_config.pen_axis_x );
						gen_plot.draw_line( point_t( x, min_y, 0, -2 ), point_t( x, min_y, 0, 2 ) );
						gen_plot.set_font( graph_config.fnt_axis_title_bold );
						if( 0 == cur_label.size( ) ) {
							if( is_first ) {
								is_first = false;
								cur_label = daw::string::ptime_to_string( ts, "%H:%M %b %d" );
							} else {
								cur_label = daw::string::ptime_to_string( ts, "%H:%M" );
							}
						}
					} else if( 30 == x % 60 ) {
						gen_plot.set_pen( graph_config.pen_axis_x );
						gen_plot.draw_line( point_t( x, min_y, 0, -2 ), point_t( x, min_y, 0, 2 ) );
						gen_plot.set_font( graph_config.fnt_axis_title );
						if( is_first ) {
							is_first = false;
							cur_label = daw::string::ptime_to_string( ts, "%H:%M %b %d" );
						} else {
							cur_label = daw::string::ptime_to_string( ts, "%H:%M" );
						}
					}

					if( 0 != cur_label.size( ) ) {
						const auto y_off = gen_plot.get_text_size( cur_label ).pos( ).y;
						const point_t p_text( x, min_y, 0 - (y_off / 2), 6 + y_off / 2 );
						gen_plot.draw_rotated_text( wxString( cur_label ), p_text, 45.0 );
					}
				}
			}
		}

		box_t::box_t( ):
				point1{ 0, 0 },
				point2{ 0, 0 } { }

		box_t::box_t( point_t point2 ):
				point1{ 0, 0 },
				point2{ ::std::move( point2 ) } { }

		box_t::box_t( point_t p1, point_t p2 ):
				point1{ ::std::move( p1 ) },
				point2{ ::std::move( p2 ) } { }

		int box_t::width( ) const {
			return abs( point2.pos( ).x - point1.pos( ).x );
		}

		int box_t::height( ) const {
			return abs( point2.pos( ).y - point1.pos( ).y );
		}

		translation_t::translation_t( ):
				scale{ },
				item_bounds{ point_t{ ::std::numeric_limits<int>::max( ), ::std::numeric_limits<int>::max( ) }, point_t{ ::std::numeric_limits<int>::min( ), ::std::numeric_limits<int>::min( ) } },
				panel_bounds{ 0, 0 },
				margins{ } { }

		translation_t::scale_t::scale_t( ):
				x{ 0 },
				y{ 0 } { }

		translation_t::margin_t::margin_t( ):
				top{ 0 },
				bottom{ 0 },
				left{ 0 },
				right{ 0 } { }

		void translation_t::margin_t::set_all( size_t sz ) {
			top = sz;
			bottom = sz;
			left = sz;
			right = sz;
		}

		size_t translation_t::margin_t::width( ) const {
			return left + right;
		}

		size_t translation_t::margin_t::height( ) const {
			return top + bottom;
		}
	}	// namespace pumpdataanalysis
}	// namespace daw
