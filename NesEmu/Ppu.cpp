#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Nes.h"
#include "Ppu.h"

CPpu::SColor CPpu::ms_pColors[64] =
{
	{84, 84, 84},	{0, 30, 116},	{8, 16, 144},	{48, 0, 136},	{68, 0, 100},	{92, 0, 48},	{84, 4, 0},	{60, 24, 0},	{32, 42, 0},	{8, 58, 0},	{0, 64, 0},	{0, 60, 0},	{0, 50, 60},	{0, 0, 0},	{0, 0, 0},	{0, 0, 0},
	{152, 150, 152},	{8, 76, 196},	{48, 50, 236},	{92, 30, 228},	{136, 20, 176},	{160, 20, 100},	{152, 34, 32},	{120, 60, 0},	{84, 90, 0},	{40, 114, 0},	{8, 124, 0},	{0, 118, 40},	{0, 102, 120},	{0, 0, 0},	{0, 0, 0},	{0, 0, 0},
	{236, 238, 236},	{76, 154, 236},	{120, 124, 236},	{176, 98, 236},	{228, 84, 236},	{236, 88, 180},	{236, 106, 100},	{212, 136, 32},	{160, 170, 0},	{116, 196, 0},	{76, 208, 32},	{56, 204, 108},	{56, 180, 204},	{60, 60, 60},	{0, 0, 0},	{0, 0, 0},
	{236, 238, 236},	{168, 204, 236},	{188, 188, 236},	{212, 178, 236},	{236, 174, 236},	{236, 174, 212},	{236, 180, 176},	{228, 196, 144},	{204, 210, 120},	{180, 222, 120},	{168, 226, 144},	{152, 226, 180},	{160, 214, 228},	{160, 162, 160},	{0, 0, 0},	{0, 0, 0},
};

void CPpu :: Reset(void)
{
	m_nCycle = 0;
	m_nScanLines = -1;
	m_bPassFrame = false;
	m_bAdressLatchIsHi = true;
	m_bDoCpuNMI = false;
	m_u16AddressLatchWrite = (uint16_t)-1;
	m_u16AddressLatchRead = (uint16_t)-1;
	m_OAMADDR = 0;
	m_u8ScrollX = 0;
	m_u8ScrollY = 0;
	
	m_u8_PPUDATA_Read = 0;
}

void CPpu :: Process(void)
{
	m_bPassFrame = false;
	m_nCycle++;

	if (m_nCycle >= 341)
	{
		m_nCycle = 0;
		m_nScanLines++;
		
		if (m_nScanLines < 241)
		{
			RenderBackground((uint8_t)m_nScanLines);
			RenderSprite((uint8_t)m_nScanLines);
		}
		else if (m_nScanLines == 241)
		{
			m_pNesRam->m_PPUReg[PPUSTATUS] |= 0x80; // Set VBlank.
			if (m_pNesRam->m_PPUReg[PPUCTRL] & 0x80)
				m_bDoCpuNMI = true;
		}
		else if (m_nScanLines > 261)
		{
			m_pNesRam->m_PPUReg[PPUSTATUS] &= ~0b10000000; // Reset VBlank.
			m_pNesRam->m_PPUReg[PPUSTATUS] &= ~0b01000000; // Reset sprite 0 hit.
			m_u8ScrollX = 0;
			m_u8ScrollY = 0;
			m_pNesRam->m_PPUReg[PPUCTRL] &= ~0b0011;
			m_bPassFrame = true;
			m_nScanLines = -1;
		}
	}
}

