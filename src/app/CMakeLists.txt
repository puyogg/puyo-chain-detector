qt_add_executable(PuyoChainDetector WIN32
    main.cpp
    mainwindow.cpp
    threading.hpp
    mainwindow.hpp
    mainwindow.ui
    settings.json
    overlay.html
)

target_link_libraries(PuyoChainDetector
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE ${OpenCV_LIBS}
    PRIVATE jsoncpp_static
    PRIVATE chainsim
    PUBLIC detector
)

# Custom command to copy over detectionsettings.json to build folder
add_custom_command(TARGET PuyoChainDetector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/puyo-mlp-gpu.onnx" "$<TARGET_FILE_DIR:PuyoChainDetector>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/settings.json" "$<TARGET_FILE_DIR:PuyoChainDetector>"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/overlay.html" "$<TARGET_FILE_DIR:PuyoChainDetector>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/overlay_images" "$<TARGET_FILE_DIR:PuyoChainDetector>/overlay_images"
)

set(FILE_DEPENDENCIES
    "settings.json"
    "puyo-mlp-gpu.onnx"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "File Dependencies" FILES ${FILE_DEPENDENCIES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Browser Overlay" FILES overlay.html)
