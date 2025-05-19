
cmake_minimum_required ( VERSION 3.10.0 )

project ( RayTracerEngineApp )

add_subdirectory ( "app"        )
add_subdirectory ( "engine"     )
add_subdirectory ( "ray tracer" )

set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app )
