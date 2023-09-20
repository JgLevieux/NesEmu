#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "Nes.h"
#include "Apu.h"

void CApu :: Reset(void)
{
	m_nCycle = 0;
	m_fCycleBeforeSampleUpdate = 0.0f;
	m_nWaveFormPos = 0;
	memset(m_fWaveForm, 0, sizeof(m_fWaveForm));
}

void CApu::SPulse :: Process(void)
{
	if (m_bEnable)
	{
		m_u16Timer--;
		if (m_u16Timer == 0xFFFF)
		{
			m_u16Timer = m_u16Reload;
			m_u8Sequence = ((m_u8Sequence & 0x0001) << 7) | ((m_u8Sequence & 0x00FE) >> 1); // ROL
			m_fOutput = (float)(m_u8Sequence & 0x1);
		}
	}
	else
		m_fOutput = 0.0f;
}

void CApu :: Process(void)
{
	m_nCycle++;

	if (m_nCycle % 6 == 0)
	{
		m_Pulse1.Process();
		m_Pulse2.Process();
	}

	int const nNbFPS = 60;
	int const nNbAudioFreq = 48000;
	int const nNbSamplePerFrame = nNbAudioFreq / nNbFPS;
	int const nNbCyclesPerFrame = 89683;//5369318 / 60; //89342;
	float const fNbCycleBeforeAddSample = (float)nNbCyclesPerFrame / (float)nNbSamplePerFrame;

	m_fCycleBeforeSampleUpdate -= 1.0f;
	if (m_fCycleBeforeSampleUpdate <= 0.0f)
	{
		m_fWaveForm[m_nWaveFormPos++] = (m_Pulse1.m_fOutput + m_Pulse2.m_fOutput) * 0.5f;
		//m_fWaveForm[m_nWaveFormPos++] = m_Pulse2.m_fOutput;
		if (m_nWaveFormPos >= APU_WAVE_FORM_MAX_SIZE)
			m_nWaveFormPos = 0;
		m_fCycleBeforeSampleUpdate += fNbCycleBeforeAddSample;
	}
}

void CApu :: WriteRegister(uint16_t nAdr, uint8_t u8Value)
{
	switch (nAdr)
	{
		case 0x4000:
		{
			switch ((u8Value & 0xC0) >> 6)
			{
				case 0 : m_Pulse1.m_u8Sequence = 0b00000001; break;
				case 1 : m_Pulse1.m_u8Sequence = 0b00000011; break;
				case 2 : m_Pulse1.m_u8Sequence = 0b00001111; break;
				case 3 : m_Pulse1.m_u8Sequence = 0b11111100; break;
			}
			break;
		}
		case 0x4001:
			//m_Pulse1.m_bEnable = u8Value & 0b10000000;
			break;
		case 0x4004:
		{
			switch ((u8Value & 0xC0) >> 6)
			{
				case 0 : m_Pulse2.m_u8Sequence = 0b00000001; break;
				case 1 : m_Pulse2.m_u8Sequence = 0b00000011; break;
				case 2 : m_Pulse2.m_u8Sequence = 0b00001111; break;
				case 3 : m_Pulse2.m_u8Sequence = 0b11111100; break;
			}
			break;
		}

		case 0x4002:
			m_Pulse1.m_u16Reload = (m_Pulse1.m_u16Reload & 0xFF00) | u8Value;
			break;
		case 0x4003:
			m_Pulse1.m_u16Reload = ((uint16_t)(u8Value & 0b111) << 8) | (m_Pulse1.m_u16Reload & 0xFF);
			break;
		case 0x4005:
			//m_Pulse2.m_bEnable = u8Value & 0b10000000;
			break;
		case 0x4006:
			m_Pulse2.m_u16Reload = (m_Pulse2.m_u16Reload & 0xFF00) | u8Value;
			break;
		case 0x4007:
			m_Pulse2.m_u16Reload = ((uint16_t)(u8Value & 0b111) << 8) | (m_Pulse2.m_u16Reload & 0xFF);
			break;

		case 0x4015:
			m_Pulse1.m_bEnable = u8Value & 0b01;
			m_Pulse2.m_bEnable = u8Value & 0b10;
			break;
	}

	m_pNesRam->m_CPURam[nAdr] = u8Value;
}
