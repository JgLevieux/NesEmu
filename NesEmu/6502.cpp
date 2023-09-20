#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "6502.h"

C6502::SOpCode C6502::ms_OpCode[256] =
{
//0x00
	{7, false, " BRK", e_AdrMode_Imp, &C6502::_BRK},
	{6, false, " ORA", e_AdrMode_XInd, &C6502::_ORA},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_Zpg, &C6502::_NOP},
	{3, false, " ORA", e_AdrMode_Zpg, &C6502::_ORA},
	{5, false, " ASL", e_AdrMode_Zpg, &C6502::_ASL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{3, false, " PHP", e_AdrMode_Imp, &C6502::_PHP},
	{2, false, " ORA", e_AdrMode_Imm, &C6502::_ORA},
	{2, false, " ASL A", e_AdrMode_Acc, &C6502::_ASL_A},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_Abs, &C6502::_NOP},
	{4, false, " ORA", e_AdrMode_Abs, &C6502::_ORA},
	{6, false, " ASL", e_AdrMode_Abs, &C6502::_ASL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
//0x10			 
	{2, false, " BPL", e_AdrMode_Rel, &C6502::_BPL},
	{5, true,  " ORA", e_AdrMode_IndY, &C6502::_ORA},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " ORA", e_AdrMode_ZpgX, &C6502::_ORA},
	{6, false, " ASL", e_AdrMode_ZpgX, &C6502::_ASL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " CLC", e_AdrMode_Imp, &C6502::_CLC},
	{4, true,  " ORA", e_AdrMode_AbsY, &C6502::_ORA},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " ORA", e_AdrMode_AbsX, &C6502::_ORA},
	{7, false, " ASL", e_AdrMode_AbsX, &C6502::_ASL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
//0x20			 
	{6, false, " JSR", e_AdrMode_AbsBranch, &C6502::_JSR},
	{6, false, " AND", e_AdrMode_XInd, &C6502::_AND},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{3, false, " BIT", e_AdrMode_Zpg, &C6502::_BIT},
	{3, false, " AND", e_AdrMode_Zpg, &C6502::_AND},
	{5, false, " ROL", e_AdrMode_Zpg, &C6502::_ROL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{4, false, " PLP", e_AdrMode_Imp, &C6502::_PLP},
	{2, false, " AND", e_AdrMode_Imm, &C6502::_AND},
	{2, false, " ROL A", e_AdrMode_Acc, &C6502::_ROL_A},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{4, false, " BIT", e_AdrMode_Abs, &C6502::_BIT},
	{4, false, " AND", e_AdrMode_Abs, &C6502::_AND},
	{6, false, " ROL", e_AdrMode_Abs, &C6502::_ROL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
//0x30			 
	{2, false, " BMI", e_AdrMode_Rel, &C6502::_BMI},
	{5, true,  " AND", e_AdrMode_IndY, &C6502::_AND},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " AND", e_AdrMode_ZpgX, &C6502::_AND},
	{6, false, " ROL", e_AdrMode_ZpgX, &C6502::_ROL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " SEC", e_AdrMode_Imp, &C6502::_SEC},
	{5, true,  " AND", e_AdrMode_AbsY, &C6502::_AND},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " AND", e_AdrMode_AbsX, &C6502::_AND},
	{6, false, " ROL", e_AdrMode_AbsX, &C6502::_ROL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
//0x40			 
	{6, false, " RTI", e_AdrMode_Imp, &C6502::_RTI},
	{5, true,  " EOR", e_AdrMode_XInd, &C6502::_EOR},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_Zpg, &C6502::_NOP},
	{3, false, " EOR", e_AdrMode_Zpg, &C6502::_EOR},
	{5, false, " LSR", e_AdrMode_Zpg, &C6502::_LSR},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " PHA", e_AdrMode_Imp, &C6502::_PHA},
	{4, true,  " EOR", e_AdrMode_Imm, &C6502::_EOR},
	{2, false, " LSR A", e_AdrMode_Acc, &C6502::_LSR_A},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{3, false, " JMP", e_AdrMode_AbsBranch, &C6502::_JMP},
	{4, false, " EOR", e_AdrMode_Abs, &C6502::_EOR},
	{6, false, " LSR", e_AdrMode_Abs, &C6502::_LSR},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
