/** \file mcd_api.h
    \brief The Multi-Core Debug (MCD) API defined as a part of the SPRINT Project

    This is the definition of the Multi-Core Debug API as defined by SPRINT.

*/

#ifndef __TC1_6_h
#define __TC1_6_h

/******************************************************************************
**                      Include Files                                        **
******************************************************************************/

#include "mcd_types.h"

/*
  This is a C interface, not a C++ interface.
*/
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
** DEFINES                                                                   **
******************************************************************************/

#define NUMOPCODES	256		// Number of Tricore 1.6 Opcodes
#define INSTSTRSIZE	8		// Size of Instruction description
#define DESCSRSIZE	48		// Size of Instruction short description
#define DESCLRSIZE	196		// Size of Instruction long description


/******************************************************************************
** TYPEDEF                                                                   **
******************************************************************************/

typedef struct deASM_n
{
    uint32_t uiOpCode;			// Opcode (e.g. 0xDC for 'ji')
    uint8_t uiOpCodeLengh;		// Lenght of the opcode (e.g. 16)
	char sInStr[INSTSTRSIZE];	// Instruction Mnemonic
	char sLongName[DESCSRSIZE];	// Instruction Longname
	char sSyntax[DESCSRSIZE];	// Instruction Syntax
	char sLDescr[DESCLRSIZE];	// Instruction long description
} deASM_t;


/******************************************************************************
** STATIC VARIBLES                                                           **
******************************************************************************/

