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
		CSVTable::CSVTable( ):wxGridTableBase{ }, m_data_analysis( nullptr ), m_valid( false ) { }

		CSVTable::CSVTable( daw::data::parse_csv_data_param param ) : wxGridTableBase{ }, m_data_analysis( new daw::pumpdataanalysis::PumpDataAnalysis( ::std::move( param ), ::std::bind( &CSVTable::set_valid, this, true ) ) ), m_valid( false ) { }

		CSVTable::CSVTable( CSVTable&& other ):wxGridTableBase{ }, m_data_analysis( ::std::move( other.m_data_analysis ) ), m_valid( ::std::move( other.m_valid ) ) { }
		CSVTable::CSVTable( const CSVTable& other ) : wxGridTableBase{ }, m_data_analysis( other.m_data_analysis ), m_valid( other.m_valid ) { }

		CSVTable& CSVTable::operator=(const CSVTable& other) {
			if( this != &other ) {
				m_data_analysis = other.m_data_analysis;
				m_valid = other.m_valid;
			}
			return *this;
		}

		CSVTable& CSVTable::operator=(CSVTable&& other) {
			if( this != &other ) {
				m_data_analysis = ::std::move( other.m_data_analysis );
				m_valid = ::std::move( other.m_valid );
			}
			return *this;
		}

		const daw::data::DataTable& CSVTable::data( ) const {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return m_data_analysis->data_table( );
		}

		daw::pumpdataanalysis::PumpDataAnalysis& CSVTable::data_analysis( ) {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return *m_data_analysis;
		}

		const daw::pumpdataanalysis::PumpDataAnalysis& CSVTable::data_analysis( ) const {
			daw::exception::dbg_throw_on_null( m_data_analysis.get( ), ": Attempt to access non-existent data" );
			return *m_data_analysis;
		}

		int CSVTable::GetNumberRows( ) {
			if( 0 == data( ).size( ) ) {
				return 0;
			}
			return data( )[0].size( );
		}

		int CSVTable::GetNumberCols( ) {
			return data( ).size( );
		}

		bool CSVTable::IsEmptyCell( int row, int col ) {
			return data( )[col][row].empty( );
		}

		wxString CSVTable::GetValue( int row, int col ) {
			const auto& cell = data( )[col][row];
			if( DataCellType::timestamp == cell.type( ) ) {
				return daw::string::ptime_to_string( cell.timestamp( ), "%Y-%m-%d %H:%M" );
			}
			return cell.to_string( );
		}

		void CSVTable::SetValue( int, int, const wxString& ) {
			throw daw::exception::NotImplemented( ": DataTable is read-only" );
		}

		wxString CSVTable::GetColLabelValue( int col ) {
			wxString ret = data( )[col].header( );
			return ret;
		}

		void CSVTable::Clear( ) { }
	}
}
