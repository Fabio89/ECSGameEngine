#pragma once

#if defined(_WIN32)

    #if defined(EDITOR_BUILD)
        #define EDITOR_API __declspec(dllexport)
    #else
        #define EDITOR_API __declspec(dllimport)
    #endif

#elif defined(__GNUC__) || defined(__clang__)

    #define EDITOR_API __attribute__((visibility("default")))

#else

    #define EDITOR_API

#endif