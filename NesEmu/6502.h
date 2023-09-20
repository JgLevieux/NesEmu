#pragma once

#include <stdint.h>
#include "Memory.h"

#define NB_BREAKPOINT 4

class C6502
{
public:
	uint16_t m_PC;
	uint8_t m_A;
	uint8_t m_X;
	uint8_t m_Y;
	uint8_t m_SP;

	union // Status Register.
	{
		struct
		{
			uint8_t m_C : 1; // bit 0
			uint8_t m_Z : 1;
			uint8_t m_I : 1;
			uint8_t m_D : 1; // bit 3
			uint8_t m_B : 1;
			uint8_t m_Unused : 1;
			uint8_t m_V : 1;
			uint8_t m_N : 1; // bit 7
		};

		uint8_t m_SR;
	};

	CNesRam *m_pNesRam;
	unsigned int m_nCycle;
	uint8_t m_u8ValueFetched;
	uint16_t m_u16AdrFetched;
	bool m_bZpgAdr;
	bool m_bReadMemoryForDebug;

	bool m_bBreak;

	void Reset(void);
	uint16_t Disasm(uint16_t nAdr, char *pText, bool bAddReg = false);
	bool StepInto(char *pDisasmText = NULL, int nLineInLog = 0);

	enum ENbAdressMode
	{
		e_AdrMode_Acc,		// A	Accumulator	OPC A	operand is AC (implied single byte instruction)
		e_AdrMode_Abs,		// abs	absolute	OPC $LLHH	operand is address $HHLL *
		e_AdrMode_AbsBranch,// abs	absolute	OPC $LLHH	operand is address $HHLL *
		e_AdrMode_AbsX,		// abs,X	absolute, X-indexed	OPC $LLHH,X	operand is address; effective address is address incremented by X with carry **
		e_AdrMode_AbsY,		// abs,Y	absolute, Y-indexed	OPC $LLHH,Y	operand is address; effective address is address incremented by Y with carry **
		e_AdrMode_Imm,		// #	immediate	OPC #$BB	operand is byte BB
		e_AdrMode_Imp,		// impl	implied	OPC	operand implied
		e_AdrMode_Ind,		// ind	indirect	OPC ($LLHH)	operand is address; effective address is contents of word at address: C.w($HHLL)
		e_AdrMode_XInd,		// X,ind	X-indexed, indirect	OPC ($LL,X)	operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
		e_AdrMode_IndY,		// ind,Y	indirect, Y-indexed	OPC ($LL),Y	operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
		e_AdrMode_Rel,		// rel	relative	OPC $BB	branch target is PC + signed offset BB ***
		e_AdrMode_Zpg,		// zpg	zeropage	OPC $LL	operand is zeropage address (hi-byte is zero, address = $00LL)
		e_AdrMode_ZpgX,		// zpg,X	zeropage, X-indexed	OPC $LL,X	operand is zeropage address; effective address is address incremented by X without carry **
		e_AdrMode_ZpgY,		// zpg,Y	zeropage, Y-indexed	OPC $LL,Y	operand is zeropage address; effective address is address incremented by Y without carry **
		e_NbAdressMode,
	};
	struct SOpCode
	{
		unsigned int m_nCycle;
		bool m_bAddCycleWhenBound;
		char *m_pInstTxt;
		ENbAdressMode m_eAdressMode;
		void (C6502::*m_ExecFunc)(void);
	};

	static char *ms_pAdressModeString[e_NbAdressMode];
	static SOpCode ms_OpCode[256];

	uint16_t m_u16Breakpoint[NB_BREAKPOINT];

	C6502(void);

	void SetBreakpoint(uint16_t u16AdrBreakPoint, int nNumBreakpoint);

	uint16_t Get16bitsAdrFromMemory(uint16_t nAdr);
	uint8_t Get8bitsValueFromMemory(uint16_t nAdr);

	void Store16bitsAdrInMemory(uint16_t nAdr, uint16_t nValue);
	void Store8bitsValueInMemory(uint16_t nAdr, uint8_t cValue);

	void Push8bits(uint8_t cValue);
	uint8_t Pull8bits(void);
	void PushPC(uint16_t nPC);
	uint16_t PullPC(void);

	void BranchOnCondition(bool bCond);
	bool AddRelToPC(uint8_t cRel); // Return TRUE if page boundary gets crossed.

	void _NMI(void);

	void _ILLEGAL(void);
	void _BRK(void);
	void _ORA(void);
	void _ASL(void);
	void _ASL_A(void);
	void _PHP(void);
	void _BPL(void);
	void _CLC(void);
	void _JSR(void);
	void _AND(void);
	void _BIT(void);
	void _ROL(void);
	void _ROL_A(void);
	void _PLP(void);
	void _BMI(void);
	void _SEC(void);
	void _RTI(void);
	void _EOR(void);
	void _LSR(void);
	void _LSR_A(void);
	void _PHA(void);
	void _JMP(void);
	void _BVC(void);
	void _CLI(void);
	void _RTS(void);
	void _ADC(void);
	void _ROR(void);
	void _ROR_A(void);
	void _PLA(void);
	void _BVS(void);
	void _SEI(void);
	void _STA(void);
	void _STY(void);
	void _STX(void);
	void _DEY(void);
	void _TXA(void);
	void _BCC(void);
	void _TYA(void);
	void _TXS(void);
	void _LDY(void);
	void _LDA(void);
	void _LDX(void);
	void _TAY(void);
	void _TAX(void);
	void _BCS(void);
	void _CLV(void);
	void _TSX(void);
	void _CPY(void);
	void _CMP(void);
	void _DEC(void);
	void _INY(void);
	void _DEX(void);
	void _BNE(void);
	void _CLD(void);
	void _CPX(void);
	void _SBC(void);
	void _INC(void);
	void _INX(void);
	void _NOP(void);
	void _BEQ(void);
	void _SED(void);
};
