add_library(detector STATIC
    Detector.cpp Detector.hpp
    Constants.hpp
    Settings.hpp
    PlayerTracker.cpp PlayerTracker.hpp
    NextTracker.cpp NextTracker.hpp
    ScoreTracker.cpp ScoreTracker.hpp
)

target_link_libraries(detector
    PUBLIC chainsim
    PRIVATE jsoncpp_static
    PUBLIC ${OpenCV_LIBS}
    PUBLIC ${Boost_LIBRARIES}
    PRIVATE browsersource
)

get_target_property(CHAINSIM_SRC_DIR chainsim SOURCE_DIR)
target_include_directories(detector
    PUBLIC ${PROJECT_SOURCE_DIR}/src
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC ${OpenCV_INCLUDE_DIRS}
)

