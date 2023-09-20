#pragma once

#include <stdint.h>
#include "Memory.h"

class CNes;

#define APU_WAVE_FORM_MAX_SIZE 4096

class CApu
{
public:
	struct SPulse
	{
		bool m_bEnable;
		uint8_t m_u8Sequence;
		uint16_t m_u16Reload;
		uint16_t m_u16Timer;
		float m_fOutput;

		SPulse(void)
		{
			m_bEnable = false;
			m_u8Sequence = 0;
			m_u16Reload = 0;
			m_u16Timer = 0;
			m_fOutput = 0.0f;
		}

		void Process(void);
	};

	CNes *m_pNes;
	CNesRam *m_pNesRam;
	int m_nCycle;

	float m_fWaveForm[APU_WAVE_FORM_MAX_SIZE];
	int m_nWaveFormPos;
	float m_fCycleBeforeSampleUpdate;

	SPulse m_Pulse1;
	SPulse m_Pulse2;

	void Reset(void);
	void Process(void);

	void WriteRegister(uint16_t nAdr, uint8_t u8Value);
};


