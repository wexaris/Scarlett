#pragma once

class Interner;

namespace global {

    extern thread_local Interner* interner;

    extern void free();
}