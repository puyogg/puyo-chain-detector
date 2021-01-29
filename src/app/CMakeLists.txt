add_executable(PuyoChainDetector
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
)

target_link_libraries(PuyoChainDetector
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE ${OpenCV_LIBS}
    PRIVATE jsoncpp_static
)
