#pragma once
#include "gb/types.hpp"

#include <string>
#include <array>
#include <cstdint>

namespace gb {
	enum class MBCType {
		MBC0, MBC1,
		// ...
	};

	class Cartridge {
		public:
			u8 read8(u16 addr);
			void write8(u16 addr, u8 value);

			bool load_cartridge(const std::string &path);
		private:
			std::array<u8, 0x200000> rom_{}; // Max 2MB ROM
			std::size_t rom_size_;
			std::size_t ram_size_;

			bool ram_ena_ = false;       // 0x0000 ~ 0x1FFF
			u8 rom_bank_no_ = 0x00;   // 0x2000 ~ 0x3FFF
			u8 ram_bank_no_ = 0x00;   // 0x4000 ~ 0x5FFF
			bool is_bank_simple_ = true; // 0x6000 ~ 0x7FFF

			MBCType mbc_ = MBCType::MBC0;

	};
} // namespace gb
