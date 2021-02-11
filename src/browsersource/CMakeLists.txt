add_library(browsersource STATIC
    server.cpp
    server.hpp
    overlay.html
)

target_include_directories(browsersource
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)


target_link_libraries(browsersource
    PUBLIC ${Boost_LIBRARIES}
)

add_custom_command(TARGET browsersource POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/overlay.html" "$<TARGET_FILE_DIR:browsersource>"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Overlay BrowserSource" FILES "overlay.html")