void CPpu :: RenderBackground(uint8_t u8Line)
{
	if (!(m_pNesRam->m_PPUReg[PPUMASK]&0b1000))
		return;

	uint8_t *pPatternTable = m_pNesRam->m_PatternTable0; // Patterntable contains 4 colors spritesheet.
	if (m_pNesRam->m_PPUReg[PPUCTRL]&0b10000)
		pPatternTable = m_pNesRam->m_PatternTable1;

	uint8_t u8ScrollXIndex = m_u8ScrollX / 8; // Nb X blocs to decal on nametable.
	uint8_t u8ScrollYIndex = m_u8ScrollY / 8; // Nb Y blocs to decal on nametable.
	uint8_t u8NbPixelScrollX = m_u8ScrollX & 0b111; // X decal in bloc for scroll.
	uint8_t u8NbPixelScrollY = m_u8ScrollY & 0b111; // Y decal in bloc for scroll.

	for (uint8_t u8y = 0 ; u8y < 0x1D ; u8y++)
	{
		uint8_t u8CurYBloc = u8y + u8ScrollYIndex;
		for (uint8_t u8x = 0 ; u8x < 0x20 ; u8x++)
		{
			uint8_t *pu8NameTable = m_pNesRam->m_NameTable0;
			uint8_t *pu8NameTableScroll = m_pNesRam->m_NameTable1;
			uint8_t u8BaseNameTable = m_pNesRam->m_PPUReg[PPUCTRL] & 0b011;
			if (u8BaseNameTable != 0)
			{
				pu8NameTable += u8BaseNameTable * 0x400;
				pu8NameTableScroll = m_pNesRam->m_NameTable0;
			}

			uint8_t u8CurXBloc = u8x + u8ScrollXIndex;
			if (u8CurXBloc >= 0x20)
			{
				u8CurXBloc -= 0x20;
				pu8NameTable = pu8NameTableScroll;
			}
			if (u8CurYBloc >= 0x1D)
			{
				u8CurYBloc -= 0x1D;
				pu8NameTable = pu8NameTableScroll;
			}
	
			uint8_t u8ChrIndex = pu8NameTable[u8CurXBloc + u8CurYBloc*0x20];
			uint8_t yidx = u8ChrIndex / 16;
			uint8_t xidx = u8ChrIndex % 16;
            uint8_t *pChrRam = &pPatternTable[xidx*16+yidx*16*16];

			uint16_t u16PalIdxInTable = (uint16_t)(u8CurXBloc/4)+(uint16_t)(u8CurYBloc/4)*8;
			assert(u16PalIdxInTable < 64);
			uint8_t u8PalIdx = pu8NameTable[u16PalIdxInTable + 0x20*0x1E];
			uint8_t u8PalNum = 0;
			if ((u8CurXBloc/2) & 1)
				if ((u8CurYBloc/2) & 1)
					u8PalNum = (u8PalIdx & 0b11000000)>>6;
				else
					u8PalNum = (u8PalIdx & 0b00001100)>>2;
			else if ((u8CurYBloc/2) & 1)
					u8PalNum = (u8PalIdx & 0b00110000)>>4;
				else
					u8PalNum = (u8PalIdx & 0b00000011);
			assert(u8PalNum < 4);

            for (int suby = 0 ; suby < 8 ; suby++)      // 8 pixels par sprites.
            {
				int nY = (u8y*8)+suby-u8NbPixelScrollY;
				if (nY < 0)
					continue;
				if (nY != u8Line)
				{
					pChrRam++;
					continue;
				}
                //uint8_t u8PixelLine = pChrRam[0] | pChrRam[8]; // Mix Bitplane 1 & BitPlane 2.
                for (int subx = 0 ; subx < 8 ; subx++)      // 8 pixels par sprites.
                {
					int nX = (u8x*8+subx)-u8NbPixelScrollX;
					if (nX < 0)
						continue;
					int nPixelCoord = nX + nY*e_ScanLines_VisibleX;
					assert(nPixelCoord >= 0 && nPixelCoord < e_ScanLines_VisibleX * e_ScanLines_VisibleY);

					uint8_t u8Color = ((pChrRam[0] >> (7 - subx)) & 0x01) | (((pChrRam[8] >> (7 - subx)) & 0x01) << 1);
					uint8_t u8NumColor = ReadPalette(u8PalNum*4 + u8Color);
					assert(u8NumColor < 64);
					m_ScreenRender[nPixelCoord] = (ms_pColors[u8NumColor].m_B << 16) | (ms_pColors[u8NumColor].m_V << 8) | (ms_pColors[u8NumColor].m_R) | (u8Color?0xF0000000:0);
                }
                pChrRam++;
            }
		}
	}
}