//0x50			 
	{2, false, " BVC", e_AdrMode_Rel, &C6502::_BVC},
	{5, true,  " EOR", e_AdrMode_IndY, &C6502::_EOR},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp, &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " EOR", e_AdrMode_ZpgX,  &C6502::_EOR},
	{6, false, " LSR", e_AdrMode_ZpgX,  &C6502::_LSR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " CLI", e_AdrMode_Imp,  &C6502::_CLI},
	{4, true,  " EOR", e_AdrMode_AbsY,  &C6502::_EOR},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " EOR", e_AdrMode_AbsX,  &C6502::_EOR},
	{7, false, " LSR", e_AdrMode_AbsX,  &C6502::_LSR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0x60			 
	{6, false, " RTS", e_AdrMode_Imp,  &C6502::_RTS},
	{6, false, " ADC", e_AdrMode_XInd,  &C6502::_ADC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_Zpg, &C6502::_NOP},
	{3, false, " ADC", e_AdrMode_Zpg,  &C6502::_ADC},
	{5, false, " ROR", e_AdrMode_Zpg,  &C6502::_ROR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " PLA", e_AdrMode_Imp,  &C6502::_PLA},
	{2, false, " ADC", e_AdrMode_Imm,  &C6502::_ADC},
	{2, false, " ROR A", e_AdrMode_Acc,  &C6502::_ROR_A},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{5, false, " JMP", e_AdrMode_Ind,  &C6502::_JMP},
	{4, false, " ADC", e_AdrMode_Abs,  &C6502::_ADC},
	{6, false, " ROR", e_AdrMode_Abs,  &C6502::_ROR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0x70			 
	{2, false, " BVS", e_AdrMode_Rel,  &C6502::_BVS},
	{5, true,  " ADC", e_AdrMode_IndY,  &C6502::_ADC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " ADC", e_AdrMode_ZpgX,  &C6502::_ADC},
	{6, false, " ROR", e_AdrMode_ZpgX,  &C6502::_ROR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " SEI", e_AdrMode_Imp,  &C6502::_SEI},
	{4, true,  " ADC", e_AdrMode_AbsY,  &C6502::_ADC},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " ADC", e_AdrMode_AbsX,  &C6502::_ADC},
	{7, false, " ROR", e_AdrMode_AbsX,  &C6502::_ROR},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0x80			 
	{2, false, "*NOP", e_AdrMode_Imm, &C6502::_NOP},
	{6, false, " STA", e_AdrMode_XInd,  &C6502::_STA},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{3, false, " STY", e_AdrMode_Zpg,  &C6502::_STY},
	{3, false, " STA", e_AdrMode_Zpg,  &C6502::_STA},
	{3, false, " STX", e_AdrMode_Zpg,  &C6502::_STX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " DEY", e_AdrMode_Imp,  &C6502::_DEY},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " TXA", e_AdrMode_Imp,  &C6502::_TXA},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " STY", e_AdrMode_Abs,  &C6502::_STY},
	{4, false, " STA", e_AdrMode_Abs,  &C6502::_STA},
	{4, false, " STX", e_AdrMode_Abs,  &C6502::_STX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0x90			 
	{2, false, " BCC", e_AdrMode_Rel,  &C6502::_BCC},
	{6, false, " STA", e_AdrMode_IndY,  &C6502::_STA},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " STY", e_AdrMode_ZpgX,  &C6502::_STY},
	{4, false, " STA", e_AdrMode_ZpgX,  &C6502::_STA},
	{4, false, " STX", e_AdrMode_ZpgY,  &C6502::_STX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " TYA", e_AdrMode_Imp,  &C6502::_TYA},
	{5, false, " STA", e_AdrMode_AbsY,  &C6502::_STA},
	{2, false, " TXS", e_AdrMode_Imp,  &C6502::_TXS},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{5, false, " STA", e_AdrMode_AbsX,  &C6502::_STA},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xA0			 
	{2, false, " LDY", e_AdrMode_Imm,  &C6502::_LDY},
	{6, false, " LDA", e_AdrMode_XInd,  &C6502::_LDA},
	{2, false, " LDX", e_AdrMode_Imm,  &C6502::_LDX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{3, false, " LDY", e_AdrMode_Zpg,  &C6502::_LDY},
	{3, false, " LDA", e_AdrMode_Zpg,  &C6502::_LDA},
	{3, false, " LDX", e_AdrMode_Zpg,  &C6502::_LDX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " TAY", e_AdrMode_Imp,  &C6502::_TAY},
	{2, false, " LDA", e_AdrMode_Imm,  &C6502::_LDA},
	{2, false, " TAX", e_AdrMode_Imp,  &C6502::_TAX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " LDY", e_AdrMode_Abs,  &C6502::_LDY},
	{4, false, " LDA", e_AdrMode_Abs,  &C6502::_LDA},
	{4, false, " LDX", e_AdrMode_Abs,  &C6502::_LDX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xB0			 
	{2, false, " BCS", e_AdrMode_Rel,  &C6502::_BCS},
	{5, true,  " LDA", e_AdrMode_IndY,  &C6502::_LDA},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " LDY", e_AdrMode_ZpgX,  &C6502::_LDY},
	{4, false, " LDA", e_AdrMode_ZpgX,  &C6502::_LDA},
	{4, false, " LDX", e_AdrMode_ZpgY,  &C6502::_LDX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " CLV", e_AdrMode_Imp,  &C6502::_CLV},
	{4, true,  " LDA", e_AdrMode_AbsY,  &C6502::_LDA},
	{2, false, " TSX", e_AdrMode_Imp,  &C6502::_TSX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, true,  " LDY", e_AdrMode_AbsX,  &C6502::_LDY},
	{4, true,  " LDA", e_AdrMode_AbsX,  &C6502::_LDA},
	{4, true,  " LDX", e_AdrMode_AbsY,  &C6502::_LDX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xC0			 
	{2, false, " CPY", e_AdrMode_Imm,  &C6502::_CPY},
	{6, false, " CMP", e_AdrMode_XInd,  &C6502::_CMP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{3, false, " CPY", e_AdrMode_Zpg,  &C6502::_CPY},
	{3, false, " CMP", e_AdrMode_Zpg,  &C6502::_CMP},
	{5, false, " DEC", e_AdrMode_Zpg,  &C6502::_DEC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " INY", e_AdrMode_Imp,  &C6502::_INY},
	{2, false, " CMP", e_AdrMode_Imm,  &C6502::_CMP},
	{2, false, " DEX", e_AdrMode_Imp,  &C6502::_DEX},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " CPY", e_AdrMode_Abs,  &C6502::_CPY},
	{4, false, " CMP", e_AdrMode_Abs,  &C6502::_CMP},
	{6, false, " DEC", e_AdrMode_Abs,  &C6502::_DEC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xD0			 
	{2, false, " BNE", e_AdrMode_Rel,  &C6502::_BNE},
	{5, true,  " CMP", e_AdrMode_IndY,  &C6502::_CMP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " CMP", e_AdrMode_ZpgX,  &C6502::_CMP},
	{6, false, " DEC", e_AdrMode_ZpgX,  &C6502::_DEC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " CLD", e_AdrMode_Imp,  &C6502::_CLD},
	{4, true,  " CMP", e_AdrMode_AbsY,  &C6502::_CMP},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " CMP", e_AdrMode_AbsX,  &C6502::_CMP},
	{7, false, " DEC", e_AdrMode_AbsX,  &C6502::_DEC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xE0			 
	{2, false, " CPX", e_AdrMode_Imm,  &C6502::_CPX},
	{6, false, " SBC", e_AdrMode_XInd,  &C6502::_SBC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{3, false, " CPX", e_AdrMode_Zpg,  &C6502::_CPX},
	{3, false, " SBC", e_AdrMode_Zpg,  &C6502::_SBC},
	{5, false, " INC", e_AdrMode_Zpg,  &C6502::_INC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " INX", e_AdrMode_Imp,  &C6502::_INX},
	{2, false, " SBC", e_AdrMode_Imm,  &C6502::_SBC},
	{2, false, " NOP", e_AdrMode_Imp,  &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{4, false, " CPX", e_AdrMode_Abs,  &C6502::_CPX},
	{4, false, " SBC", e_AdrMode_Abs,  &C6502::_SBC},
	{6, false, " INC", e_AdrMode_Abs,  &C6502::_INC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
//0xF0			 
	{2, false, " BEQ", e_AdrMode_Rel,  &C6502::_BEQ},
	{5, true,  " SBC", e_AdrMode_IndY,  &C6502::_SBC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_ZpgX, &C6502::_NOP},
	{4, false, " SBC", e_AdrMode_ZpgX,  &C6502::_SBC},
	{6, false, " INC", e_AdrMode_ZpgX,  &C6502::_INC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, " SED", e_AdrMode_Imp,  &C6502::_SED},
	{4, true,  " SBC", e_AdrMode_AbsY,  &C6502::_SBC},
	{2, false, "*NOP", e_AdrMode_Imp, &C6502::_NOP},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL},
	{2, false, "*NOP", e_AdrMode_AbsX, &C6502::_NOP},
	{4, true,  " SBC", e_AdrMode_AbsX,  &C6502::_SBC},
	{7, false, " INC", e_AdrMode_AbsX,  &C6502::_INC},
	{2, false, " ILLEGAL", e_AdrMode_Imp,  &C6502::_ILLEGAL}
};


char *C6502::ms_pAdressModeString[C6502::e_NbAdressMode]=
{
	"",										//	e_AdrMode_Acc,			A	Accumulator	OPC A	operand is AC (implied single byte instruction)
	"$%04X = %02X",							//	e_AdrMode_Abs,			abs	absolute	OPC $LLHH	operand is address $HHLL *
	"$%04X",								//	e_AdrMode_AbsBranch,	abs	absolute	OPC $LLHH	operand is address $HHLL *
	"$%04X,X @ %04X = %02X",				//	e_AdrMode_AbsX,			abs,X	absolute, X-indexed	OPC $LLHH,X	operand is address; effective address is address incremented by X with carry **
	"$%04X,Y @ %04X = %02X",				//	e_AdrMode_AbsY,			abs,Y	absolute, Y-indexed	OPC $LLHH,Y	operand is address; effective address is address incremented by Y with carry **
	"#$%02X",								//	e_AdrMode_Imm,			#	immediate	OPC #$BB	operand is byte BB
	"",										//	e_AdrMode_Imp,			impl	implied	OPC	operand implied
	"($%04X) = %04X",						//	e_AdrMode_Ind,			ind	indirect	OPC ($LLHH)	operand is address; effective address is contents of word at address: C.w($HHLL)
	"($%02X,X) @ %02X = %04X = %02X",		//	e_AdrMode_XInd,			X,ind	X-indexed, indirect	OPC ($LL,X)	operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
	"($%02X),Y = %04X @ %04X = %02X",		//	e_AdrMode_IndY,			ind,Y	indirect, Y-indexed	OPC ($LL),Y	operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
	"$%04X",								//	e_AdrMode_Rel,			rel	relative	OPC $BB	branch target is PC + signed offset BB ***
	"$%02X = %02X",							//	e_AdrMode_Zpg,			zpg	zeropage	OPC $LL	operand is zeropage address (hi-byte is zero, address = $00LL)
	"$%02X,X @ %02X = %02X",				//	e_AdrMode_ZpgX,			zpg,X	zeropage, X-indexed	OPC $LL,X	operand is zeropage address; effective address is address incremented by X without carry **
	"$%02X,Y @ %02X = %02X",				//	e_AdrMode_ZpgY,			zpg,Y	zeropage, Y-indexed	OPC $LL,Y	operand is zeropage address; effective address is address incremented by Y without carry **
};

C6502 :: C6502(void)
{
	for (int i = 0 ; i < NB_BREAKPOINT ; i++)
	{
		m_u16Breakpoint[i] = 0xFFFF;
	}
}

void C6502 :: Reset(void)
{
	m_SR = 0x24;
	m_PC = Get16bitsAdrFromMemory(0xFFFC);
	m_A = 0;
	m_X = 0;
	m_Y = 0;
	m_SP = 0xFD;

	m_nCycle = 0;

	m_bZpgAdr = false;
	m_bReadMemoryForDebug = false;
}

void C6502 :: SetBreakpoint(uint16_t u16AdrBreakPoint, int nNumBreakpoint)
{
	if (nNumBreakpoint >= NB_BREAKPOINT)
		return;
	m_u16Breakpoint[nNumBreakpoint] = u16AdrBreakPoint;
}

void C6502 :: Push8bits(uint8_t cValue)
{
	Store8bitsValueInMemory(0x100+m_SP, cValue);
	m_SP -= 1;
}

uint8_t C6502 :: Pull8bits(void)
{
	m_SP += 1;
	return Get8bitsValueFromMemory(0x100+m_SP);
}

void C6502 :: PushPC(uint16_t nPC)
{
	Store16bitsAdrInMemory(0x100+m_SP-1, nPC);
	m_SP -= 2;
}

uint16_t C6502 :: PullPC(void)
{
	m_SP += 2;
	uint16_t u16Value = ((Get8bitsValueFromMemory(0x100+m_SP)<<8) & 0xFF00) | (Get8bitsValueFromMemory(0x100+m_SP-1));
	return u16Value;
}

bool C6502 :: AddRelToPC(uint8_t cRel)
{
	char cSignedRel = (char)cRel;
	int nSignedPC = m_PC;
	nSignedPC += cSignedRel;
	nSignedPC &= 0xFFFF;
	uint16_t nOldPC = m_PC;
	m_PC = (uint16_t)nSignedPC;
	
// page boundary gets crossed ?
	if ((m_PC & 0xFF00) == (nOldPC & 0xFF00))
		return false;
	return true;
}

uint16_t C6502 :: Disasm(uint16_t nAdr, char *pText, bool bAddReg)
{
	m_bReadMemoryForDebug = true;
	uint8_t cOpCode = m_pNesRam->m_CPURam[nAdr];

	char pcOperand[256];
	char pcOpcode[256];
	uint16_t nAdrInst = nAdr;
	m_bZpgAdr = false;

	switch (ms_OpCode[cOpCode].m_eAdressMode)
	{
		case e_AdrMode_Acc:
		case e_AdrMode_Imp:
			pcOperand[0] = 0;
			sprintf_s(pcOpcode, "%02X      ", cOpCode);
			nAdr += 1;
			break;

		case e_AdrMode_Rel:
		{
			int8_t s8SignedRel = (int8_t)Get8bitsValueFromMemory(nAdr+1);
			int16_t s16SignedPC = nAdr + 2;
			s16SignedPC += s8SignedRel;
			s16SignedPC &= 0xFFFF;

			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], (uint16_t)s16SignedPC);
			sprintf_s(pcOpcode, "%02X %02X   ", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;
		}

		case e_AdrMode_Zpg:
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(Get8bitsValueFromMemory(nAdr+1)&0xFF));
			sprintf_s(pcOpcode, "%02X %02X   ", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;

		case e_AdrMode_ZpgX:
		{
			uint16_t u16AdrFetched = Get8bitsValueFromMemory(nAdr+1);
			uint8_t u8ValueFetched = Get8bitsValueFromMemory((u16AdrFetched+m_X)&0xFF);

			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16AdrFetched, (u16AdrFetched+m_X)&0xFF, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X   ", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;
		}

		case e_AdrMode_ZpgY:
		{
			uint16_t u16AdrFetched = Get8bitsValueFromMemory(nAdr+1);
			uint8_t u8ValueFetched = Get8bitsValueFromMemory((u16AdrFetched+m_Y)&0xFF);

			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16AdrFetched, (u16AdrFetched+m_Y)&0xFF, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X   ", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;
		}

		case e_AdrMode_Imm:
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], Get8bitsValueFromMemory(nAdr+1));
			sprintf_s(pcOpcode, "%02X %02X   ", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;

		case e_AdrMode_Abs:
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], Get16bitsAdrFromMemory(nAdr+1), Get8bitsValueFromMemory(Get16bitsAdrFromMemory(nAdr+1)));
			sprintf_s(pcOpcode, "%02X %02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(nAdr+2));
			nAdr += 3;
			break;

		case e_AdrMode_AbsBranch:
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], Get16bitsAdrFromMemory(nAdr+1));
			sprintf_s(pcOpcode, "%02X %02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(nAdr+2));
			nAdr += 3;
			break;

		case e_AdrMode_AbsX:
		{
			uint16_t u16AdrFetched = Get16bitsAdrFromMemory(nAdr+1);
			uint8_t u8ValueFetched = Get8bitsValueFromMemory(u16AdrFetched+m_X);
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16AdrFetched, (u16AdrFetched+m_X)&0XFFFF, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(nAdr+2));
			nAdr += 3;
			break;
		}

		case e_AdrMode_AbsY:
		{
			uint16_t u16AdrFetched = Get16bitsAdrFromMemory(nAdr+1);
			uint8_t u8ValueFetched = Get8bitsValueFromMemory(u16AdrFetched+m_Y);
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16AdrFetched, (u16AdrFetched+m_Y)&0XFFFF, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(nAdr+2));
			nAdr += 3;
			break;
		}

		case e_AdrMode_Ind:
		{
			uint16_t u16AdrFetched = Get16bitsAdrFromMemory(nAdr+1);

			if ((m_u16AdrFetched & 0xFF) == 0xFF) // Simulate page boundary bug.
				u16AdrFetched = (Get8bitsValueFromMemory(u16AdrFetched & 0xFF00) << 8) | Get8bitsValueFromMemory(u16AdrFetched);
			else
				u16AdrFetched = Get16bitsAdrFromMemory(u16AdrFetched);

			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], Get16bitsAdrFromMemory(nAdr+1), u16AdrFetched);
			sprintf_s(pcOpcode, "%02X %02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1), Get8bitsValueFromMemory(nAdr+2));
			nAdr += 3;
			break;
		}

		case e_AdrMode_XInd:
		{
			uint16_t u16Adr = Get8bitsValueFromMemory(nAdr+1);
			uint16_t lo = Get8bitsValueFromMemory((uint16_t)(u16Adr + (uint16_t)m_X) & 0x00FF);
			uint16_t hi = Get8bitsValueFromMemory((uint16_t)(u16Adr + (uint16_t)m_X + 1) & 0x00FF);
			uint16_t u16AdrFetched = (hi << 8) | lo;
			uint8_t u8ValueFetched = Get8bitsValueFromMemory(u16AdrFetched);

			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16Adr, (u16Adr+m_X)&0xFF, u16AdrFetched, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;
		}

		case e_AdrMode_IndY:
		{
			uint16_t u16Adr = Get8bitsValueFromMemory(nAdr+1);
			uint16_t lo = Get8bitsValueFromMemory(u16Adr & 0x00FF);
			uint16_t hi = Get8bitsValueFromMemory((u16Adr + 1) & 0x00FF);
			uint16_t u16AdrFetched = (hi << 8) | lo;
			uint8_t u8ValueFetched = Get8bitsValueFromMemory(u16AdrFetched+m_Y);
			sprintf_s(pcOperand, ms_pAdressModeString[ms_OpCode[cOpCode].m_eAdressMode], u16Adr, u16AdrFetched, (u16AdrFetched+m_Y)&0XFFFF, u8ValueFetched);
			sprintf_s(pcOpcode, "%02X %02X", cOpCode, Get8bitsValueFromMemory(nAdr+1));
			nAdr += 2;
			break;
		}
	}

	sprintf(pText, "%04X  %s                                                                         ", nAdrInst, pcOpcode);
	sprintf(pText+15, "%s %s                                                                         ", ms_OpCode[cOpCode].m_pInstTxt, pcOperand);
	if (bAddReg)
		sprintf(pText+48, "A:%02X X:%02X Y:%02X P:%02X SP:%02X", m_A, m_X, m_Y, m_SR, m_SP);

	m_bReadMemoryForDebug = false;

	return nAdr;
}

