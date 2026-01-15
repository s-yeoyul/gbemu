#include "gb/types.hpp"
#include "gb/joypad.hpp"
#include "SDL2/SDL.h"

#include <array>

namespace gb {
	struct Sprites {
		u8 x, y;
		u8 tile;
		u8 attr;
	};

	class PPU {
		public:
			void initPPU();
			void present();
			bool pump_events(Joypad& joypad);
			void shutdownPPU();
			void renderTestPattern(u32 frame);
			u8 tick(int cycles);
			u8 read8(u16 addr);
			void write8(u16 addr, u8 value);
			
			void oam_search();
			void pixel_transfer();
		private:
			SDL_Renderer* renderer_;
			SDL_Window* window_;
			SDL_Texture* texture_;

			int dot_cycles = 0;
			int mode = 2;
			int sprites_num = 0;

			// RAM
			std::array<u8, 0x2000> vram_{}; // 0x8000 ~ 0x9FFF
			std::array<u8, 0xA0> oam_{};    // 0xFE00 ~ 0xFE9F

			// Registers
			u8 lcdc_ = 0;
			u8 stat_ = 0;
			u8 scy_ = 0; u8 scx_ = 0;
			u8 ly_ = 0;
			u8 lyc_ = 0;
			u8 dma_ = 0;
			u8 bgp_ = 0;
			u8 obp0_ = 0; u8 obp1_ = 0;
			u8 wy_ = 0; u8 wx_ = 0;

			std::array<u8, 160 * 144 * 4> framebuffer_{};
			std::array<Sprites, 10> ly_sprites_{};
	};
} // namespace gb