void CPpu :: RenderSprite(uint8_t u8Line)
{
	if (!(m_pNesRam->m_PPUReg[PPUMASK]&0b10000))
		return;

	bool b16x16Mode = m_pNesRam->m_PPUReg[PPUCTRL]&0b100000;

	for (int i = 0 ; i < NB_SPRITES ; i++)
	{
		SSprite *pS = &m_Sprites[i];

		uint8_t *pPatternTable = m_pNesRam->m_PatternTable0;
		if (b16x16Mode)
		{
			if (pS->m_u8TitleIndex & 1)
				pPatternTable = m_pNesRam->m_PatternTable1;
		}
		else if (m_pNesRam->m_PPUReg[PPUCTRL]&0b001000)
		{
			pPatternTable = m_pNesRam->m_PatternTable1;
		}

		uint8_t u8ChrIndex = pS->m_u8TitleIndex;
		uint8_t yidx = u8ChrIndex / 16;
		uint8_t xidx = u8ChrIndex % 16;
		uint8_t *pChrRam = &pPatternTable[xidx*16+yidx*16*16];

		uint8_t u8PalNum = pS->m_u8Attributes & 0b11;
		bool bFlipH = pS->m_u8Attributes & 0b01000000;
		bool bFlipV = pS->m_u8Attributes & 0b10000000;
		bool bBehingBG = pS->m_u8Attributes & 0b00100000;
		assert(u8PalNum < 4);

		if (bFlipV)
			pChrRam += 7;

		int nSize = b16x16Mode?16:8;

        for (int suby = 0 ; suby < nSize ; suby++)      // 8 pixels par sprites.
        {
			if (pS->m_u8Y+suby >= e_ScanLines_VisibleY)
				continue;
			if (pS->m_u8Y+suby != u8Line)
			{
				pChrRam += bFlipV?-1:1;
				continue;
			}

			//uint8_t u8PixelLine = pChrRam[0] | pChrRam[8]; // Mix Bitplane 1 & BitPlane 2.
            
			for (int subx = 0 ; subx < nSize ; subx++)      // 8 pixels par sprites.
            {
				if (pS->m_u8X+subx >= e_ScanLines_VisibleX)
					continue;
				int nPixelCoord = (pS->m_u8X+subx) + ((pS->m_u8Y+suby)*e_ScanLines_VisibleX);
				assert(nPixelCoord >= 0 && nPixelCoord < e_ScanLines_VisibleX * e_ScanLines_VisibleY);

				uint8_t u8Color;
				if (bFlipH)
					u8Color = ((pChrRam[0] >> (subx)) & 0x01) | (((pChrRam[8] >> (subx)) & 0x01) << 1);
				else
					u8Color = ((pChrRam[0] >> (7 - subx)) & 0x01) | (((pChrRam[8] >> (7 - subx)) & 0x01) << 1);
				if (u8Color)
				{
					uint8_t u8NumColor = m_pNesRam->m_Palette[16 + u8PalNum*4 + u8Color];
					assert(u8NumColor < 64);
					uint32_t u32Color = (ms_pColors[u8NumColor].m_B << 16) | (ms_pColors[u8NumColor].m_V << 8) | (ms_pColors[u8NumColor].m_R);

					if (i == 0 && u8Color != 0) // Sprite 0 hit ?
					{
						if (m_ScreenRender[nPixelCoord] & 0xFF000000) // Opaque background ?
						{
							m_pNesRam->m_PPUReg[PPUSTATUS] |= 0b01000000;
							//sprintf_s(m_pNes->m_cDisasembleLogText[m_pNes->m_nNbLogLines], "Hit - %d, %d", m_nScanLines, m_nCycle);
							//m_pNes->m_nNbLogLines++;
						}
					}
					if (!(bBehingBG && (m_ScreenRender[nPixelCoord] & 0xFF000000)))
						m_ScreenRender[nPixelCoord] = u32Color | 0x0F000000;
				}
				/*else
					m_ScreenRender[nPixelCoord] = 0xFF0000 ;//u32Color;*/
            }
            pChrRam += bFlipV?-1:1;
        }
	}
}

uint8_t CPpu :: ReadRegister(uint16_t nAdr)
{
	assert(nAdr >= 0x0 && nAdr <= 0x7);

	uint8_t u8ToReturn = 0x0;

	switch (nAdr)
	{
		case PPUCTRL:
			return 0x00; // No Read
			break;
		case PPUMASK:
			return 0x00; // No Read
			break;
		case PPUSTATUS:
			u8ToReturn = m_pNesRam->m_PPUReg[nAdr];
			m_pNesRam->m_PPUReg[nAdr] = m_pNesRam->m_PPUReg[nAdr] & 0x7F; // Remove VBL flag when reading.
			m_bAdressLatchIsHi = true;
			break;
		case OAMADDR:
			return 0x00; // No Read
			break;
		case OAMDATA:
			return 0x00; // No Read
			break;
		case PPUSCROLL:
			return 0x00; // No Read
			break;
		case PPUADDR:
			return 0x00; // No Read
			break;
		case PPUDATA:
			assert(m_u16AddressLatchRead < 0x4000);
			if (m_u16AddressLatchRead >= 0x3F00 && m_u16AddressLatchRead <= 0x3F1F)
			{
				u8ToReturn = ReadPalette(m_u16AddressLatchRead - 0x3F00);
				m_u16AddressLatchRead += (m_pNesRam->m_PPUReg[PPUCTRL]&0b100) ? 32 : 1;
				m_u16AddressLatchRead &= 0x3FFF;
			}
			else
			{
				u8ToReturn = m_u8_PPUDATA_Read;
				m_u8_PPUDATA_Read = m_pNesRam->m_PPURam[m_u16AddressLatchRead];
				m_u16AddressLatchRead += (m_pNesRam->m_PPUReg[PPUCTRL]&0b100) ? 32 : 1;
				m_u16AddressLatchRead &= 0x3FFF;
			}
			break;
	}

	return u8ToReturn;
}

