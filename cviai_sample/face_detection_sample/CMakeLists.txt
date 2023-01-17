cmake_minimum_required(VERSION 2.8.0)

project(cvitek_samples C CXX)

add_subdirectory(face_detection)


install(FILES ${SCRIPT_FILES}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
    DESTINATION .)

