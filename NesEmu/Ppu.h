#pragma once
#include <stdint.h>
#include "Memory.h"

class CNes;

#define NB_SPRITES 64

class CPpu
{
public:
	enum ERegisters
	{
		PPUCTRL = 0x00,
		PPUMASK = 0x01,
		PPUSTATUS = 0x02,
		OAMADDR = 0x03,
		OAMDATA = 0x04,
		PPUSCROLL = 0x05,
		PPUADDR = 0x06,
		PPUDATA = 0x07,
	};

	enum EScanLines
	{
		e_ScanLines_VisibleX = 256,
		e_ScanLines_VisibleY = 240,
	};

	struct SColor
	{
		uint8_t m_R;
		uint8_t m_V;
		uint8_t m_B;
	};

	struct SSprite
	{
		uint8_t m_u8Y;
		uint8_t m_u8TitleIndex;
		uint8_t m_u8Attributes;
		uint8_t m_u8X;
	};

	static SColor ms_pColors[64];
	SSprite m_Sprites[NB_SPRITES];

	CNes *m_pNes;
	CNesRam *m_pNesRam;
	int m_nCycle;
	int m_nScanLines;
	bool m_bPassFrame;
	bool m_bDoCpuNMI;

	uint32_t m_ScreenRender[e_ScanLines_VisibleX*e_ScanLines_VisibleY]; // ARGB

	uint8_t m_u8AddressLatchHi;
	uint8_t m_u8AddressLatchLow;
	uint16_t m_u16AddressLatchWrite;
	uint16_t m_u16AddressLatchRead;
	bool m_bAdressLatchIsHi;
	uint8_t m_u8ScrollX;
	uint8_t m_u8ScrollY;

	uint8_t m_u8_PPUDATA_Read;

	uint8_t m_OAMADDR;

	void Reset(void);
	void Process(void);

	void RenderBackground(uint8_t u8Line);
	void RenderSprite(uint8_t u8Line);

	uint8_t ReadRegister(uint16_t nAdr);
	void WriteRegister(uint16_t nAdr, uint8_t u8Value);

	uint8_t ReadPalette(uint16_t nAdr);
	void WritePalette(uint16_t nAdr, uint8_t u8Value);

	void OAMDMA_Write(uint8_t u8Value);
};