bool C6502 :: StepInto(char *pDisasmText, int nLineInLog)
{
	uint8_t cOpCode = m_pNesRam->Read(m_PC);

	if (nLineInLog == 5071)
		nLineInLog = nLineInLog;
	if (cOpCode == 0x6c)
		cOpCode = cOpCode;
	if (m_PC == 0xc00e)
		m_PC = m_PC;

	if (pDisasmText)
		Disasm(m_PC, pDisasmText, true);

	m_PC += 1;
	m_u8ValueFetched = 0;
	m_u16AdrFetched = 0;
	m_bBreak = false;
	m_bZpgAdr = false;

	switch (ms_OpCode[cOpCode].m_eAdressMode)
	{
		case e_AdrMode_Acc:
		case e_AdrMode_Imp:
		{
			break;
		}

		case e_AdrMode_Imm:
		{
			m_u8ValueFetched = Get8bitsValueFromMemory(m_PC);
			m_PC += 1;
			break;
		}

		case e_AdrMode_Rel:
		{
			m_u8ValueFetched = Get8bitsValueFromMemory(m_PC);
			m_PC += 1;
			break;
		}

		case e_AdrMode_Zpg:
		{
			m_u16AdrFetched = Get8bitsValueFromMemory(m_PC);
			m_bZpgAdr = true;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 1;
			break;
		}

		case e_AdrMode_ZpgX:
		{
			m_u16AdrFetched = Get8bitsValueFromMemory(m_PC);
			m_u16AdrFetched += m_X;
			m_bZpgAdr = true;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 1;
			break;
		}

		case e_AdrMode_ZpgY:
		{
			m_u16AdrFetched = Get8bitsValueFromMemory(m_PC);
			m_u16AdrFetched += m_Y;
			m_bZpgAdr = true;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 1;
			break;
		}

		case e_AdrMode_Abs:
		case e_AdrMode_AbsBranch:
		{
			m_u16AdrFetched = Get16bitsAdrFromMemory(m_PC);
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 2;
			break;
		}

		case e_AdrMode_AbsX:
		{
			m_u16AdrFetched = Get16bitsAdrFromMemory(m_PC);
			uint16_t u16BeforeAdd = m_u16AdrFetched;
			m_u16AdrFetched += m_X;
			if (((m_u16AdrFetched & 0xFF00) != (u16BeforeAdd & 0xFF00)) && ms_OpCode[cOpCode].m_bAddCycleWhenBound)
				m_nCycle += 1;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 2;
			break;
		}

		case e_AdrMode_AbsY:
		{
			m_u16AdrFetched = Get16bitsAdrFromMemory(m_PC);
			uint16_t u16BeforeAdd = m_u16AdrFetched;
			m_u16AdrFetched += m_Y;
			if (((m_u16AdrFetched & 0xFF00) != (u16BeforeAdd & 0xFF00)) && ms_OpCode[cOpCode].m_bAddCycleWhenBound)
				m_nCycle += 1;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 2;
			break;
		}

		case e_AdrMode_Ind:
		{
			m_u16AdrFetched = Get16bitsAdrFromMemory(m_PC);

			if ((m_u16AdrFetched & 0xFF) == 0xFF) // Simulate page boundary bug.
				m_u16AdrFetched = (Get8bitsValueFromMemory(m_u16AdrFetched & 0xFF00) << 8) | Get8bitsValueFromMemory(m_u16AdrFetched);
			else
				m_u16AdrFetched = Get16bitsAdrFromMemory(m_u16AdrFetched);
	
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 2;
			break;
		}

		case e_AdrMode_XInd:
		{
			uint16_t u16Adr = Get8bitsValueFromMemory(m_PC);
			uint16_t lo = Get8bitsValueFromMemory((uint16_t)(u16Adr + (uint16_t)m_X) & 0x00FF);
			uint16_t hi = Get8bitsValueFromMemory((uint16_t)(u16Adr + (uint16_t)m_X + 1) & 0x00FF);
			m_u16AdrFetched = (hi << 8) | lo;
			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 1;
			break;
		}
		
		case e_AdrMode_IndY:
		{
			uint16_t u16Adr = Get8bitsValueFromMemory(m_PC);
			uint16_t lo = Get8bitsValueFromMemory(u16Adr & 0x00FF);
			uint16_t hi = Get8bitsValueFromMemory((u16Adr + 1) & 0x00FF);
			m_u16AdrFetched = (hi << 8) | lo;
			uint16_t u16BeforeAdd = m_u16AdrFetched;
			m_u16AdrFetched += m_Y;

			if (((m_u16AdrFetched & 0xFF00) != (u16BeforeAdd & 0xFF00)) && ms_OpCode[cOpCode].m_bAddCycleWhenBound)
				m_nCycle += 1;

			m_u8ValueFetched = Get8bitsValueFromMemory(m_u16AdrFetched);
			m_PC += 1;
			break;
		}
	}

	void (C6502::*pAdrFunc)(void) = ms_OpCode[cOpCode].m_ExecFunc;
	(*this.*pAdrFunc)();

	m_nCycle += ms_OpCode[cOpCode].m_nCycle;

	for (int i = 0 ; i < NB_BREAKPOINT ; i++)
	{
		if (m_u16Breakpoint[i] == m_PC)
			m_bBreak = true;
	}

	return m_bBreak;
}

