#ifndef ASSEMTOOLS_H
#define ASSEMTOOLS_H
#include "assembler.h"
#include "data.h"


line isArgument(char *line,cmd cmdList[], registers regsList[],int *pindexCmd, table *labelList, table *externList);
boolean isCommand(char *str, cmd[],int* indexcmd);
boolean isLabel(char *str,cmd cmdList[], registers regsList[], table *labelList, table *externList);
boolean isOperandLabel(char *str,cmd cmdList[],registers regsList[]);
boolean isString(char * str,int *indexStart);
boolean isData(char *str, int *indexStart);
boolean isRegister(registers regs[],char *str,int* indexregistrer);
int DecToBase4(int num);
int ConvertToMachineCodeBase4(int num);
boolean sourceOffIncmd(cmd commandsList[],int indexCmd);
boolean destinatOffIncmd(cmd commandsList[],int indexCmd);
char *copyStrLastLess(char *str);

void printBinaryCode(int num);
#endif

