#pragma once
#include <stdint.h>

class CPpu;
class CApu;
class C6502;

class CNesRam
{
public:
	struct SRomHeader
	{
		uint8_t m_NES[4];
		uint8_t m_PRGSize;
		uint8_t m_CHRSize;
		uint8_t m_Flags[10];
	};
	struct SRom
	{
		SRomHeader m_Header;
		uint8_t m_Data[256*1024];
	};

	union
	{
		uint8_t m_CPURam[0x10000]; // 64kb
		struct
		{
			uint8_t m_WorkRam[0x800];		// 0x0000	// 2KB internal RAM
			uint8_t m_Mirror01[0x800];		// 0x0800	// Mirrors of $0000-$07FF
			uint8_t m_Mirror02[0x800];		// 0x1000
			uint8_t m_Mirror03[0x800];		// 0x1800
			uint8_t m_PPUReg[0x8];			// 0x2000	// NES PPU registers
			uint8_t m_PPURegMirror[0x1FF8];	// 0x2008	// Mirrors of $2000-2007 (repeats every 8 bytes)
			uint8_t m_Registers[0x20];		// 0x4000	// NES APU and I/O registers & APU and I/O functionality that is normally disabled.
			uint8_t m_ExpansionROM[0x1fE0];	// 0x4020
			uint8_t m_SRam[0x2000];			// 0x6000
			uint8_t m_PRGRom01[0x4000];		// 0x8000	// 16kb
			uint8_t m_PRGRom02[0x4000];		// 0xC000	// 16kb
		};
	};

	union
	{
		uint8_t m_PPURam[0x4000]; // 16kb
		struct
		{
			uint8_t m_PatternTable0[0x1000];	// $0000-$0FFF	$1000	Pattern table 0
			uint8_t m_PatternTable1[0x1000];	// $1000-$1FFF	$1000	Pattern table 1
			uint8_t m_NameTable0[0x400];		// $2000-$23FF	$0400	Nametable 0
			uint8_t m_NameTable1[0x400];		// $2400-$27FF	$0400	Nametable 1
			uint8_t m_NameTable2[0x400];		// $2800-$2BFF	$0400	Nametable 2
			uint8_t m_NameTable3[0x400];		// $2C00-$2FFF	$0400	Nametable 3
			uint8_t m_MirrorNameTable[0xF00];	// $3000-$3EFF	$0F00	Mirrors of $2000-$2EFF
			uint8_t m_Palette[0x20];			// $3F00-$3F1F	$0020	Palette RAM indexes
			uint8_t m_MirrorPalette[0x400];		// $3F20-$3FFF	$00E0	Mirrors of $3F00-$3F1F
		};
	};

	struct SPaddle
	{
		enum EPaddleStatus // http://wiki.nesdev.com/w/index.php/Standard_controller
		{
			e_PaddleStatus_WaitForWrite1,
			e_PaddleStatus_WaitForWrite0,
			e_PaddleStatus_WaitForRead0,
			e_PaddleStatus_WaitForRead1,
			e_PaddleStatus_WaitForRead2,
			e_PaddleStatus_WaitForRead3,
			e_PaddleStatus_WaitForRead4,
			e_PaddleStatus_WaitForRead5,
			e_PaddleStatus_WaitForRead6,
			e_PaddleStatus_WaitForRead7,
			e_PaddleStatus_NothingMoreToRead,
		};

		enum EPaddleButtons
		{
			e_PaddleButtons_A = 1 << 0,
			e_PaddleButtons_B = 1 << 1,
			e_PaddleButtons_Select = 1 << 2,
			e_PaddleButtons_Start = 1 << 3,
			e_PaddleButtons_Up = 1 << 4,
			e_PaddleButtons_Down = 1 << 5,
			e_PaddleButtons_Left = 1 << 6,
			e_PaddleButtons_Right = 1 << 7,
		};

		EPaddleStatus m_eStatus;
		uint8_t m_u8Buttons;

		uint8_t Read(void);
		void Write(uint8_t u8Value);
		
		void Reset(void);

		void SetButton(EPaddleButtons eButton);
		void ResetButton(EPaddleButtons eButton);
	};

	uint8_t m_u8MapperID;
	
	SPaddle m_Controller_0;
	SPaddle m_Controller_1;

	CPpu *m_pPpu;
	CApu *m_pApu;
	C6502 *m_p6502;

public:
	void Reset(void);
	void LoadRom(char *pName);

	uint8_t Read(uint16_t nAdr);
	void Write(uint16_t nAdr, uint8_t u8Value);
};
