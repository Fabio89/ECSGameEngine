#pragma once

#if defined(_WIN32)

    #if defined(ENGINE_BUILD)
        #define ENGINE_API __declspec(dllexport)
    #else
        #define ENGINE_API __declspec(dllimport)
    #endif

#elif defined(__GNUC__) || defined(__clang__)

    #define ENGINE_API __attribute__((visibility("default")))

#else

    #define ENGINE_API

#endif