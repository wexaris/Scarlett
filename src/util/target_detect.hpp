#pragma once
#include <string>

// Windows
#ifdef _MSC_VER
	#if defined(_WIN64)
        #define DETECT_TARGET "x86_64-windows-msvc";
	#else
        #define DETECT_TARGET "x86-windows-msvc"
	#endif
// Linux
#elif defined(__linux__)
	#if defined(__amd64__)
        #define DETECT_TARGET "x86_64-linux-gnu"
	#elif defined(__aarch64__)
        #define DETECT_TARGET "aarch64-linux-gnu"
	#elif defined(__arm__)
        #define DETECT_TARGET "arm-linux-gnu"
	#elif defined(__i386__)
        #define DETECT_TARGET "i586-linux-gnu"
	#elif defined(__m68k__)
        #define DETECT_TARGET "m68k-linux-gnu"
	#else
		#warning "Unable to detect a suitable default target (linux-gnu)"
	#endif
// MinGW
#elif defined(__MINGW32__)
	#if defined(_WIN64)
        #define DETECT_TARGET "x86_64-windows-gnu"
	#else
        #define DETECT_TARGET "i586-windows-gnu"
	#endif
// FreeBSD
#elif defined(__FreeBSD__)
	#if defined(__amd64__)
        #define DETECT_TARGET "x86_64-unknown-freebsd"
	#elif defined(__aarch64__)
        #define DETECT_TARGET "aarch64-unknown-freebsd"
	#elif defined(__arm__)
        #define DETECT_TARGET "arm-unknown-freebsd"
	#elif defined(__i386__)
        #define DETECT_TARGET "i686-unknown-freebsd"
	#else
		#warning "Unable to detect a suitable default target (FreeBSD)"
	#endif
// NetBSD
#elif defined(__NetBSD__)
	#if defined(__amd64__)
        #define DETECT_TARGET "x86_64-unknown-netbsd"
	#else
		#warning "Unable to detect a suitable default target (NetBSD)"
	#endif
// OpenBSD
#elif defined(__OpenBSD__)
	#if defined(__amd64__)
        #define DETECT_TARGET "x86_64-unknown-openbsd"
	#elif defined(__aarch64__)
        #define DETECT_TARGET "aarch64-unknown-openbsd"
	#elif defined(__arm__)
        #define DETECT_TARGET "arm-unknown-openbsd"
	#elif defined(__i386__)
        #define DETECT_TARGET "i686-unknown-openbsd"
	#else
		#warning "Unable to detect a suitable default target (OpenBSD)"
	#endif
// DragonFly
#elif defined(__DragonFly__)
    #define DETECT_TARGET "x86_64-unknown-dragonfly"
// Apple devices
#elif defined(__APPLE__)
    #define DETECT_TARGET "x86_64-apple-macosx"
// Haiku
#elif defined(__HAIKU__)
	#if defined(__x86_64__)
        #define DETECT_TARGET "x86_64-unknown-haiku"
	#elif defined(__arm__)
        #define DETECT_TARGET "arm-unknown-haiku"
	#else
		#warning "Unable to detect a suitable default target (Haiku)"
	#endif
// Unknown
#else
    #warning "Unable to detect a suitable default target"
#endif

#ifndef DETECT_TARGET
    #define DETECT_TARGET "";
#endif