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

#include <memory>
#include <wx/wx.h>

// menu items ids
enum {
	MDI_REFRESH = 101,
	MDI_CHANGE_POSITION,
	MDI_CHANGE_SIZE
};

// Define a new frame
class PanelPumpDataAnalyis;

class FramePumpDataAnalysis: public wxMDIParentFrame {
public:
	FramePumpDataAnalysis( wxApp* app );
	static wxMenuBar *create_menu_bar( );

private:
	void on_size( wxSizeEvent& event );
	void on_about( wxCommandEvent& event );
	void on_file_open( wxCommandEvent& event );
	void on_fullscreen( wxCommandEvent& event );
	void on_quit( wxCommandEvent& event );
	void on_close_all( wxCommandEvent& event );

	void on_close( wxCloseEvent& event );

	wxApp* m_wxapp;

	DECLARE_EVENT_TABLE( )
};