uint16_t C6502 :: Get16bitsAdrFromMemory(uint16_t nAdr)
{
	if (m_bReadMemoryForDebug)
		return ((m_pNesRam->m_CPURam[nAdr+1]<<8) & 0xFF00) | (m_pNesRam->m_CPURam[nAdr]);
	return ((m_pNesRam->Read(nAdr+1)<<8) & 0xFF00) | (m_pNesRam->Read(nAdr));
}

uint8_t C6502 :: Get8bitsValueFromMemory(uint16_t nAdr)
{
	if (m_bZpgAdr)
		nAdr &= 0xFF;
	if (m_bReadMemoryForDebug)
		return m_pNesRam->m_CPURam[nAdr];

	return m_pNesRam->Read(nAdr);
}

void C6502 :: Store16bitsAdrInMemory(uint16_t nAdr, uint16_t nValue)
{
	assert(!m_bReadMemoryForDebug);
	m_pNesRam->Write(nAdr+1, (nValue & 0xFF00)>>8);
	m_pNesRam->Write(nAdr, (nValue & 0xFF));
}

void C6502 :: Store8bitsValueInMemory(uint16_t nAdr, uint8_t cValue)
{
	assert(!m_bReadMemoryForDebug);
	if (m_bZpgAdr)
		nAdr &= 0xFF;
	m_pNesRam->Write(nAdr, cValue);
}

