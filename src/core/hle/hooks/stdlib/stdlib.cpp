// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included

#include <cmath>
#include <cstring>
#include "core/hle/hooks/manager.h"
#include "core/hle/hooks/stdlib/stdlib.h"
#include "core/memory.h"

namespace Hooks::stdlib {

static u64 strcpy(u64 dest, u64 src) {
    std::strcpy(reinterpret_cast<char*>(Memory::GetPointer(dest)),
                reinterpret_cast<char*>(Memory::GetPointer(src)));
    return dest;
}

static u64 strncpy(u64 dest, u64 src, std::size_t count) {
    std::strncpy(reinterpret_cast<char*>(Memory::GetPointer(dest)),
                 reinterpret_cast<char*>(Memory::GetPointer(src)), count);
    return dest;
}

static u64 strcat(u64 dest, u64 src) {
    std::strcat(reinterpret_cast<char*>(Memory::GetPointer(dest)),
                reinterpret_cast<char*>(Memory::GetPointer(src)));
    return dest;
}

static u64 strncat(u64 dest, u64 src, std::size_t count) {
    std::strncat(reinterpret_cast<char*>(Memory::GetPointer(dest)),
                 reinterpret_cast<char*>(Memory::GetPointer(src)), count);
    return dest;
}

static u64 strxfrm(u64 dest, u64 src, std::size_t count) {
    return std::strxfrm(reinterpret_cast<char*>(Memory::GetPointer(dest)),
                        reinterpret_cast<char*>(Memory::GetPointer(src)), count);
}

/////////////////////////////////////////////

static std::size_t strlen(u64 str) {
    return std::strlen(reinterpret_cast<char*>(Memory::GetPointer(str)));
}

static int strcmp(u64 lhs, u64 rhs) {
    return std::strcmp(reinterpret_cast<char*>(Memory::GetPointer(lhs)),
                       reinterpret_cast<char*>(Memory::GetPointer(rhs)));
}

static int strncmp(u64 lhs, u64 rhs, size_t count) {
    return std::strncmp(reinterpret_cast<char*>(Memory::GetPointer(lhs)),
                        reinterpret_cast<char*>(Memory::GetPointer(rhs)), count);
}

static int strcoll(u64 lhs, u64 rhs) {
    return std::strcoll(reinterpret_cast<char*>(Memory::GetPointer(lhs)),
                        reinterpret_cast<char*>(Memory::GetPointer(rhs)));
}

static u64 strchr(u64 str, int ch) {
    // const char* src_ptr = reinterpret_cast<char*>(Memory::GetPointer(str));
    // const char* res = std::strchr(src_ptr, ch);

    // if (!res) {
    //    return {};
    //}

    // return str + (res - src_ptr);

    return {};
}

/////////////////////////////////////////////
static u64 memchr(u64 dest, int ch, std::size_t count) {
    std::memchr(Memory::GetPointer(dest), ch, count);
    return dest;
}

static int memcmp(u64 lhs, u64 rhs, std::size_t count) {
    return std::memcmp(Memory::GetPointer(lhs), Memory::GetPointer(rhs), count);
}

static u64 memset(u64 dest, int ch, std::size_t count) {
    std::memset(Memory::GetPointer(dest), ch, count);
    return dest;
}

static u64 memcpy(u64 dest, u64 src, std::size_t count) {
    std::memcpy(Memory::GetPointer(dest), Memory::GetPointer(src), count);
    return dest;
}

static u64 memmove(u64 dest, u64 src, std::size_t count) {
    std::memmove(Memory::GetPointer(dest), Memory::GetPointer(src), count);
    return dest;
}

void RegisterFunctions(Manager& manager) {
    // String manipulation
    manager.RegisterFunction({"strcpy"}, Hooks::MakeFunction(&strcpy));
    manager.RegisterFunction({"strncpy"}, Hooks::MakeFunction(&strncpy));
    manager.RegisterFunction({"strcat"}, Hooks::MakeFunction(&strcat));
    manager.RegisterFunction({"strncat"}, Hooks::MakeFunction(&strncat));
    manager.RegisterFunction({"strxfrm"}, Hooks::MakeFunction(&strxfrm));

    // String examination
    manager.RegisterFunction({"strlen"}, Hooks::MakeFunction(&strlen));
    manager.RegisterFunction({"strcmp"}, Hooks::MakeFunction(&strcmp));
    manager.RegisterFunction({"strncmp"}, Hooks::MakeFunction(&strncmp));
    manager.RegisterFunction({"strcoll"}, Hooks::MakeFunction(&strcoll));
    manager.RegisterFunction({"strchr"}, Hooks::MakeFunction(&strchr));

    // Character array manipulation
    manager.RegisterFunction({"memchr"}, Hooks::MakeFunction(&memchr));
    manager.RegisterFunction({"memcmp"}, Hooks::MakeFunction(&memcmp));
    manager.RegisterFunction({"memset"}, Hooks::MakeFunction(&memset));
    manager.RegisterFunction({"memcpy"}, Hooks::MakeFunction(&memcpy));
    manager.RegisterFunction({"memmove"}, Hooks::MakeFunction(&memmove));
}

} // namespace Hooks::stdlib