/* Deassembler look-up table, based on: https://www.infineon.com/dgdl/Infineon-TC2xx_Architecture_vol2-UM-v01_00-EN.pdf?fileId=5546d46269bda8df0169ca1bf33124a8 */
extern deASM_t tc16[NUMOPCODES] = {
	{0x3B, 32, "MOVD", "Move", "MOVD[c], const16", "Move the contents to data register"},
	{0x4E, 16, "JGTZ", "Jump if Greater Than Zero", "JGTZD[b], disp4", "If the contents of D[b] are greater than zero, then add the value specified by disp4, zero-extended and multiplied by 2, to the contents of PC and jump to that address"},
	{0x54, 16, "LD.W", "Load Word", "LD.WD[c], A[b]", "Load word contents of the memory location specified by the addressing mode into data register D[a]"},
	{0x8E, 16, "JLEZ", "Jump if Less Than or Equal to Zero", "JLEZD[b], disp4", "If the contents of D[b] are less than or equal to zero, then add the value specified by disp4, zero-extended and multiplied by 2, to the contents of PC and jump to that address"},
	{0x091, 32, "MOVH.A", "Move High to Address", "MOVH.AA[c], const16", "Move the value const16 to the most-significant half-word of address register A[c] and set the least-significant 16-bits to zero"},
	{0xCE, 16, "JGEZ", "Jump if Greater Than or Equal to Zero", "JGEZD[b], disp4", "If the contents of D[b] are greater than or equal to zero, then add the value specified by disp4, zero-extended and multiplied by 2, to the contents of PC and jump to that address"},
	{0xCD, 32, "MTCR", "Move To Core Register", "MTCRconst16, D[a]", "Move the value in data register D[a] to the Core Special Function Register (CSFR) selected by the value const16. Supervisor mode only!"},
	{0x02800049, 32, "LEA", "Load Effective Address", "LEAA[a], A[b], off10", "Compute the absolute (effective) address defined by the addressing mode and put the result in address register A[a]"},
	{0x000000C5, 32, "LEA", "Load Effective Address", "LEAA[a], off18 (ABS)", "Compute the absolute (effective) address defined by the addressing mode and put the result in address register A[a]"},
	{0xD9, 32,		 "LEA", "Load Effective Address", "LEAA[a], A[b], off16", "Compute the absolute (effective) address defined by the addressing mode and put the result in address register A[a]"},
	{0xDA, 16, "MOV", "Move", "MOVD[15], const8", "Move the contents of const8 to data register D[15]. The value const8 is zero-extended before it is moved. "},
	{0xDC, 16, "JIA", "Jump Indirect", "JIA[a]", "Load the contents of address register A[a] into PC and jump to that address. The least-significant bit is always set to 0"},
	{0xDD, 32, "JLA", "Jump and Link Absolute","JLA disp24", "Store the address of the next instruction in A[11] (return address). Load the value specified by disp24 into PC and  jump to that address"},
	{0x6E, 16, "JZ", "Jump if Zero", "JZD[15], disp8", "If contents of D[15] is equal to zero, then add value specified by disp8, sign-extended and multiplied by 2, to the contents of PC and jump to that address."},
	{0x76, 16, "JZ", "Jump if Zero", "JZD[b], disp4", "If contents of D[b] is equal to zero, then add value specified by disp4,  zero-extended and multiplied by 2, to the contents of PC and jump to that address."},
	{0xEE, 16, "JNZ", "Jump if Not Equal to Zero", "JNZD[15], disp8", "If contents of D[15] is not equal to zero, then add value specified by disp8, sign-extended and multiplied by 2, to the contents of PC and jump to that address."},
	{0xF6, 16, "JNZ", "Jump if Not Equal to Zero", "JNZD[b], disp4", "If contents of D[b] is not equal to zero, then add value specified by disp4,  zero-extended and multiplied by 2, to the contents of PC and jump to that address."},

	{0x00600037, 32, "EXTR", "Extract Bit Field", "EXTR.UD[c], D[a], pos, width", "Extract the number of consecutive bits"},
	{0x00400037, 32, "EXTR", "Extract Bit Field", "EXTRD[c], D[a], pos, width", "Extract the number of consecutive bits"},


	{0x01E0000F, 32, "ORN", "Bitwise OR-Not", "ORND[c], D[a], D[b]", "Compute the bitwise OR of the contents of data register and the ones’ complement of the contents [..]"},
	{0x01E0008F, 32, "ORN", "Bitwise OR-Not", "ORND[c], D[a], const9", "Compute the bitwise OR of the contents of data register and the ones’ complement of the contents [..]"},

	{0x01C0000F, 32, "ANDN", "Bitwise AND-Not", "ANDND[c], D[a], D[b]", "Compute the bitwise AND of the contents of data register and the ones complement of the contents [..]"},
	{0x01C0008F, 32, "ANDN", "Bitwise AND-Not", "ANDND[c], D[a], const9", "Compute the bitwise AND of the contents of data register and the ones complement of the contents [..]"},

	{0x01A0000F, 32, "XNOR", "Bitwise XNOR", "XNORD[c], D[a], D[b]", "Compute the bitwise exclusive NOR of the contents of data register [..]"},
	{0x01A0008F, 32, "XNOR", "Bitwise XNOR", "XNORD[c], D[a], const9", "Compute the bitwise exclusive NOR of the contents of data register [..]"},

	{0x0160000F, 32, "NOR", "Bitwise NOR", "NORD[c], D[a], D[b]", "Compute the bitwise NOR of the contents of data register [..]"},
	{0x0160008F, 32, "NOR", "Bitwise NOR", "NORD[c], D[a], const9", "Compute the bitwise NOR of the contents of data register [..]"},

	{0x0140000F, 32, "OR", "Bitwise OR", "ORD[c], D[a], D[b]", "Compute the bitwise OR of the contents of data register [..]"},
	{0x0140008F, 32, "OR", "Bitwise OR", "ORD[c], D[a], const9", "Compute the bitwise OR of the contents of data register [..]"},

	{0x0120000F, 32, "NAND", "Bitwise NAND", "NANDD[c], D[a], D[b]", "Compute the bitwise NAND of the contents of data register [..]"},
	{0x0120008F, 32, "NAND", "Bitwise NAND", "NANDD[c], D[a], const9", "Compute the bitwise NAND of the contents of data register [..]"},

	{0x0100000F, 32, "AND", "Bitwise AND", "ANDD[c], D[a], D[b]", "Compute the bitwise AND of the contents of data register [..]"},
	{0x0100008F, 32, "AND", "Bitwise AND", "ANDD[c], D[a], const9", "Compute the bitwise AND of the contents of data register [..]"},

	{0x0000000F, 32, "SH", "Shift", "SHD[c], D[a], D[b]", "Shift the value by the amount specified by shift count [..]"},
	{0x0000008F, 32, "SH", "Shift", "SHD[c], D[a], const9", "Shift the value by the amount specified by shift count[..]"},


	{0x00, 16, "NOP", "No Operation", "NOP", "Used to implement efficient low-power, non-operational instructions"},
	{0xFF, 32, "Mnemo", "Longname", "syntax", "Description"},
};

/******************************************************************************
** FUNCTION PROTOTYPES                                                       **
******************************************************************************/


#ifdef __cplusplus
}
#endif
//****************************************************************************
//                                 END OF FILE
//****************************************************************************


#endif // __TC1_6_h