void C6502 :: BranchOnCondition(bool bCond)
{
	if (bCond)
	{
		bool bPageBound = AddRelToPC(m_u8ValueFetched);
		m_nCycle += (bPageBound?2:1);
	}
}

void C6502 :: _NMI(void)
{
	PushPC(m_PC);
	m_B = 0;
	m_Unused = 1;
	m_I = 1;
	Push8bits(m_SR);

	m_PC = Get16bitsAdrFromMemory(0xFFFA);

	m_nCycle += 8;
}

#define SET_FLAGS_NZ_A m_Z = m_A ? 0 : 1; m_N = m_A&0x80 ? 1 : 0;
#define SET_FLAGS_NZ_X m_Z = m_X ? 0 : 1; m_N = m_X&0x80 ? 1 : 0;
#define SET_FLAGS_NZ_Y m_Z = m_Y ? 0 : 1; m_N = m_Y&0x80 ? 1 : 0;

void C6502 :: _ILLEGAL(void)
{
}

void C6502 :: _BRK(void)
{
	m_bBreak = true;
}

void C6502 :: _BPL(void)
{
	BranchOnCondition(m_N == 0);
}

void C6502 :: _CLC(void)
{
	m_C = 0;
}

void C6502 :: _CLI(void)
{
	m_I = 0;
}

