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
#include <wx/grid.h>

#include <daw/csv_helper/data_table.h>

#include "pump_data_analysis.h"

namespace daw {
	namespace data {
		/// Provides a link to <c>daw::data::DataTable</c> from wxWidgets
		class CSVTable;
		void swap( CSVTable & lhs, CSVTable & rhs ) noexcept;

		class CSVTable final: public wxGridTableBase {
			inline void set_valid( bool valid ) {
				m_valid = valid;
			}
			std::shared_ptr<daw::pumpdataanalysis::PumpDataAnalysis> m_data_analysis;
			bool m_valid;

		public:
			CSVTable( );
			/// <summary>Construct from a CSV Text File</summary>
			explicit CSVTable( daw::data::parse_csv_data_param param );

			CSVTable( CSVTable const & ) = default;

			CSVTable & operator=( CSVTable const & ) = default;

			friend void swap( CSVTable & lhs, CSVTable & rhs ) noexcept;
			CSVTable( CSVTable && other );
			CSVTable & operator=( CSVTable && rhs );

			~CSVTable( );

			daw::data::DataTable const & data( ) const;

			daw::pumpdataanalysis::PumpDataAnalysis & data_analysis( );
			daw::pumpdataanalysis::PumpDataAnalysis const & data_analysis( ) const;

			int GetNumberRows( ) override ;
			int GetNumberCols( ) override ;
			bool IsEmptyCell( int row, int col ) override ;
			wxString GetValue( int row, int col ) override ;
			void SetValue( int row, int col, wxString const & value ) override;
			wxString GetColLabelValue( int col ) override;
			void Clear( ) override;

			bool is_valid( ) const;

		};


	}	// namespace data
}	// namespace daw

