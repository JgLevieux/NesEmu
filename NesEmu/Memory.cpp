#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "Memory.h"
#include "Ppu.h"
#include "Apu.h"
#include "6502.h"

void CNesRam :: Reset(void)
{
	memset(&m_CPURam, 0, sizeof(m_CPURam));
	memset(&m_PPURam, 0, sizeof(m_CPURam));
	m_Controller_0.Reset();
	m_Controller_1.Reset();
	m_u8MapperID = 0;
}

void CNesRam :: LoadRom(char *pName)
{
	FILE *pFile = NULL;

	fopen_s(&pFile, pName, "rb");
	if (pFile)
	{
		fseek(pFile, 0L, SEEK_END);
		long nSize = ftell(pFile);
		fseek(pFile, 0L, SEEK_SET);

		assert(nSize <= sizeof(SRom));

		SRom Rom;
		fread(&Rom, nSize, 1, pFile);
		fclose(pFile);

		m_u8MapperID = ((Rom.m_Header.m_Flags[0] >> 4) & 0xF) | (Rom.m_Header.m_Flags[1] & 0xF0);

		int nIndexForCHR = 0;
		if (Rom.m_Header.m_PRGSize == 1)
		{
			memcpy(&m_PRGRom01, Rom.m_Data, 16*1024);
			memcpy(&m_PRGRom02, Rom.m_Data, 16*1024);
			nIndexForCHR = 16*1024;
		}
		else if (Rom.m_Header.m_PRGSize == 2)
		{
			memcpy(&m_PRGRom01, Rom.m_Data, 32*1024);
			nIndexForCHR = 32*1024;
		}
		else if (Rom.m_Header.m_PRGSize == 8)
		{
			memcpy(&m_PRGRom01, Rom.m_Data, 32*1024);
			nIndexForCHR = 64*1024;
		}
		else if (Rom.m_Header.m_PRGSize == 16)
		{
			memcpy(&m_PRGRom01, Rom.m_Data, 32*1024);
			nIndexForCHR = 128*1024;
		}
		else
		{
			assert(0);
		}

		if (Rom.m_Header.m_CHRSize == 0)
		{
			memcpy(&m_PatternTable0, Rom.m_Data + 32*1024, 4*1024);
			memcpy(&m_PatternTable1, Rom.m_Data + 40*1024, 4*1024);
		}
		else if (Rom.m_Header.m_CHRSize == 1)
		{
			memcpy(&m_PatternTable0, Rom.m_Data+nIndexForCHR, 4*1024);
			memcpy(&m_PatternTable1, Rom.m_Data+nIndexForCHR + 4*1024, 4*1024);
		}
		else if (Rom.m_Header.m_CHRSize == 2)
		{
			nIndexForCHR += 8*1024;
			memcpy(&m_PatternTable0, Rom.m_Data+nIndexForCHR, 4*1024);
			memcpy(&m_PatternTable1, Rom.m_Data+nIndexForCHR + 4*1024, 4*1024);
		}
		else
		{
			assert(0);
		}
	}
}

uint8_t CNesRam :: Read(uint16_t nAdr)
{
	if (nAdr >= 0x2000 && nAdr <= 0x2007)
	{
		return m_pPpu->ReadRegister(nAdr-0x2000);
	}
	else if (nAdr == 0x4016)
	{
		return m_Controller_0.Read();
	}
	else if (nAdr == 0x4017)
	{
		return m_Controller_1.Read();
	}

	return m_CPURam[nAdr];
}

void CNesRam :: Write(uint16_t nAdr, uint8_t u8Value)
{
// PPU Registers
	if (nAdr >= 0x2000 && nAdr <= 0x2007)
	{
		m_pPpu->WriteRegister(nAdr-0x2000, u8Value);
		return;
	}
// APU Registers
	if ((nAdr >= 0x4000 && nAdr <= 0x4013) || nAdr == 0x4015 || nAdr == 0x4017)
	{
		m_pApu->WriteRegister(nAdr, u8Value);
	}
// OAM DMA (sprites data)
	if (nAdr == 0x4014)
	{
		m_pPpu->OAMDMA_Write(u8Value);
	}

// Controller
	if (nAdr == 0x4016 || nAdr == 0x4017)
	{
		m_Controller_0.Write(u8Value);
		m_Controller_1.Write(u8Value);
	}

	m_CPURam[nAdr] = u8Value;
}

void CNesRam :: SPaddle :: Reset(void)
{
	m_eStatus = e_PaddleStatus_WaitForWrite1;
}

void CNesRam :: SPaddle :: SetButton(EPaddleButtons eButton)
{
	m_u8Buttons |= eButton;
}

void CNesRam :: SPaddle :: ResetButton(EPaddleButtons eButton)
{
	m_u8Buttons &= ~eButton;
}

uint8_t CNesRam :: SPaddle :: Read(void)
{
	switch (m_eStatus)
	{
		case e_PaddleStatus_WaitForRead0:
			m_eStatus = e_PaddleStatus_WaitForRead1;
		case e_PaddleStatus_WaitForWrite1:
		case e_PaddleStatus_WaitForWrite0:
			return (m_u8Buttons & e_PaddleButtons_A) ? 1 : 0;
		case e_PaddleStatus_WaitForRead1:
			m_eStatus = e_PaddleStatus_WaitForRead2;
			return (m_u8Buttons & e_PaddleButtons_B) ? 1 : 0;
		case e_PaddleStatus_WaitForRead2:
			m_eStatus = e_PaddleStatus_WaitForRead3;
			return (m_u8Buttons & e_PaddleButtons_Select) ? 1 : 0;
		case e_PaddleStatus_WaitForRead3:
			m_eStatus = e_PaddleStatus_WaitForRead4;
			return (m_u8Buttons & e_PaddleButtons_Start) ? 1 : 0;
		case e_PaddleStatus_WaitForRead4:
			m_eStatus = e_PaddleStatus_WaitForRead5;
			return (m_u8Buttons & e_PaddleButtons_Up) ? 1 : 0;
		case e_PaddleStatus_WaitForRead5:
			m_eStatus = e_PaddleStatus_WaitForRead6;
			return (m_u8Buttons & e_PaddleButtons_Down) ? 1 : 0;
		case e_PaddleStatus_WaitForRead6:
			m_eStatus = e_PaddleStatus_WaitForRead7;
			return (m_u8Buttons & e_PaddleButtons_Left) ? 1 : 0;
		case e_PaddleStatus_WaitForRead7:
			m_eStatus = e_PaddleStatus_NothingMoreToRead;
			return (m_u8Buttons & e_PaddleButtons_Right) ? 1 : 0;
		case e_PaddleStatus_NothingMoreToRead:
			return 1;
		default:
			assert(0);
			break;
	}
	return 0;
}

void CNesRam :: SPaddle :: Write(uint8_t u8Value)
{
	if (u8Value & 1)
	{
		if (m_eStatus == e_PaddleStatus_WaitForWrite1)
			m_eStatus = e_PaddleStatus_WaitForWrite0;
		else 
			m_eStatus = e_PaddleStatus_WaitForWrite1;
	}
	else if ((u8Value & 1) == 0)
	{
		if (m_eStatus == e_PaddleStatus_WaitForWrite0)
			m_eStatus = e_PaddleStatus_WaitForRead0;
		else 
			m_eStatus = e_PaddleStatus_WaitForWrite1;
	}
}