void C6502 :: _CLD(void)
{
	m_D = 0;
}

void C6502 :: _CLV(void)
{
	m_V = 0;
}

void C6502 :: _SEC(void)
{
	m_C = 1;
}

void C6502 :: _SEI(void)
{
	m_I = 1;
}

void C6502 :: _SED(void)
{
	m_D = 1;
}


void C6502 :: _RTI(void)
{
	m_PC += 1;
	m_SR = Pull8bits();
	m_SR &= 0b11011111; // Reset B.
	m_Unused = 1;

	m_PC = PullPC();
}

void C6502 :: _NOP(void)
{
}
// Branch & Jump
void C6502 :: _BMI(void)
{
	BranchOnCondition(m_N == 1);
}

void C6502 :: _JSR(void)
{
	m_PC--;
	PushPC(m_PC);
	m_PC = m_u16AdrFetched;
}

void C6502 :: _JMP(void)
{
	m_PC = m_u16AdrFetched;
}

void C6502 :: _BVC(void)
{
	BranchOnCondition(m_V == 0);
}

void C6502 :: _BVS(void)
{
	BranchOnCondition(m_V == 1);
}

void C6502 :: _RTS(void)
{
	m_PC = PullPC();
	m_PC += 1;
}

void C6502 :: _BCC(void)
{
	BranchOnCondition(m_C == 0);
}

