cmake_minimum_required(VERSION 3.10)


project(playthrough)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main src/main.cpp)

    find_package(portaudio CONFIG REQUIRED)
    target_link_libraries(main PRIVATE portaudio)

find_package(SFML COMPONENTS system window graphics audio REQUIRED)
target_link_libraries(main PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)

    find_package(SampleRate CONFIG REQUIRED)
    target_link_libraries(main PRIVATE SampleRate::samplerate)
