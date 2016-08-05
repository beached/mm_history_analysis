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

#include <wx/log.h>
#include <wx/wx.h>

#include <daw/csv_helper/data_common.h>
#include <daw/daw_exception.h>

#include "csv_table.h"
#include "string_helpers.h"

namespace daw {
	namespace data {
		CSVTable::CSVTable( ):
				wxGridTableBase{ },
				m_data_analysis( nullptr ),
				m_valid( false ) { }

		CSVTable::CSVTable( daw::data::parse_csv_data_param param ) :
				wxGridTableBase{ },
				m_data_analysis( new daw::pumpdataanalysis::PumpDataAnalysis( ::std::move( param ), ::std::bind( &CSVTable::set_valid, this, true ) ) ),
				m_valid( false ) { }

		daw::data::DataTable const & CSVTable::data( ) const {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return m_data_analysis->data_table( );
		}

		daw::pumpdataanalysis::PumpDataAnalysis & CSVTable::data_analysis( ) {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return *m_data_analysis;
		}

		daw::pumpdataanalysis::PumpDataAnalysis const & CSVTable::data_analysis( ) const {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return *m_data_analysis;
		}

		int CSVTable::GetNumberRows( ) {
			if( this->data().empty( ) ) {
				return 0;
			}
			return static_cast<int>(this->data( )[0].size( ));
		}

		int CSVTable::GetNumberCols( ) {
			return static_cast<int>( this->data( ).size( ) );
		}

		bool CSVTable::IsEmptyCell( int row, int col ) {
			return this->data( )[col][row].empty( );
		}

		wxString CSVTable::GetValue( int row, int col ) {
			auto const & cell = this->data( )[col][row];
			if( DataCellType::timestamp == cell.type( ) ) {
				return daw::string::ptime_to_string( cell.timestamp( ), "%Y-%m-%d %H:%M" );
			}
			return cell.to_string( );
		}

		void CSVTable::SetValue( int, int, wxString const & ) {
			throw daw::exception::NotImplemented( ": DataTable is read-only" );
		}

		wxString CSVTable::GetColLabelValue( int col ) {
			auto ret = this->data( )[col].header( );
			return ret;
		}

		void CSVTable::Clear( ) { }

		CSVTable::~CSVTable( ) { }

		CSVTable::CSVTable( CSVTable && other ):
			wxGridTableBase{ },
			m_data_analysis{ std::move( other.m_data_analysis ) },
			m_valid{ other.m_valid } { }


		void swap( CSVTable & lhs, CSVTable & rhs ) noexcept {
			using std::swap;
			swap( lhs.m_data_analysis, rhs.m_data_analysis );
			swap( lhs.m_valid, rhs.m_valid );
		}

		CSVTable & CSVTable::operator=( CSVTable && rhs ) {
			if( this != &rhs ) {
				using std::swap;
				CSVTable tmp{ std::move( rhs ) };
				swap( *this, tmp );
			}
			return *this;
		}

		bool CSVTable::is_valid( ) const {
			return m_valid;
		}

	}
}
