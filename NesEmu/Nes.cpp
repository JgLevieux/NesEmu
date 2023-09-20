#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Nes.h"

void CNes :: Reset(char *pRomName)
{
    m_NesRam.Reset();
    m_NesRam.LoadRom(pRomName);
    m_NesRam.m_pPpu = &m_Ppu;
    m_NesRam.m_pApu = &m_Apu;
    m_NesRam.m_p6502 = &m_6502;

    m_6502.m_pNesRam = &m_NesRam;
    m_6502.Reset();

    m_Ppu.m_pNesRam = &m_NesRam;
    m_Ppu.m_pNes = this;
    m_Ppu.Reset();

    m_Apu.m_pNesRam = &m_NesRam;
    m_Apu.m_pNes = this;
    m_Apu.Reset();

    m_nNbLogLines = 0;
    m_bPassFrame = false;
}

bool CNes :: Process(bool bStepOneCpu, bool bLog)
{
    bool bBreak = false;
    bool bLoopProcess = true;
    m_bPassFrame = false;

    while (bLoopProcess)
    {
        int nCPUCycle = m_6502.m_nCycle;
    // Do one inst.
        char pTxt[256];
        if (bLog)
            bBreak = m_6502.StepInto(pTxt, m_nNbLogLines);
        else
            bBreak = m_6502.StepInto();
        if (m_Ppu.m_bDoCpuNMI)
        {
            m_6502._NMI();
            m_Ppu.m_bDoCpuNMI = false;
        }

    // Process ppu depending of the last CPU cycles consumed.
        int nNbCPUCycle = m_6502.m_nCycle - nCPUCycle;
        int nNbPPUCycleToDo = nNbCPUCycle * 3;
        for (int i = 0 ; i < nNbPPUCycleToDo ; i++)
        {
            m_Ppu.Process();
            m_Apu.Process();
            if (m_Ppu.m_bPassFrame)
            {
                m_Apu.m_nCycle = 0;
                //m_Apu.m_nWaveFormPos = 0;
                m_6502.m_nCycle = 0;
                m_bPassFrame = true;
            }
        }

    // Log if needed.
        if (bLog)
        {
            sprintf_s(m_cDisasembleLogText[m_nNbLogLines], "%s PPU:%3d,%3d CYC:%d", pTxt, m_Ppu.m_nCycle, m_Ppu.m_nScanLines, m_6502.m_nCycle);
            m_nNbLogLines++;
            if (m_nNbLogLines >= NES_MAX_LOG_LINE)
                m_nNbLogLines = 0;
        }

    // Continue?
        if (bStepOneCpu || bBreak)
        {
            bLoopProcess = false;
        }
    }

    return bBreak;
}