void CPpu :: WriteRegister(uint16_t nAdr, uint8_t u8Value)
{
	assert(nAdr <= 0x7);
	
	switch (nAdr)
	{
		case PPUCTRL:
			if ((u8Value&1) == 0x1)
				nAdr = nAdr;
			break;
		case PPUMASK:
			break;
		case PPUSTATUS:
			return; // No Write.
			break;
		case OAMADDR:
			m_OAMADDR = u8Value;
			break;
		case OAMDATA:
			nAdr = nAdr;
			break;
		case PPUSCROLL:
			if (m_bAdressLatchIsHi)
				m_u8ScrollX = u8Value;
			else
				m_u8ScrollY = u8Value;
			m_bAdressLatchIsHi = !m_bAdressLatchIsHi;
			break;
		case PPUADDR:
			if (m_bAdressLatchIsHi)
			{
				m_u8AddressLatchHi = u8Value;
				m_bAdressLatchIsHi = false;
			}
			else
			{
				m_u8AddressLatchLow = u8Value;
				m_bAdressLatchIsHi = true;

				m_u16AddressLatchWrite = (m_u8AddressLatchHi<<8) | m_u8AddressLatchLow;
				m_u16AddressLatchWrite &= 0x3FFF;
				m_u16AddressLatchRead = m_u16AddressLatchWrite;
			}
			break;
		case PPUDATA:
		{
			assert(m_u16AddressLatchWrite < 0x4000);
			if (m_u16AddressLatchWrite >= 0x3F00 && m_u16AddressLatchWrite <= 0x3F1F)
				WritePalette(m_u16AddressLatchWrite - 0x3F00, u8Value);
			else
				m_pNesRam->m_PPURam[m_u16AddressLatchWrite] = u8Value;
			m_u16AddressLatchWrite += (m_pNesRam->m_PPUReg[PPUCTRL]&0b100) ? 32 : 1;
			m_u16AddressLatchWrite &= 0x3FFF;
			break;
		}
	}

	m_pNesRam->m_PPUReg[nAdr] = u8Value;
}

uint8_t CPpu :: ReadPalette(uint16_t nAdr)
{
	assert(nAdr <= 0x1F);
	uint8_t u8Color = m_pNesRam->m_Palette[nAdr];
	if ((nAdr % 4) == 0x0)
		u8Color = m_pNesRam->m_Palette[0];

	return u8Color;
}

void CPpu :: WritePalette(uint16_t nAdr, uint8_t u8Value)
{
	assert(nAdr <= 0x1F);
	m_pNesRam->m_Palette[nAdr] = u8Value;

// Mirror.
	if (nAdr == 0x00 || nAdr == 0x10)
	{
		m_pNesRam->m_Palette[0x00] = u8Value;
		m_pNesRam->m_Palette[0x10] = u8Value;
	}
	else if (nAdr == 0x04 || nAdr == 0x14)
	{
		m_pNesRam->m_Palette[0x04] = u8Value;
		m_pNesRam->m_Palette[0x14] = u8Value;
	}
	else if (nAdr == 0x08 || nAdr == 0x18)
	{
		m_pNesRam->m_Palette[0x08] = u8Value;
		m_pNesRam->m_Palette[0x18] = u8Value;
	}
	else if (nAdr == 0x0C || nAdr == 0x1C)
	{
		m_pNesRam->m_Palette[0x0C] = u8Value;
		m_pNesRam->m_Palette[0x1C] = u8Value;
	}
}

void CPpu :: OAMDMA_Write(uint8_t u8Value)
{
	uint16_t u16Adr = (u8Value << 8) + m_OAMADDR;
	assert(sizeof(m_Sprites) == NB_SPRITES*4);
	memcpy(m_Sprites, &m_pNesRam->m_CPURam[u16Adr], sizeof(m_Sprites));
}

