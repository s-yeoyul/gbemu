#include "gb/types.hpp"

#include <string>
#include <array>
#include <cstdint>

namespace gb {
	class Bus {
		public:
			u8 read8(u16 addr) const;
			void write8(u16 addr, u8 value);

			bool load_bootrom(const std::string &path);
			void set_bootrom_enabled(bool flag) { bootrom_enabled = flag; }
			bool get_bootrom_enabled() { return bootrom_enabled; }
	private:
		bool bootrom_enabled = false;
		std::array<u8, 0x100> bootrom_{};
		std::array<u8, 0x2000> wram_{};
		std::array<u8, 0x7F> hram_{};
	};
} // gb