void C6502 :: _BCS(void)
{
	BranchOnCondition(m_C == 1);
}

void C6502 :: _BNE(void)
{
	BranchOnCondition(m_Z == 0);
}

void C6502 :: _BEQ(void)
{
	BranchOnCondition(m_Z == 1);
}

// ADC
// Got from https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp
void C6502 :: _ADC(void)
{
	uint16_t u16Temp = (uint16_t)m_A + (uint16_t)m_u8ValueFetched + (uint16_t)m_C;

	m_C = u16Temp > 255 ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0) ? 1 : 0;
	m_V = ((~((uint16_t)m_A ^ (uint16_t)m_u8ValueFetched) & ((uint16_t)m_A ^ (uint16_t)u16Temp)) & 0x0080) ? 1 : 0;
	m_N = (u16Temp & 0x80) ? 1 : 0;
	m_A = u16Temp & 0x00FF;
}

// SUB
void C6502 :: _SBC(void)
{
	uint16_t u16Value = ((uint16_t)m_u8ValueFetched) ^ 0x00FF;
	
	// Notice this is exactly the same as addition from here!
	uint16_t u16Temp = (uint16_t)m_A + (uint16_t)u16Value + (uint16_t)m_C;

	m_C = u16Temp > 255 ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0) ? 1 : 0;
	m_V = ((~((uint16_t)m_A ^ (uint16_t)u16Value) & ((uint16_t)m_A ^ (uint16_t)u16Temp)) & 0x0080) ? 1 : 0;
	m_N = (u16Temp & 0x80) ? 1 : 0;
	m_A = u16Temp & 0x00FF;
}

// AND
void C6502 :: _AND(void)
{
	m_A = m_u8ValueFetched & m_A;
	SET_FLAGS_NZ_A
}

// ASL
void C6502 :: _ASL(void)
{
	uint16_t u16Temp = (uint16_t)m_u8ValueFetched << 1;
	m_C = ((u16Temp & 0xFF00) > 0) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x80) ? 1 : 0;
	Store8bitsValueInMemory(m_u16AdrFetched, u16Temp & 0x00FF);
}

void C6502 :: _ASL_A(void)
{
	uint16_t u16Temp = (uint16_t)m_A << 1;
	m_C = ((u16Temp & 0xFF00) > 0) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x80) ? 1 : 0;
	m_A = (uint8_t)(u16Temp & 0x00FF);
}

// BIT
void C6502 :: _BIT(void)
{
	m_N = (m_u8ValueFetched & 0b10000000) ? 1 : 0;
	m_V = (m_u8ValueFetched & 0b01000000) ? 1 : 0;
	m_Z = (m_u8ValueFetched & m_A) ? 0 : 1;
}

