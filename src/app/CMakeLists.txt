add_executable(PuyoChainDetector
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    detectionsettings.json
    settings.json
)

target_link_libraries(PuyoChainDetector
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE ${OpenCV_LIBS}
    PRIVATE jsoncpp_static
    PRIVATE chainsim
    PRIVATE detector
)

# Custom command to copy over detectionsettings.json to build folder
add_custom_command(TARGET PuyoChainDetector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/detectionsettings.json" "$<TARGET_FILE_DIR:PuyoChainDetector>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/settings.json" "$<TARGET_FILE_DIR:PuyoChainDetector>"
)

set(JSON_FILES
    "detectionsettings.json"
    "settings.json"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "JSON Default Settings" FILES ${JSON_FILES})
