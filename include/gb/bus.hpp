#include "gb/types.hpp"

#include <string>
#include <array>
#include <cstdint>

namespace gb {
	class Timer;

	class Bus {
		public:
			explicit Bus(Timer &timer);

			u8 read8(u16 addr) const;
			void write8(u16 addr, u8 value);

			void tick(int cycles);

			bool load_bootrom(const std::string &path);
			void set_bootrom_enabled(bool flag) { bootrom_enabled = flag; }
			bool get_bootrom_enabled() { return bootrom_enabled; }

			bool load_cartridge(const std::string &path);
		private:
			Timer &timer_;
			bool bootrom_enabled = false;
			std::array<u8, 0x100> bootrom_{};    // 0x0000 ~ 0x00FF
			std::array<u8, 0x8000> cartridge_{}; // 0x0000 ~ 0x7FFF
			std::array<u8, 0x2000> vram_{};      // 0x8000 ~ 0x9FFF
			std::array<u8, 0x2000> wram_{};      // 0xC000 ~ 0xDFFF
			std::array<u8, 0x80> ioregs_{};      // 0xFF00 ~ 0xFF7F
			std::array<u8, 0x7F> hram_{};        // 0xFF80 ~ 0xFFFE
			u8 intr_reg = 0;												 // 0xFFFF
	};
} // gb


