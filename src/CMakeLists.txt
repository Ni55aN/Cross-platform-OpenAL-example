
if(EMSCRIPTEN) #################################################################


  set(flags "--embed-file ../../../sounds -Wall -s FORCE_FILESYSTEM=1")

    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${flags})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${flags})


    add_executable (openal_example main.cc)

elseif(ANDROID)

    include_directories("/root/Android/Sdk/ndk-bundle/platforms/android-21/arch-x86_64/usr/include" "/home/ni55an/libs/openal-soft/include")

    add_library (openal_example SHARED main.cc)

    target_link_libraries(openal_example
        /home/ni55an/libs/openal-soft/build-android/libopenal.so
        android
    )



else()

    add_executable (openal_example main.cc)

    target_link_libraries(openal_example openal alut)


endif()