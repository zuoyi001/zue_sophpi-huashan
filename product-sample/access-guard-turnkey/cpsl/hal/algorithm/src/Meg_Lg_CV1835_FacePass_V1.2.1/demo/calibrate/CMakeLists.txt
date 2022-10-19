
set(OPENCV_EXTRA_LIBS "opencv_calib3d -lopencv_features2d -lopencv_flann")

add_executable( calibrate calibrate.cpp)
target_link_libraries( calibrate sample_common ${OPENCV_EXTRA_LIBS})
