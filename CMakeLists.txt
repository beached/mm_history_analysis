cmake_minimum_required( VERSION 2.8.12 )
project( mm_history_analysis )

include( ExternalProject )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )

find_package( wxWidgets REQUIRED )

set( Boost_USE_STATIC_LIBS OFF ) 
set( Boost_USE_MULTITHREADED ON )  
set( Boost_USE_STATIC_RUNTIME OFF ) 

add_definitions( -DwxUSE_GUI )

find_package( Boost 1.58.0 REQUIRED COMPONENTS system filesystem regex unit_test_framework program_options iostreams )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 )
else( )
	find_package( Threads REQUIRED )
	if( THREADS_HAVE_PTHREAD_ARG )
		set( COMPILER_SPECifIC_LIBS, "-lpthread" )
	endif( )

	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -g -stdlib=libc++ -Weverything -Wno-c++98-compat -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)		
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options( -std=c++14 -g -Wall -fopenmp -Wno-deprecated-declarations )
	endif( )

endif( )

externalproject_add( 
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)


externalproject_add( 
	csv_helper_prj
	GIT_REPOSITORY "https://github.com/beached/csv_helper.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/csv_helper"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
include_directories( "./include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${wxWidgets_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )
link_directories( ${wxWidgets_LIBRARY_DIRS} )
include( ${wxWidgets_USE_FILE} )

set( HEADER_FOLDER "include" )

set( HEADER_FILES
	${HEADER_FOLDER}/aggregate_data.h
	${HEADER_FOLDER}/app_pump_data_analysis.h
	${HEADER_FOLDER}/csv_table.h
	${HEADER_FOLDER}/dialog_date_range_chooser.h
	${HEADER_FOLDER}/frame_pump_data_analysis.h
	${HEADER_FOLDER}/future_value.h
	${HEADER_FOLDER}/impl/future_value_impl.h
	${HEADER_FOLDER}/multi_lock.h
	${HEADER_FOLDER}/panel_average_basal_derivative.h
	${HEADER_FOLDER}/panel_average_basal.h
	${HEADER_FOLDER}/panel_data_plot.h
	${HEADER_FOLDER}/panel_generic_plot.h
	${HEADER_FOLDER}/panel_pump_data_analysis.h
	${HEADER_FOLDER}/pump_data_analysis.h
	${HEADER_FOLDER}/string_helpers.h
)

set( SOURCE_FILES
	app_pump_data_analysis.cpp
	csv_table.cpp
	dialog_date_range_chooser.cpp
	frame_pump_data_analysis.cpp
	panel_average_basal.cpp
	panel_average_basal_derivative.cpp
	panel_data_plot.cpp
	panel_generic_plot.cpp
	panel_pump_data_analysis.cpp
	pump_data_analysis.cpp
	string_helpers.cpp
	string_helpers.cpp
)

set( WT_CONNECTOR "wthttp" CACHE STRING "Connector used (wthttp or wtfcgi)" )

add_executable( mm_history_analysis ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( mm_history_analysis header_libraries_prj csv_helper_prj )

target_link_libraries( mm_history_analysis csv_helper ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} )

