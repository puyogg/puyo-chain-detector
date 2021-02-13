add_library(browsersource STATIC
    server.cpp
    server.hpp
)

target_include_directories(browsersource
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)


target_link_libraries(browsersource
    PUBLIC ${Boost_LIBRARIES}
    PRIVATE jsoncpp_static
)

