// Copyright 2019 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/common_funcs.h"
#include "common/logging/log.h"
#include "common/swap.h"
#include "core/core.h"
#include "core/hle/hooks/function.h"
#include "core/hle/hooks/manager.h"
#include "core/loader/nso_linker.h"
#include "core/memory.h"

namespace Elf64 {

enum class RelocationType : u32 { ABS64 = 257, GLOB_DAT = 1025, JUMP_SLOT = 1026, RELATIVE = 1027 };

enum DynamicType : u32 {
    DT_NULL = 0,
    DT_PLTRELSZ = 2,
    DT_STRTAB = 5,
    DT_SYMTAB = 6,
    DT_RELA = 7,
    DT_RELASZ = 8,
    DT_STRSZ = 10,
    DT_JMPREL = 23,
};

struct Rela {
    u64_le offset;
    RelocationType type;
    u32_le symbol;
    s64_le addend;
};
static_assert(sizeof(Rela) == 0x18, "Rela has incorrect size.");

struct Dyn {
    u64_le tag;
    u64_le value;
};
static_assert(sizeof(Dyn) == 0x10, "Dyn has incorrect size.");

struct Sym {
    u32_le name;
    INSERT_PADDING_BYTES(0x2);
    u16_le shndx;
    u64_le value;
    u64_le size;
};
static_assert(sizeof(Sym) == 0x18, "Sym has incorrect size.");

} // namespace Elf64

namespace Loader::NSO {

struct Symbol {
    Symbol(std::string&& name, u64 value) : name(std::move(name)), value(value) {}
    std::string name;
    u64 value;
};

static void FindExternals(std::vector<u8>& program_image, const std::vector<Symbol>& symbols,
                          u64 relocation_offset, u64 size, VAddr load_base,
                          Hooks::Manager& hooks_manager) {
    for (u64 i = 0; i < size; i += sizeof(Elf64::Rela)) {
        Elf64::Rela rela;
        std::memcpy(&rela, &program_image[relocation_offset + i], sizeof(Elf64::Rela));

        const Symbol& symbol = symbols[rela.symbol];

        switch (rela.type) {
        case Elf64::RelocationType::JUMP_SLOT:
        case Elf64::RelocationType::GLOB_DAT:
            if (!symbol.value) {
                hooks_manager.RegisterImport({rela.offset + load_base, {symbol.name}});
            }
            break;
        case Elf64::RelocationType::RELATIVE:
        case Elf64::RelocationType::ABS64:
            break;
        default:
            LOG_CRITICAL(Loader, "Unknown relocation type: {}", static_cast<int>(rela.type));
            break;
        }
    }
}

void ParseDynamicSection(std::vector<u8>& program_image, u32 dynamic_section_offset,
                         VAddr load_base, Hooks::Manager& hooks_manager) {
    std::map<u64, u64> dynamic;
    while (dynamic_section_offset < program_image.size()) {
        Elf64::Dyn dyn;
        std::memcpy(&dyn, &program_image[dynamic_section_offset], sizeof(Elf64::Dyn));
        dynamic_section_offset += sizeof(Elf64::Dyn);

        if (dyn.tag == Elf64::DT_NULL) {
            break;
        }
        dynamic[dyn.tag] = dyn.value;
    }

    u64 offset = dynamic[Elf64::DT_SYMTAB];
    std::vector<Symbol> symbols;
    while (offset < program_image.size()) {
        Elf64::Sym sym;
        std::memcpy(&sym, &program_image[offset], sizeof(Elf64::Sym));
        offset += sizeof(Elf64::Sym);

        if (sym.name >= dynamic[Elf64::DT_STRSZ]) {
            break;
        }

        std::string name =
            reinterpret_cast<char*>(&program_image[dynamic[Elf64::DT_STRTAB] + sym.name]);
        if (sym.value) {
            symbols.emplace_back(std::move(name), load_base + sym.value);
        } else {
            symbols.emplace_back(std::move(name), 0);
        }
    }

    if (dynamic.find(Elf64::DT_RELA) != dynamic.end()) {
        FindExternals(program_image, symbols, dynamic[Elf64::DT_RELA], dynamic[Elf64::DT_RELASZ],
                      load_base, hooks_manager);
    }

    if (dynamic.find(Elf64::DT_JMPREL) != dynamic.end()) {
        FindExternals(program_image, symbols, dynamic[Elf64::DT_JMPREL],
                      dynamic[Elf64::DT_PLTRELSZ], load_base, hooks_manager);
    }
}

} // namespace Loader::NSO
