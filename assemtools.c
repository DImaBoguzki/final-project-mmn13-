#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assembler.h"
#include "assemtools.h"
#include "data.h"

/*check witch is argument*/
line isArgument(char *line,cmd cmdList[], registers regsList[],int *pindexCmd, table *labelList, table *externList)
{
	/*if the line is empty*/
	if(line==NULL)
		return empty;
	/*check if the line is comment*/
	if(line[0]==';')
		return comment;
	if(isLabel(line,cmdList,regsList,labelList,externList)==true)
		return label;
	/*check if the line is instructions*/
	if(strcmp(line,".string")==0)
		return instructString;
	if(strcmp(line,".data")==0)
		return instructData;
	if(strcmp(line,".entry")==0)
		return instructEntry;
	if(strcmp(line,".extern")==0)
		return instructExtern;
	/*check if the line is command*/
	if(isCommand(line,cmdList,pindexCmd)==true)
		return command;
	/*if the line is unknow*/
	return unknow;
}


/*check if str is name of commands*/
boolean isCommand(char *str, cmd commandList[],int* pindexCmd)
{
	int i;
	int len=strlen(str);
	if(len>1)
		len=len-1;
	for(i=0;i<=COMMAND_NUM;i++)
		if(strncmp(commandList[i].nameCmd,str,len)==0)
		{
			if(pindexCmd!=NULL)
				(*pindexCmd)=i;
			return true;
		}
	return false;
}

boolean isRegister(registers regs[],char *str, int* pindexregistrer)
{
	int i;
	for(i=0;i<=REGISTER;i++)
	{
		if(strcmp(regs[i].name,str)==0)
		{
			if(pindexregistrer!=NULL)
				(*pindexregistrer)=i;
			return true;
		}
	}
	return false;
}

/*check if str is label*/
boolean isLabel(char *str,cmd cmdList[], registers regsList[], table *labelList, table *externList)
{
	int i;
	int len=strlen(str);
	char *tmpStr=NULL;
	if(len>LABEL_STRLEN)
		return false;
	/*label name cannot be equal to command name or register name*/
	if(isCommand(str,cmdList,NULL)==true || isRegister(regsList,str,NULL)==true)
		return false;
	else if((str[0]<65 || (str[0]>90 && str[0]<97) || str[0]>122))
		return false;
	for(i=0;i<len-1;i++)
	{
		if(!(isdigit(str[i]) || isalpha(str[i])))
			return false;
	}
	if(!(str[len-1]==':'))
		return false;
	/*copy name label without : */	
	tmpStr=copyStrLastLess(str);	
	if(searchByName(labelList,tmpStr)!=NULL)
	{
		printf("label already exist in list!\n");
		return false;
	}
	else if(searchByName(externList,tmpStr)!=NULL)
	{
		printf("label already exist in list!\n");
		return false;
	}
	free(tmpStr);		
	return true;
}

boolean isOperandLabel(char *str, cmd cmdList[], registers regsList[])
{
	int i;
	int len=strlen(str);
	if(len>LABEL_STRLEN)
		return false;
	/*label name cannot be equal to command name or register name*/
	if(isCommand(str,cmdList,NULL)==true || isRegister(regsList,str,NULL)==true)
		return false;
	else if((str[0]<65 || (str[0]>90 && str[0]<97) || str[0]>122))
		return false;
	for(i=0;i<len;i++)
	{
		if(!(isdigit(str[i]) || isalpha(str[i])))
		{
			printf("invalid label name\n");
			return false;
		}
	}
	return true;
}


/*check if the instructString define correct */
boolean isString(char * str, int *indexStart)
{
	int i;
	int lengthLine=strlen(str);
	for(i=0;str[i]!='.';i++);
	(*indexStart)=(i+7); /*skip .string*/
	for(i=(*indexStart);i<lengthLine;i++)
	{
		if(str[i]=='"')
		{
			(*indexStart)=i+1;
			break;
		}
		else if(str[i]!=' ')/*must be next char is '"'*/
		{
			printf("error invalid string must start with (\")\n");
			return false;
		}
	}
	for(i=lengthLine-1;i>(*indexStart);i--)
	{
		if(str[i]=='"')
			return true;
		else if(str[i]!='\0' && str[i]!='\n' && str[i]!='\t' && str[i]!=' ')
		{
				printf("error invalid string must end with (\")\n");
			return false;
		}
	}
	printf("error invalid string\n");
	return false;
}
/*check if the data is define correct  */
boolean isData(char *str, int *indexStart)
{
	int i;
	int countData=0;
	int countComma=0;
	boolean flagSign=false;
	boolean flagComma=false;
	boolean flagDigit=false;
	for(i=0;str[i]!='.';i++);
	
	(*indexStart)=i+5; /*skip .data*/
	if(str[(*indexStart)]==' ')
		while(str[(*indexStart)]==' ')
			(*indexStart)++;
	for(i=(*indexStart);i<LINE_STR && str[i]!='\0' ;i++)
	{
		if(isdigit(str[i]))
		{
			while(isdigit(str[i])) /*skip digits*/
				i++;
			countData++;
			flagComma=false;
			flagSign=false;
			flagDigit=true;
		}
		if(str[i]==',')
		{
			if(flagComma==true)
			{
				printf("error! define data invalid!, comma (,) is multiplied!\n");
				return false;
			}
			flagComma=true;
			flagDigit=false;
			countComma++;
		}
		else if((str[i]=='-' || str[i]=='+') && flagDigit==false)
		{
			if(flagSign==true)
			{
				printf("error! define data invalid!, sign + or - is multiplied!\n");
				return false;
			}
			flagComma=false;
			flagSign=true;
		}
		else if(str[i]==' ' || str[i]=='\0')
			flagDigit=false;	
		else if (str[i]=='.')
		{
			printf("the number must be int!\n");
			return false;
		}
	}
	if((countData-1)==countComma)
		return true;
	else
	{
		printf("error invalid data!\n");
		return false;
	}
}

int ConvertToMachineCodeBase4(int num)
{
	int base4=0;
	int power=1;
	int tmp;
	int mask=3;
	int i;
	for(i=0;i<6 && num!=0;i++)
	{
		tmp=(num & mask);
		base4+=power*tmp;
		num>>=2;
		power=power*10;
	}
	return base4;
}

boolean sourceOffIncmd(cmd commandsList[],int indexCmd)
{
	int i;
	for(i=0;i<METHOD_ADDRESSING;i++)
	{
		if(commandsList[indexCmd].Source[i]!=OFF)
			return false;
	}
	return true;
}
boolean destinatOffIncmd(cmd commandsList[],int indexCmd)
{
	int i;
	for(i=0;i<METHOD_ADDRESSING;i++)
	{
		if(commandsList[indexCmd].Destinat[i]!=OFF)
			return false;
	}
	return true;
}

char *copyStrLastLess(char *str)
{
	int i;
	int len=strlen(str);
	char *copy=(char*)calloc((len-1),sizeof(char));
	for(i=0;i<len-1;i++)
		copy[i]=str[i];
	return copy;	
	
}


/*only for test!*/
void printBinaryCode(int num)
{
	char res[12];
	int i;
	for(i=0;i<12;i++)
	{
		res[i]=(num & 1);
		num>>=+1;
	}
	for(i=11;i>=0;i--)
	{
		if(i==7 || i==2)
			printf("%d-",res[i]);
		else
			printf("%d",res[i]);
	}
	printf("\n");
}
