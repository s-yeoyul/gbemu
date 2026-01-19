#include "gb/ppu.hpp"
#include "gb/joypad.hpp"
#include "SDL2/SDL.h"

#include <iostream>

namespace gb {
	void PPU::initPPU() {
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
			return;
		}	

		if (SDL_CreateWindowAndRenderer(160, 144, 0, &window_, &renderer_) != 0) {
			std::cerr << "SDL_CreateWindowAndRenderer failed: " << SDL_GetError() << "\n";
			SDL_Quit();
			return;
		}

		SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
		SDL_SetWindowSize(window_, 480, 432);
		SDL_SetWindowResizable(window_, SDL_TRUE);

		// === Texture 생성(핵심) ===
		texture_ = SDL_CreateTexture(
				renderer_,
				SDL_PIXELFORMAT_ABGR8888,      // framebuffer_랑 맞춤
				SDL_TEXTUREACCESS_STREAMING,   // 매 프레임 업데이트할 거라서
				160, 144);

	 if (!texture_) {
			std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
			shutdownPPU();
		}
	}

	void PPU::present() {
		if(!renderer_ || !texture_) return;

		// framebuffer_ → texture_
		const int pitch = 160 * 4; // RGBA8888: 4 bytes per pixel
		if (SDL_UpdateTexture(texture_, nullptr, framebuffer_.data(), pitch) != 0) {
			std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << "\n";
			return;
		}

		SDL_RenderClear(renderer_);
		SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
		SDL_RenderPresent(renderer_);
	}

	bool PPU::pump_events(Joypad& joypad) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) return false;
			if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) return false;

			if (e.type == SDL_WINDOWEVENT) {
				if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
						e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
					joypad.set_a(false);
					joypad.set_b(false);
					joypad.set_start(false);
					joypad.set_select(false);
					joypad.set_left(false);
					joypad.set_right(false);
					joypad.set_up(false);
					joypad.set_down(false);
				}
			}

			SDL_PumpEvents();
			const Uint8* ks = SDL_GetKeyboardState(nullptr);

			joypad.set_a(ks[SDL_SCANCODE_Z] != 0);
			joypad.set_b(ks[SDL_SCANCODE_X] != 0);
			joypad.set_start(ks[SDL_SCANCODE_RETURN] != 0);
			joypad.set_select(ks[SDL_SCANCODE_RSHIFT] != 0);

			joypad.set_left(ks[SDL_SCANCODE_LEFT] != 0);
			joypad.set_right(ks[SDL_SCANCODE_RIGHT] != 0);
			joypad.set_up(ks[SDL_SCANCODE_UP] != 0);
			joypad.set_down(ks[SDL_SCANCODE_DOWN] != 0);
			/*
			if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
				if(e.key.keysym.sym == SDLK_z) joypad.set_a(true);
				if(e.key.keysym.sym == SDLK_x) joypad.set_b(true);
				if(e.key.keysym.sym == SDLK_RETURN) joypad.set_start(true);
				if(e.key.keysym.sym == SDLK_RSHIFT) joypad.set_select(true);
				if(e.key.keysym.sym == SDLK_LEFT) joypad.set_left(true);
				if(e.key.keysym.sym == SDLK_RIGHT) joypad.set_right(true);
				if(e.key.keysym.sym == SDLK_UP) joypad.set_up(true);
				if(e.key.keysym.sym == SDLK_DOWN) joypad.set_down(true);
			}

			if (e.type == SDL_KEYUP) {
				if(e.key.keysym.sym == SDLK_z) joypad.set_a(false);
				if(e.key.keysym.sym == SDLK_x) joypad.set_b(false);
				if(e.key.keysym.sym == SDLK_RETURN) joypad.set_start(false);
				if(e.key.keysym.sym == SDLK_RSHIFT) joypad.set_select(false);
				if(e.key.keysym.sym == SDLK_LEFT) joypad.set_left(false);
				if(e.key.keysym.sym == SDLK_RIGHT) joypad.set_right(false);
				if(e.key.keysym.sym == SDLK_UP) joypad.set_up(false);
				if(e.key.keysym.sym == SDLK_DOWN) joypad.set_down(false);
			}
			*/
		}
		return true;
	}	

	void PPU::shutdownPPU() {
		if (texture_) { SDL_DestroyTexture(texture_); texture_ = nullptr; }
		if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
		if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
		SDL_Quit();
	}

	void PPU::renderTestPattern(uint32_t frame) {
		// 1) framebuffer_ 채우기 (RGBA)
		for (int y = 0; y < 144; y++) {
			for (int x = 0; x < 160; x++) {
				u8 r=0, g=0, b=0, a=0xFF;

				bool left = (x < 80);
				bool top  = (y < 72);

				// 4분면 색상 (방향 검증용)
				if (top && left)      { r = 0xFF; g = 0x00; b = 0x00; } // TL red
				else if (top && !left){ r = 0x00; g = 0xFF; b = 0x00; } // TR green
				else if (!top && left){ r = 0x00; g = 0x00; b = 0xFF; } // BL blue
				else                  { r = 0xFF; g = 0xFF; b = 0xFF; } // BR white

				// 움직이는 컬러바 (업데이트/시간 흐름 검증)
				int bar_x = (frame % 160);
				if (x == bar_x || x == bar_x + 1) {
					r = 0x00; g = 0x00; b = 0x00; // 검은 세로줄
				}

				const int idx = (y * 160 + x) * 4;
				framebuffer_[idx + 0] = r;
				framebuffer_[idx + 1] = g;
				framebuffer_[idx + 2] = b;
				framebuffer_[idx + 3] = a;
			}
		}

		// 2) texture 업데이트
		SDL_UpdateTexture(texture_, nullptr, framebuffer_.data(), 160 * 4);

		// 3) 렌더
		SDL_RenderClear(renderer_);
		SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
		SDL_RenderPresent(renderer_);
	}
	
	u8 PPU::read8(u16 addr) {
		if(addr >= 0x8000 && addr < 0xA000) return vram_[addr-0x8000];
		else if(addr >= 0xFE00 && addr < 0xFEA0) return oam_[addr-0xFE00];

		switch(addr) {
			case 0xFF40: return lcdc_;
			case 0xFF41: return stat_;
			case 0xFF42: return scy_;
			case 0xFF43: return scx_;
			case 0xFF44: return ly_;
			case 0xFF45: return lyc_;
			case 0xFF46: return dma_;
			case 0xFF47: return bgp_;
			case 0xFF48: return obp0_;
			case 0xFF49: return obp1_;
			case 0xFF4A: return wy_;
			case 0xFF4B: return wx_;
			default: std::cout << "invalid addr in ppu.read8, @0x" << std::hex << addr << std::endl;
							 return 0xFF;
		}
	}

	void PPU::write8(u16 addr, u8 value) {
		if(addr >= 0x8000 && addr < 0xA000) vram_[addr-0x8000] = value;
		else if(addr >= 0xFE00 && addr < 0xFEA0) {
			oam_[addr-0xFE00]= value;
			//std::cout << "oam write @0x" << std::hex << (int)addr << ", value=@x" << (int)value << std::endl;
		}

		switch(addr) {
			case 0xFF40: lcdc_ = value;
									 break;
			case 0xFF41: stat_ = (stat_ & 0x07) | (value & 0x78); // CPU only can write to stat_[6:3]
									 break;
			case 0xFF42: scy_ = value;
									 break;
			case 0xFF43: scx_ = value;
									 break;
			case 0xFF44: ly_ = 0; // LY is read-only for CPU
									 break;
			case 0xFF45: lyc_ = value;
									 break;
			case 0xFF46: dma_ = value;
									 break;
			case 0xFF47: bgp_ = value;
									 //std::cout << "bgp set to" << std::hex << (int)value << std::endl;
									 break;
			case 0xFF48: obp0_ = value;
									 break;
			case 0xFF49: obp1_ = value;
									 break;
			case 0xFF4A: wy_ = value;
									 break;
			case 0xFF4B: wx_ = value;
									 break;
		}
	}

	u8 PPU::tick(int cycles) {
		u8 intr = 0;
		dot_cycles += cycles;
		u8 prev_stat_line = (((stat_ & 0x08) >> 3) && (mode == 0)) |
												(((stat_ & 0x10) >> 4) && (mode == 1)) |
												(((stat_ & 0x20) >> 5) && (mode == 2)) |
												(((stat_ & 0x40) >> 6) && (ly_ == lyc_));
		if(mode == 2 && dot_cycles >= 80) {
			mode = 3; // Pixel Transfer mode
			stat_ = (stat_ & 0xFC) | 0x3;
			dot_cycles -= 80;
			pixel_transfer();
		}
		if(mode == 3 && dot_cycles >= 172) {
			mode = 0; // H-Blank
			stat_ = (stat_ & 0xFC);
			dot_cycles -= 172;
		}
		if(mode == 0 && dot_cycles >= 204) {
			dot_cycles -= 204;
			int next_mode = (ly_++ == 143) ? 1 : 2;
			if(next_mode == 1) {
				stat_ = ((stat_ & 0xFC) | 0x01);
				intr |= 0x1;
				present();
			}
			else if(next_mode == 2) {
				stat_ = ((stat_ & 0xFC) | 0x02);
				oam_search();
			}
			mode = next_mode;
			if(ly_ == lyc_) stat_ |= 0x04;
			else stat_ &= ~0x04;
		}
		if(mode == 1 && dot_cycles >= 456) {
			dot_cycles -= 456;
			if(ly_ == 153) {
				mode = 2;
				ly_ = 0;
				stat_ = ((stat_ & 0xFC) | 0x02);
				oam_search();
			} else {
				ly_++;
				mode = 1;
				stat_ = ((stat_ & 0xFC) | 0x01);
			}
			if(ly_ == lyc_) stat_ |= 0x04;
			else stat_ &= ~0x04;
			// LCDC.7
			if((prev_lcdc_ & 0x80) == 0x80 && (lcdc_ & 0x80) != 0x80) {
				ly_ = 0;
				mode = 0;
				stat_ = (stat_ & 0xFC);
				dot_cycles = 0;
			}
		}
		
		u8 cur_stat_line = (((stat_ & 0x08) >> 3) && (mode == 0)) |
												(((stat_ & 0x10) >> 4) && (mode == 1)) |
												(((stat_ & 0x20) >> 5) && (mode == 2)) |
												(((stat_ & 0x40) >> 6) && (ly_ == lyc_));
		if(prev_stat_line == 0x0 && cur_stat_line == 0x1) {
			intr |= 0x2;
		}
		prev_lcdc_= lcdc_;
		return intr;
	}

	void PPU::oam_search() {
		// Search for total 40 sprites
		int max_obj = 10;
		for(int i = 0; i < 40; i++) {
			if(max_obj == 0) break;
		  int x_pos = oam_[4 * i + 1];
			int y_pos = oam_[4 * i + 0];
			int tile = oam_[4 * i + 2];
			int attr = oam_[4 * i + 3];

			if((lcdc_ & 0x04) == 0x04) {
				// 8 x 16
				if((x_pos != 0) && (ly_ >= y_pos - 16) && (ly_ < y_pos)) {
					Sprites sprite = {(u8)x_pos, (u8)y_pos, (u8)tile, (u8)attr};
					ly_sprites_[10-max_obj] = sprite;
					max_obj--;
				}
			} else {
				// 8 x 8
				if((x_pos != 0) && (ly_ >= y_pos - 16) && (ly_ < y_pos - 8)) {
					Sprites sprite = {(u8)x_pos, (u8)y_pos, (u8)tile, (u8)attr};
					ly_sprites_[10-max_obj] = sprite;
					max_obj--;
				}
			}
		}
		sprites_num = 10 - max_obj;
	}

	void PPU::pixel_transfer() {
		// Array for priority
		std::array<u8, 160> color_bit_array{};

		for(int i = 0; i < 160; i++) {
			/* Window Rendering */
			if((lcdc_ & 0x20) == 0x20 && wy_ <= ly_ && i >= (int)wx_ - 7) {
				// 1. Calculate window coordinate
				u8 w_x = (i - ((int)wx_ - 7));
				u8 w_y = (ly_ - wy_);

				// 2. Get tileID
				u64 w_tilemap_base = (lcdc_ & 0x40) ? 0x9C00 : 0x9800;
				u8 w_tileID = vram_[w_tilemap_base + (w_x >> 3) + ((w_y >> 3) << 5) - 0x8000];

				// 3. Get tile data
				u16 w_tiledata_addr;
				// 3-1. $8000 Addressing
				if((lcdc_ & 0x10) == 0x10) {
					w_tiledata_addr = 0x8000 + w_tileID * 0x10 + ((w_y & 0x7) << 1);
				}
				// 3-2. $8800 Addressing
				else {
					s8 s_tileID = static_cast<s8>(w_tileID);
					w_tiledata_addr = 0x9000 + s_tileID * 0x10 + ((w_y & 0x7) << 1);
				}
				u8 w_lo = vram_[w_tiledata_addr - 0x8000];
				u8 w_hi = vram_[w_tiledata_addr - 0x8000 + 1];

				// 4. Select color bit from each tiledata
				u8 bit = 0x7 - (w_x & 0x7);
				u8 color_bit = ((w_lo & (1 << bit)) >> bit) | (((w_hi & (1 << bit)) >> bit) << 1);

				// 5. Fill framebuffer
				u8 shade = (bgp_ >> (color_bit * 2)) & 0x3;
				u64 idx = (ly_ * 160 + i) << 2;
				u8 color;
				color_bit_array[i] = shade;

				switch(shade) {
					case 0x00: color = 0xFF; // white
										 break;
					case 0x01: color = 0xAA; // light gray
										 break;
					case 0x02: color = 0x55; // dark gray
										 break;
					case 0x03: color = 0x00; // black
										 break;
					default: color = 0xFF;
				}

				framebuffer_[idx] = color;     // R
				framebuffer_[idx + 1] = color; // G
				framebuffer_[idx + 2] = color; // B
				framebuffer_[idx + 3] = 0xFF;  // A

				continue;
			}
			/* BG Rendering */
			// 1. Calculate scrolled coordinate
			u8 bg_x = (scx_ + i) & 0xFF;
			u8 bg_y = (scy_ + ly_) & 0xFF;

			// 2. Get tileID by calculated coordinate
			u64 tilemap_base = (lcdc_ & 0x08) ? 0x9C00 : 0x9800;
			u8 tileID = vram_[tilemap_base + (bg_x >> 3) + ((bg_y >> 3) << 5) - 0x8000];

			// 3. Get tile data from tileID
			u16 tiledata_addr;
			// 3-1. $8000 Addressing
			if((lcdc_ & 0x10) == 0x10) {
				tiledata_addr = 0x8000 + tileID * 0x10 + ((bg_y & 0x7) << 1);
			}
			// 3-2. $8800 Addressing
			else {
				s8 s_tileID = static_cast<s8>(tileID);
				tiledata_addr = 0x9000 + s_tileID * 0x10 + ((bg_y & 0x7) << 1);
			}
			u8 lo = vram_[tiledata_addr - 0x8000];
			u8 hi = vram_[tiledata_addr - 0x8000 + 1];

			// 4. Select color bit from each tiledata
			u8 bit = 0x7 - (bg_x & 0x7);
			u8 color_bit = ((lo & (1 << bit)) >> bit) | (((hi & (1 << bit)) >> bit) << 1);

			// 5. Fill framebuffer
			u8 shade = (bgp_ >> (color_bit * 2)) & 0x3;
			u64 idx = (ly_ * 160 + i) << 2;
			u8 color;
			color_bit_array[i] = shade;

			switch(shade) {
				case 0x00: color = 0xFF; // white
								   break;
				case 0x01: color = 0xAA; // light gray
							     break;
				case 0x02: color = 0x55; // dark gray
									 break;
				case 0x03: color = 0x00; // black
									 break;
				default: color = 0xFF;
			}

			framebuffer_[idx] = color;     // R
			framebuffer_[idx + 1] = color; // G
			framebuffer_[idx + 2] = color; // B
			framebuffer_[idx + 3] = 0xFF;  // A
		}

		// 6. Sprite rendering
		for(int i = 0; i < sprites_num; i++) {
			int sprite_x = ly_sprites_[i].x - 8;
			int sprite_y = ly_sprites_[i].y - 16;
			u8 sprite_attr = ly_sprites_[i].attr;
			u8 sprite_tileID = ly_sprites_[i].tile;
			u8 sprite_height = ((lcdc_ & 0x04) == 0x04) ? 16 : 8;

			for(int x = 0; x < 8; x++) {
				if((sprite_x + x < 0) || (sprite_x + x >= 160)) continue;
				if((ly_ < sprite_y) || (ly_ >= sprite_y + sprite_height)) continue;

				// 6-1. Get tile data from tileID
				u16 sprite_tile_addr;
				if(sprite_height == 8) {
					// Y flip
					if((sprite_attr & 0x40) == 0x40) {
						sprite_tile_addr = (sprite_tileID * 0x10) + ((sprite_y + 7 - ly_) << 1);
					} else sprite_tile_addr = (sprite_tileID * 0x10) + ((ly_ - sprite_y) << 1); 
				} 
				else {
					if((ly_ - sprite_y) < 8) {
						// Y flip
						if((sprite_attr & 0x40) == 0x40) {
							sprite_tile_addr = (sprite_tileID | 0x01) * 0x10 + (((sprite_y + 15 - ly_) & 0x7) << 1);
						} else sprite_tile_addr = (sprite_tileID & 0xFE) * 0x10 + ((ly_ - sprite_y) << 1);
					} else {
						// Y flip
						if((sprite_attr & 0x40) == 0x40) {
							sprite_tile_addr = (sprite_tileID & 0xFE) * 0x10 + (((sprite_y + 15 - ly_) & 0x7) << 1);
						} else sprite_tile_addr = (sprite_tileID | 0x01) * 0x10 + (((ly_ - sprite_y) & 0x7) << 1);
					}
				}
				u8 sprite_lo = vram_[sprite_tile_addr];
				u8 sprite_hi = vram_[sprite_tile_addr + 1];
				// 6-2. Select color bit from each tiledata
				u8 sprite_bit = 0x7 - x;
				u8 sprite_color_bit = ((sprite_lo & (1 << sprite_bit)) >> sprite_bit) | (((sprite_hi & (1 << sprite_bit)) >> sprite_bit) << 1);

				if(sprite_color_bit == 0x00) continue;

				// 6-3. Fill framebuffer according to palette
				bool is_obp0 = ((sprite_attr & 0x10) == 0x10) ? false : true;
				u8 sprite_shade;
				if(is_obp0) {
					sprite_shade = (obp0_ >> (sprite_color_bit * 2)) & 0x3;
				} else {
					sprite_shade = (obp1_ >> (sprite_color_bit * 2)) & 0x3;
				}
				u64 sprite_idx = (ly_ * 160 + sprite_x + x) << 2;
				u8 sprite_color;
				switch(sprite_shade) {
					case 0x00: sprite_color = 0xFF; // white
										 break;
					case 0x01: sprite_color = 0xAA; // light gray
										 break;
					case 0x02: sprite_color = 0x55; // dark gray
										 break;
					case 0x03: sprite_color = 0x00; // black
										 break;
					default: sprite_color = 0xFF;
				}

				// 6-4. Adjust attribute
				// 6-4-1. Priority check
				if((sprite_attr & 0x80) != 0x80 || (color_bit_array[sprite_x + x] == 0x00)) {
					// 6-4-2. X flip check
					if((sprite_attr & 0x20) == 0x20) {
						sprite_idx = (ly_ * 160 + sprite_x + (7 - x)) << 2;
					}
					framebuffer_[sprite_idx] = sprite_color;     // R
					framebuffer_[sprite_idx + 1] = sprite_color; // G
					framebuffer_[sprite_idx + 2] = sprite_color; // B
					framebuffer_[sprite_idx + 3] = 0xFF; // A
				}
			}
		}
	}
} // namespace gb
