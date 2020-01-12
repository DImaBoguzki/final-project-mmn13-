#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include "data.h"
#define ERROR -1
#define MASK_BITS 0xFFFF
#define METHOD_ADDRESSING 4
#define OPERAND_BIT 4
#define SHIFT_TO_CMD 6
#define SHIFT_TO_GROUP 10
#define SHIFT_TO_SOURCE 4
#define SHIFT_TO_DESTINAT 2
#define SHIFT_TO_SOURCE_REGS 7
#define SHIFT_TO_DESTINAT_REGS 2
#define SHIFT_ARE 2


typedef enum{false,true}boolean;
typedef enum{empty,comment,label,command,instructString,instructData,instructEntry,instructExtern,unknow} line;
typedef enum{immediate,direct,copy,directRegister,OFF=-2} addressingType;
typedef enum{operandDestinat,operandSource} operandType;
typedef enum{A,E,R} typeEncoding;
typedef struct _cmd
{
	char nameCmd[5];
	unsigned int codeCmd : 4;
	unsigned int numOperand : 2;
	int Source[METHOD_ADDRESSING]; /*Law of operand source in command*/
	int Destinat[METHOD_ADDRESSING]; /*Law of operand destinat in command*/	
}cmd;

typedef struct _register
{
	char name[3];
	int registerWord : MAX_BITS;
}registers;

typedef struct _labelRelocatable
{
	char source[LABEL_STRLEN];
	char destinat[LABEL_STRLEN];
}labelRelocatable;

typedef struct _sysVar
{
	int lineCounter;
	int instructionCounter;
	int dataCounter;
	int indexCmd; /*index to  current command */
	int indexRegs; /*index to  current register */	
	addressingType lastOperandType;/*for addressing $$*/
	int lastOperandCoding; /*for addressing $$*/	
}sysVar;


void assembler(char *line);
addressingType addresing(char *str,cmd commandList[],registers regs[], int *op,operandType lastOperandType,operandType currentOperandType);
boolean saveData(int type);
boolean validAddress(int indexCmd,addressingType address,operandType operand);
boolean checkOperation(int indexCmd,addressingType destinat,addressingType source);
boolean encodingOperation(int indexCmd,addressingType destinat,addressingType source,int encodeDestinat,int encodeSource,int numEncoding,labelRelocatable nameLabelRelo);
int numEncoding(char *cmd);
void updateEncoding(relocatable *head,char *name);
boolean updateAddress(char *name,int address);
void print(object* head);
void resetSys(sysVar *var);
#endif

