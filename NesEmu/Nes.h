#pragma once
#include "6502.h"
#include "Memory.h"
#include "Ppu.h"
#include "Apu.h"

#define NES_MAX_LOG_LINE 65000

class CNes
{
public:
	char m_cDisasembleLogText[NES_MAX_LOG_LINE][128];
	int m_nNbLogLines;

	C6502 m_6502;
	CNesRam m_NesRam;
	CPpu m_Ppu;
	CApu m_Apu;

	bool m_bPassFrame;

	void Reset(char *pRomName);
	bool Process(bool bStepOneCpu = false, bool bLog = false);
};