// CMP
void C6502 :: _CPY(void)
{
	if (m_Y < m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 0;
		uint8_t u8Sub = m_Y - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else if (m_Y > m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 1;
		uint8_t u8Sub = m_Y - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else
	{
		m_Z = 1;
		m_C = 1;
		m_N = 0;
	}
}

void C6502 :: _CMP(void)
{
	if (m_A < m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 0;
		uint8_t u8Sub = m_A - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else if (m_A > m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 1;
		uint8_t u8Sub = m_A - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else
	{
		m_Z = 1;
		m_C = 1;
		m_N = 0;
	}
}

void C6502 :: _CPX(void)
{
	if (m_X < m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 0;
		uint8_t u8Sub = m_X - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else if (m_X > m_u8ValueFetched)
	{
		m_Z = 0;
		m_C = 1;
		uint8_t u8Sub = m_X - m_u8ValueFetched;
		m_N = u8Sub&0x80 ? 1 : 0;
	}
	else
	{
		m_Z = 1;
		m_C = 1;
		m_N = 0;
	}
}

// DEC
void C6502 :: _DEY(void)
{
	m_Y = m_Y - 1;
	SET_FLAGS_NZ_Y
}

void C6502 :: _DEC(void)
{
	m_u8ValueFetched--;
	Store8bitsValueInMemory(m_u16AdrFetched, m_u8ValueFetched);
	m_Z = m_u8ValueFetched ? 0 : 1;
	m_N = m_u8ValueFetched&0x80 ? 1 : 0;
}

void C6502 :: _DEX(void)
{
	m_X = m_X - 1;
	SET_FLAGS_NZ_X
}

// EOR
void C6502 :: _EOR(void)
{
	m_A = m_u8ValueFetched ^ m_A;
	SET_FLAGS_NZ_A
}

//INC
void C6502 :: _INY(void)
{
	m_Y = m_Y + 1;
	SET_FLAGS_NZ_Y
}

void C6502 :: _INC(void)
{
	m_u8ValueFetched = m_u8ValueFetched + 1;
	Store8bitsValueInMemory(m_u16AdrFetched, m_u8ValueFetched);
	m_Z = m_u8ValueFetched ? 0 : 1;
	m_N = m_u8ValueFetched & 0x80 ? 1 : 0;
}

void C6502 :: _INX(void)
{
	m_X = m_X + 1;
	SET_FLAGS_NZ_X
}

//LD
void C6502 :: _LDY(void)
{
	m_Y = m_u8ValueFetched;
	SET_FLAGS_NZ_Y
}

void C6502 :: _LDX(void)
{
	m_X = m_u8ValueFetched;
	SET_FLAGS_NZ_X
}

void C6502 :: _LDA(void)
{
	m_A = m_u8ValueFetched;
	SET_FLAGS_NZ_A
}

// LSR
void C6502 :: _LSR(void)
{
	m_C = (m_u8ValueFetched & 0x01)? 1 : 0;
	uint16_t u16Temp = m_u8ValueFetched >> 1;	
	m_Z = ((u16Temp & 0x00FF) == 0x0000)? 1 : 0;
	m_N= (u16Temp & 0x0080) ? 1 : 0;
	Store8bitsValueInMemory(m_u16AdrFetched, u16Temp & 0x00FF);
}

void C6502 :: _LSR_A(void)
{
	m_C = (m_A & 0x01)? 1 : 0;
	uint16_t u16Temp = m_A >> 1;	
	m_Z = ((u16Temp & 0x00FF) == 0x0000)? 1 : 0;
	m_N = (u16Temp & 0x0080) ? 1 : 0;
	m_A = (uint8_t)(u16Temp & 0x00FF);
}

// ORA
void C6502 :: _ORA(void)
{
	m_A = m_u8ValueFetched | m_A;
	SET_FLAGS_NZ_A
}

// ROL
void C6502 :: _ROL(void)
{
	uint16_t u16Temp = (uint16_t)(m_u8ValueFetched << 1) | m_C;
	m_C = (u16Temp & 0xFF00) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x0080) ? 1 : 0;

	Store8bitsValueInMemory(m_u16AdrFetched, (uint8_t)(u16Temp & 0xFF));
}

void C6502 :: _ROL_A(void)
{
	uint16_t u16Temp = (uint16_t)(m_A << 1) | m_C;
	m_C = (u16Temp & 0xFF00) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x0080) ? 1 : 0;

	m_A = (uint8_t)(u16Temp & 0xFF);
}

// ROR
void C6502 :: _ROR(void)
{
	uint16_t u16Temp = (uint16_t)(m_C << 7) | (m_u8ValueFetched >> 1);
	m_C = (m_u8ValueFetched & 0x01) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x0080) ? 1 : 0;

	Store8bitsValueInMemory(m_u16AdrFetched, (uint8_t)(u16Temp & 0xFF));
}

void C6502 :: _ROR_A(void)
{
	uint16_t u16Temp = (uint16_t)(m_C << 7) | (m_A >> 1);
	m_C = (m_A & 0x01) ? 1 : 0;
	m_Z = ((u16Temp & 0x00FF) == 0x00) ? 1 : 0;
	m_N = (u16Temp & 0x0080) ? 1 : 0;

	m_A = (uint8_t)(u16Temp & 0xFF);
}

// Stack
void C6502 :: _PHA(void)
{
	Push8bits(m_A);
}

void C6502 :: _PHP(void)
{
	uint8_t SR = m_SR | 0b00110000; // Set U & B.
	Push8bits(SR);
}

void C6502 :: _PLA(void)
{
	m_A = Pull8bits();
	SET_FLAGS_NZ_A
}

void C6502 :: _PLP(void)
{
	uint8_t SR = Pull8bits() & 0b11001111; // Ignore U & B
	uint8_t u8SaveUB = m_SR & 0b00110000; // Get U & B from SR
	m_SR = SR | u8SaveUB;
}

// STore
void C6502 :: _STY(void)
{
	Store8bitsValueInMemory(m_u16AdrFetched, m_Y);
}

void C6502 :: _STA(void)
{
	Store8bitsValueInMemory(m_u16AdrFetched, m_A);
}

void C6502 :: _STX(void)
{
	Store8bitsValueInMemory(m_u16AdrFetched, m_X);
}

// Transfert
void C6502 :: _TXA(void)
{
	m_A = m_X;
	SET_FLAGS_NZ_A
}

void C6502 :: _TAX(void)
{
	m_X = m_A;
	SET_FLAGS_NZ_X
}

void C6502 :: _TYA(void)
{
	m_A = m_Y;
	SET_FLAGS_NZ_A
}

void C6502 :: _TAY(void)
{
	m_Y = m_A;
	SET_FLAGS_NZ_Y
}

void C6502 :: _TXS(void)
{
	m_SP = m_X;
}

void C6502 :: _TSX(void)
{
	m_X = m_SP;
	SET_FLAGS_NZ_X
}
