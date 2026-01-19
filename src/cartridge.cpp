#include "gb/cartridge.hpp"

#include <fstream>
#include <iostream>

namespace gb {
	u8 Cartridge::read8(u16 addr) {
		switch(mbc_) {
			case MBCType::MBC0:
				return rom_[addr];
			case MBCType::MBC1:
				{
					u32 addr_expansion;
					if(is_bank_simple_) {
						if(addr < 0x4000) return rom_[addr];
						else if(addr < 0x8000) {
							addr_expansion = (static_cast<u32>(rom_bank_no_) << 14) + (addr - 0x4000);
							return rom_[addr_expansion];
						}
						else {
							if(ram_ena_) {
								addr_expansion = (addr - 0xA000) + ram_bank_no_ * 0x2000;
								return ram_[addr_expansion];
							}
						}
					} else {
						if(addr < 0x4000) {
							addr_expansion = (ram_bank_no_ << 19) + addr;
							return rom_[addr_expansion];
						}
						else if(addr < 0x8000) {
							addr_expansion = (static_cast<u32>(rom_bank_no_) << 14) + (addr - 0x4000);
							return rom_[addr_expansion];
						}
						else {
							if(ram_ena_) {
								addr_expansion = (addr - 0xA000) + ram_bank_no_ * 0x2000;
								return ram_[addr_expansion];
							}
						}
					}
					break;
				}
			case MBCType::MBC3:
				{
					u32 addr_expansion;
					if(addr < 0x4000) return rom_[addr];
					else if(addr < 0x8000) {
						addr_expansion = (static_cast<u32>(rom_bank_no_) << 14) + (addr - 0x4000);
						return rom_[addr_expansion];
					}
					else {
						if(ram_ena_) {
							addr_expansion = (addr - 0xA000) + ram_bank_no_ * 0x2000;
							return ram_[addr_expansion];
						}
					}
					break;
				}
		}
		return 0xFF;
	}
	
	void Cartridge::write8(u16 addr, u8 value) {
		switch(mbc_) {
			case MBCType::MBC0:
				break;
			case MBCType::MBC1:
				{
					if(addr < 0x2000 && (value & 0x0F) == 0xA) {
						ram_ena_ = true;
					}
					else if(addr < 0x2000 && (value & 0x0F) != 0xA) {
						ram_ena_ = false;
					}
					else if(addr < 0x4000) {
						rom_bank_no_ = (value & 0x1F); // 5-bit
						if(rom_bank_no_ == 0x00) rom_bank_no_ = 0x01;
						// Mask process
						if(rom_size_ == 256) rom_bank_no_ &= 0x0F;
						//std::cout << "rom bank no." << (int)rom_bank_no_ << "\n";
					}
					else if(addr < 0x6000) {
						ram_bank_no_ = (value & 0x03);
						//std::cout << "ram bank no." << (int)ram_bank_no_ << "\n";
					}
					else if(addr < 0x8000) {
						is_bank_simple_ = (value == 0x00);
					}

					if(addr >= 0xA000 && addr < 0xC000 && ram_ena_) {
						u32 addr_expansion = (addr - 0xA000) + ram_bank_no_ * 0x2000;
						ram_[addr_expansion] = value;
					}
					break;
				}
			case MBCType::MBC3:
				{
					if(addr < 0x2000 && (value & 0x0F) == 0xA) {
						ram_ena_ = true;
					}
					else if(addr < 0x2000 && (value != 0x0A)) {
						ram_ena_ = false;
					}
					else if(addr < 0x4000) {
						rom_bank_no_ = (value & 0x7F); // 7-bit
						if(rom_bank_no_ == 0x00) rom_bank_no_ = 0x01;
					}
					else if(addr < 0x6000) {
						ram_bank_no_ = (value & 0x07);
					}
					else if(addr < 0x8000) {
						//is_bank_simple_ = (value == 0x00);
					}

					if(addr >= 0xA000 && addr < 0xC000 && ram_ena_) {
						u32 addr_expansion = (addr - 0xA000) + ram_bank_no_ * 0x2000;
						ram_[addr_expansion] = value;
					}
					break;
				}
		}
	}

	bool Cartridge::is_save() {
		if(mbc_ == MBCType::MBC3) return true;
		return false;
	}

	bool Cartridge::load_cartridge(const std::string &path) {
    std::ifstream ifs(path, std::ios::binary);
    if(!ifs) return false;

    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    if(!ifs.read(reinterpret_cast<char*>(rom_.data()), size)) return false;

		// Cartridge Type
		switch(rom_[0x147]) {
			case 0x00: mbc_ = MBCType::MBC0;
								 break;
			case 0x01:
			case 0x02:
			case 0x03: mbc_ = MBCType::MBC1;
								 break;
			case 0x0F:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
								 mbc_ = MBCType::MBC3;
								 break;
			// ...
		}

		std::cout << "MBC: " << (int)rom_[0x147] << '\n';

		// ROM size
		rom_size_ = 32 * (1 << rom_[0x148]);
		std::cout << "ROM: " << rom_size_ << '\n';

		// RAM size
		switch(rom_[0x149]) {
			case 0x00:
				ram_size_ = 0;
				break;
			case 0x01:
				ram_size_ = 0;
				break;
			case 0x02:
				ram_size_ = 8;
				break;
			case 0x03:
				ram_size_ = 32;
				break;
			case 0x04:
				ram_size_ = 64;
				break;
			case 0x05:
				ram_size_ = 128;
				break;
			case 0x06:
				ram_size_ = 64;
				break;
		}
		std::cout << "RAM: " << ram_size_ << '\n';

		std::filesystem::path p(path);
		auto stem = p.stem();
		path_ = stem.string();

		std::cout << "Cartridge Name: " << path_ << '\n';
    return true;
	}

	bool Cartridge::load_savefile() {
    std::ifstream ifs("saves/" + path_ + ".sav", std::ios::binary);
    if(!ifs) return false;

    ifs.seekg(0, std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    if(!ifs.read(reinterpret_cast<char*>(ram_.data()), ram_size_ * 1024)) return false;
		return true;
	}

	bool Cartridge::store_savefile() {
		std::ofstream ofs("saves/" + path_ + ".sav", std::ios::binary);
		if(!ofs) return false;

		ofs.write(reinterpret_cast<const char*>(ram_.data()), ram_size_* 1024);
		return true;
	}
} // namespace gb
