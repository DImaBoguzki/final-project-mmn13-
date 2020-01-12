#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assembler.h"
#include "assemtools.h"
#include "data.h"
#include "assemfile.h"


/*all list data*/
object *objectList=NULL; /*object list, all word in memory*/
relocatable *relocatableList=NULL; /*relocatable list*/
table *labelList=NULL;  /*label list, all label in program*/
table *externList=NULL; /*extern label list*/
table *entryList=NULL; /*entry label list*/
table *externListwithAddress=NULL;
memmoryWord dataMemmory[MEMORY_SIZE]; /* array data all data save here*/

/* all global varibel*/
sysVar globalVar;
/*the variables:
-lineCounter 
-instructionCounter
-dataCounter
-indexCmd
-indexRegs	
-lastOperandType
-lastOperandCoding;
*/
char tmpLine[LINE_STR];
char *currenArg;/*pointer to current  word in line*/
labelRelocatable nameLabelRelo; /*save name label if the label is relocatable*/


/*define array with all command*/
cmd commandsList[]={/* all command*/
	{"mov" , 0 , 2,{immediate,direct,copy,directRegister},{OFF,direct,OFF,directRegister}},
	{"cmp" , 1 , 2,{immediate,direct,copy,directRegister},{immediate,direct,copy,directRegister}},
	{"add" , 2 , 2,{immediate,direct,copy,directRegister},{OFF,direct,OFF,directRegister},},
	{"sub" , 3 , 2,{immediate,direct,copy,directRegister},{OFF,direct,OFF,directRegister},},
	{"not" , 4 , 1,{OFF,OFF,OFF,OFF},{OFF,direct,OFF,directRegister}},
	{"clr" , 5 , 1,{OFF,OFF,OFF,OFF},{OFF,direct,OFF,directRegister}},
	{"lea" , 6 , 2,{OFF,direct,OFF,OFF},{OFF,direct,OFF,directRegister}},
	{"inc" , 7 , 1,{OFF,OFF,OFF,OFF},{OFF,direct,OFF,directRegister}},
	{"dec" , 8 , 1,{OFF,OFF,OFF,OFF},{OFF,direct,OFF,directRegister}},
	{"jmp" , 9 , 1,{OFF,OFF,OFF,OFF},{OFF,direct,copy,directRegister}},
	{"bne" , 10, 1,{OFF,OFF,OFF,OFF},{OFF,direct,copy,directRegister}},
	{"red" , 11, 1,{OFF,OFF,OFF,OFF},{OFF,direct,copy,directRegister}},
	{"prn" , 12, 1,{OFF,OFF,OFF,OFF},{immediate,direct,copy,directRegister}},
	{"jsr" , 13, 1,{OFF,OFF,OFF,OFF},{OFF,direct,OFF,OFF}},
	{"rts" , 14, 0,{OFF,OFF,OFF,OFF},{OFF,OFF,OFF,OFF}},
	{"stop", 15, 0,{OFF,OFF,OFF,OFF},{OFF,OFF,OFF,OFF}}
};
/*array with all registers */
registers regs[]={{"r0",0},{"r1",0},{"r2",0},{"r3",0},{"r4",0},{"r5",0},{"r6",0},{"r7",0}};

/*###################################*/
/*##############-MAIN-################*/
/*###################################*/

int main(int argc,char *argv[])
{
	int j;
	FILE *in=NULL;
	int lenLine=-1;
	char *nameFileIn;
	char *nameFile;
	int indexArgc=argc-1;
	char line[LINE_STR];	
	if(argc==1)
	{
		printf("unread file!\n");
		return 0;
	}
	else
	{
		printf("loading...\n");
		while(indexArgc>=1)
		{
			/*reset global varible*/
			resetSys(&globalVar);
			lenLine=strlen(argv[indexArgc]);
			/*name file*/
			nameFileIn=(char*)calloc((lenLine+4),sizeof(char));
			nameFile=(char*)calloc((lenLine),sizeof(char));
			strcpy(nameFileIn,argv[indexArgc]);
			/*name file for the functions write file*/
			strcpy(nameFile,argv[indexArgc]);
			/*stracat .as for open program assembles for read*/
			strcat(nameFileIn,".as");	
			in=fopen(nameFileIn,"r");	
			if(in==NULL)
				exit(1);
			do/*start assmbler*/
			{
				memset(line,0,LINE_STR);
				fgets(line,LINE_STR,in);
				assembler(line);
				globalVar.lineCounter++;
			}							
			while(line[0]!='\0');
			/*write files*/
			writeObjectFile(nameFile,objectList,dataMemmory,globalVar.instructionCounter,globalVar.dataCounter);
			writeLabelFile(nameFile,".ent",entryList,NULL);
			writeLabelFile(nameFile,".ext",externListwithAddress,relocatableList);
			/*free all data */
			freeLabelList();
			freeExternList();
			freeEntryList();
			freeRelocaTableList();
			FreeExternListwithAddress();
			for(j=0;j<globalVar.dataCounter;j++)/*free data (.data & .string*/
				dataMemmory[j].word=0;			
			printf("assembler complete %s!\n",nameFileIn);
			indexArgc--;
		}		
	}
	return 1;	
}


/*assembler line from file .as*/
void assembler(char *line)
{
	relocatable *relocatableTMP=NULL;
	boolean firstWord=false;
	boolean flagLabel=false;

	/*encoding operand  */
	int firstOperand=-1;
	int secendOperand=-1;


	char nameLabel[LABEL_STRLEN];
	int res;
	int numerEncoding=1;
	
	table *newLabel=NULL;
	table *newExtern=NULL;
	table *newEntry=NULL;
	table *tmpLabel=NULL;
	
	addressingType addressFirst=OFF;
	addressingType addressSecend=OFF;
	
	typeEncoding typeEncode=A;
	boolean flagSourceCmdOFF=false;
	boolean flagDestinatCmdOFF=false;
	strcpy(tmpLine,line);
	currenArg=strtok(line," \t\n");
	do{
		res=isArgument(currenArg,commandsList,regs,&globalVar.indexCmd,labelList,externList);
		switch(res)
		{
			case empty:
				return;
			case comment:
				return;
			case label:
			{
				flagLabel=true;
				if(firstWord==true)
				{
					printf("error define label must be first word!(liNE %d)\n",globalVar.lineCounter);
					return;
				}
				else
				{
					firstWord=true;
					newLabel=(table*)calloc(1,sizeof(table));
					memset(nameLabel,0,LABEL_STRLEN);
					strncpy(nameLabel, currenArg, strlen(currenArg)-1);
					strcpy(newLabel->name,nameLabel);
					newLabel->addressLabel=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					labelList=addNewNode(labelList,newLabel);					
					/*update label address if the label is entry*/
					if((tmpLabel=searchByName(entryList,nameLabel))!=NULL)
						tmpLabel->addressLabel=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					/*get next word*/	
					currenArg=strtok(NULL," \t\n");
					break;
				}
			}
			case instructString:
			case instructData:
			{
				/*if flag is true  ad the newLabel to label list with type label-> data*/
				if(flagLabel==true)
				{
					strcpy(newLabel->flag,"data");
					/*check if ther is label in relocatable and update the address*/
					if(updateAddress(nameLabel,newLabel->addressLabel)==true)
					{
						/*update object realocatable*/
						updateEncoding(relocatableList,nameLabel);
					}
				}
				saveData(res);
				/*get next word*/	
				currenArg=strtok(NULL,"\t\n");
				break;
			}
			case command:
			{
				/*if flag is true, ad the newLabel to label list with type label-> code */
				if(flagLabel==true)
				{
					strcpy(newLabel->flag,"code");
					/*check if label in relocatable and update the address*/
					if(updateAddress(nameLabel,newLabel->addressLabel)==true)
						updateEncoding(relocatableList,nameLabel);/*update object realocatable*/			
				}
				/*check if in current cmd the operand type is OFF*/
				flagDestinatCmdOFF=destinatOffIncmd(commandsList,globalVar.indexCmd);
				flagSourceCmdOFF=sourceOffIncmd(commandsList,globalVar.indexCmd);
				/*how many times encode*/
				numerEncoding=numEncoding(currenArg);
				if(numerEncoding<0 || numerEncoding>2)
				{
					printf("error num==%d emcodinf is invalid,is must be 1 or 2! (LINE %d)\n",numerEncoding,globalVar.lineCounter);
					return ;
				}
				/*get next word*/	
				currenArg=strtok(NULL," ,\t\n");
					
				/*###################################*/
				/*############-source-##############*/
				/*#################################*/				
				if(currenArg!=NULL && flagSourceCmdOFF==false)
				{	
					/*if the operand source is label and a label not exist in list*/
					if(isOperandLabel(currenArg,commandsList,regs)==true)
					{
						/*if label is extern, add address to label list*/
						if(searchByName(externList,currenArg)!=NULL)
						{
							typeEncode=E; /*external*/		
							memset(nameLabelRelo.source,0,LABEL_STRLEN);				
							strcpy(nameLabelRelo.source,currenArg);
						}
						else if(searchByName(labelList,currenArg)==NULL)
						{							
							typeEncode=R; /*relocatanle*/
							memset(nameLabelRelo.source,0,LABEL_STRLEN);
							strncpy(nameLabelRelo.source, currenArg, strlen(currenArg));
													
						}
						else 	
							typeEncode=A; /*absolute*/
																	
						addressFirst=direct;
						firstOperand=typeEncode;
						globalVar.lastOperandType=addressFirst;
						globalVar.lastOperandCoding=typeEncode;	
						/*copy name label if source is $$ and $$ is E*/
					}
					else
					{
						typeEncode=A; /*absolute*/
						/*####################################*/
						/*get the addressing of source operand*/
						/*####################################*/
						addressFirst=addresing(currenArg,commandsList,regs,&firstOperand,globalVar.lastOperandType,operandSource);
						globalVar.lastOperandType=addressFirst;
						globalVar.lastOperandCoding=firstOperand;
					}
				}
				else /*if source off*/
					addressFirst=OFF;
				/*end source*/	
					
				/*if in current cmd source is not off*/	
				if(flagSourceCmdOFF==false)				
					currenArg=strtok(NULL," ,\t\n");
				/*###################################*/
				/*############-direct-##############*/
				/*#################################*/
				if(currenArg!=NULL && flagDestinatCmdOFF==false)		
				{
					/*if the operand direct is label and a label not exist in list*/
					if(isOperandLabel(currenArg,commandsList,regs)==true)
					{
						/*if label is extern, add address to label list*/
						if(searchByName(externList,currenArg)!=NULL)
						{
							typeEncode=E; /*external*/					
							memset(nameLabelRelo.destinat,0,LABEL_STRLEN);				
							strcpy(nameLabelRelo.destinat,currenArg);
						}
						else if(searchByName(labelList,currenArg)==NULL)
						{
							typeEncode=R; /*relocatanle*/
							memset(nameLabelRelo.destinat,0,LABEL_STRLEN);
							strncpy(nameLabelRelo.destinat, currenArg, strlen(currenArg));							
						}
						else 	
							typeEncode=A; /*absolute*/
							
						addressSecend=direct;
						secendOperand=typeEncode;							
						
						if(addressFirst==OFF && addressSecend!=OFF)
						{
							globalVar.lastOperandType=addressSecend;
							globalVar.lastOperandCoding=typeEncode;
							/*copy name label if direct is $$ and $$ is E*/
							strcpy(nameLabelRelo.source,nameLabelRelo.destinat);							
						}																	
					}	
					else
					{
						typeEncode=A; /*absolute*/
						/*####################################*/
						/*get the addressing of destinat operand*/
						/*####################################*/
						addressSecend=addresing(currenArg,commandsList,regs,&secendOperand,globalVar.lastOperandType,operandDestinat);
						
						if(addressFirst==OFF && addressSecend!=OFF)
						{
							globalVar.lastOperandType=addressSecend;
							globalVar.lastOperandCoding=secendOperand;							
						}
					}
				}
				else /*if destinat off*/
					addressSecend=OFF;
				/*end destinat*/		
					
				/*emcoding the operation*/
				if(encodingOperation(globalVar.indexCmd,addressFirst,addressSecend,firstOperand,secendOperand,numerEncoding,nameLabelRelo)==false)
				{
					printf("error in line %d\n",globalVar.lineCounter);
				}
				/*get next word*/	
				currenArg=strtok(NULL," \t\n");
				break;
			}
			case instructEntry:
			{
				currenArg=strtok(NULL," \t\n");
				newEntry=(table*)calloc(1,sizeof(table));
				memset(nameLabel,0,LABEL_STRLEN);
				strncpy(nameLabel, currenArg, strlen(currenArg));
				/*update label address if the label is entry*/
				if((tmpLabel=searchByName(labelList,nameLabel))!=NULL)
					newEntry->addressLabel=tmpLabel->addressLabel;
				else
					newEntry->addressLabel=0;	
									
				strcpy(newEntry->name,nameLabel);
				entryList=addNewNode(entryList,newEntry);
				/*get next word*/	
				currenArg=strtok(NULL," \t\n");
				/*reset name label for next label*/
				memset(nameLabel,0,LABEL_STRLEN);
				break;
			}
			case instructExtern:
			{
				currenArg=strtok(NULL," \t\n");
				newExtern=(table*)calloc(1,sizeof(table));
				memset(nameLabel,0,LABEL_STRLEN);
				strncpy(nameLabel, currenArg, strlen(currenArg));
				strcpy(newExtern->name,nameLabel);
				newExtern->addressLabel=0;
				externList=addNewNode(externList,newExtern);
				/*check if label in relocatable and update the address*/
				if(updateAddress(nameLabel,E)==true)
					updateEncoding(relocatableList,nameLabel);/*update object realocatable*/
				/*get next word*/	
				currenArg=strtok(NULL," \t\n");
				/*reset name label for next label*/
				memset(nameLabel,0,LABEL_STRLEN);
				break;
			}
			case unknow:/*if the wors is unknow for the assembler*/
			{
				printf("error unknow argument! %s (LINE %d)\n",currenArg,globalVar.lineCounter);
				/*get next word*/	
				currenArg=strtok(NULL," \t\n");
				break;
			}
		}
		firstWord=true;
	}while(currenArg!=NULL);
}
/*encoding the operatiom and save to object list*/
boolean encodingOperation(int indexCmd,addressingType source,addressingType destinat,int encodeSource,int encodeDestinat,int numEncoding,labelRelocatable nameLabelRelo)
{
	int i;
	object *newOperation=NULL;
	object *newDestinat=NULL;
	object *newSource=NULL;
	table *newExtern=NULL;/*if operand is extern label, update address label in extern list*/
	relocatable *newRelocatable=NULL;
	if(checkOperation(indexCmd,destinat,source)==false)
		return false;
		
	for(i=0;i<numEncoding;i++)
	{			
		newOperation=(object*)calloc(1,sizeof(object));
		if(newOperation==NULL)
			exit(1);	
		newOperation->address=globalVar.instructionCounter+globalVar.dataCounter+BASE_LINE;
		newOperation->machineCode.word=commandsList[indexCmd].numOperand<<SHIFT_TO_GROUP; /*group*/
		newOperation->machineCode.word|=(commandsList[indexCmd].codeCmd)<<SHIFT_TO_CMD; /*cmd*/
		if(source!=OFF)
			newOperation->machineCode.word|=source<<SHIFT_TO_SOURCE;
		if(destinat!=OFF)
			newOperation->machineCode.word|=destinat<<SHIFT_TO_DESTINAT;
		newOperation->machineCode.word|=A; 
		
		objectList=addNewObject(objectList,newOperation); /*add to object list*/
		globalVar.instructionCounter++;
		
		/*if operand soutce and operand destinat is registers*/
		if(source==directRegister && destinat==directRegister)
		{
			newSource=(object*)calloc(1,sizeof(object));
			newSource->machineCode.word|=encodeSource<<SHIFT_TO_SOURCE_REGS;
			newSource->machineCode.word|=encodeDestinat<<SHIFT_TO_DESTINAT_REGS;
			newSource->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
			objectList=addNewObject(objectList,newSource);
			globalVar.instructionCounter++;
		}
		else
		{
			/*encoding source operand*/
			if(source!=OFF)
			{
				newSource=(object*)calloc(1,sizeof(object));
				if(newSource==NULL)
					exit(1);
				if(source==directRegister)
				{
					newSource->machineCode.word|=encodeSource<<SHIFT_TO_SOURCE_REGS;
					newSource->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
				}	
				/*encoding two registers in one word*/
				else if(source==direct && (encodeSource & R)==R)
				{
					newRelocatable=(relocatable*)calloc(1,sizeof(relocatable));
					newRelocatable->addressLabel=0;
					newRelocatable->addressObject=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					
					memset(newRelocatable->name,0,LABEL_STRLEN);
					strcpy(newRelocatable->name,nameLabelRelo.source);
					
					newSource->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newSource->machineCode.word=encodeSource;
					relocatableList=addNewRelocatable(relocatableList,newRelocatable);					
				}
				else if(source==direct && (encodeSource & E)==E)
				{
					newSource->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newSource->machineCode.word=encodeSource;
					/*add to extern listwith address*/
					newExtern=(table*)calloc(1,sizeof(table));
					strcpy(newExtern->name,nameLabelRelo.source);
					newExtern->addressLabel=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					externListwithAddress=addNewNode(externListwithAddress,newExtern);									
				}
				else
				{
					newSource->machineCode.word|=encodeSource<<SHIFT_ARE;
					newSource->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
				}
				objectList=addNewObject(objectList,newSource);
				globalVar.instructionCounter++;
				
			}
			/*encoding destinat operand*/		
			if(destinat!=OFF)
			{
				newDestinat=(object*)calloc(1,sizeof(object));
				if(newDestinat==NULL)
					exit(1);
				if(destinat==direct && (encodeDestinat & R)==R)
				{
					newRelocatable=(relocatable*)calloc(1,sizeof(relocatable));
					newRelocatable->addressLabel=0;
					newRelocatable->addressObject=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					
					memset(newRelocatable->name,0,LABEL_STRLEN);
					if(source!=OFF)
						strcpy(newRelocatable->name,nameLabelRelo.destinat);
					else
						strcpy(newRelocatable->name,nameLabelRelo.source);		
					
					newDestinat->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newDestinat->machineCode.word=encodeDestinat;
					relocatableList=addNewRelocatable(relocatableList,newRelocatable);
				}
				else if(destinat==direct && (encodeDestinat & E)==E)
				{
					newDestinat->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newDestinat->machineCode.word=encodeDestinat;
					/*add to extern listwith address*/
					newExtern=(table*)calloc(1,sizeof(table));
					strcpy(newExtern->name,nameLabelRelo.destinat);
					newExtern->addressLabel=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					externListwithAddress=addNewNode(externListwithAddress,newExtern);										
				}				
				else if(source==OFF && destinat!=OFF)
				{
					newDestinat->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newDestinat->machineCode.word|=encodeDestinat<<SHIFT_ARE;

				}
				else
				{
					newDestinat->address=globalVar.instructionCounter+BASE_LINE+globalVar.dataCounter;
					newDestinat->machineCode.word|=encodeDestinat<<SHIFT_ARE;
				}
				objectList=addNewObject(objectList,newDestinat);
				globalVar.instructionCounter++;
			}
		}
	}
	
	return true;
}
/*check if the operetions is valid*/
boolean checkOperation(int indexCmd,addressingType destinat,addressingType source)
{
	int numOperand=0;
	if(destinat!=OFF)
		numOperand++;
	if(source!=OFF)
		numOperand++;
	/*check if the num of operand is corret exist to command*/
	if(numOperand < commandsList[indexCmd].numOperand)
	{
		printf("missing operand in command %s must be %d operand! (LINE %d)\n",commandsList[indexCmd].nameCmd,commandsList[indexCmd].numOperand,globalVar.lineCounter);
		return false;
	}
	else if(numOperand > commandsList[indexCmd].numOperand)
	{
		printf("overly operand in command %s must be %d operands!(LINE %d)\n",commandsList[indexCmd].nameCmd,commandsList[indexCmd].numOperand,globalVar.lineCounter);
		return false;
	}
	/*check if method for commands is exist*/
	if(destinat!=OFF)
	{
		if(validAddress(indexCmd,destinat,operandDestinat)==false)
		{
			printf("the opperand is not exist for this command: %s (LINE %d)\n",commandsList[indexCmd].nameCmd,globalVar.lineCounter);
			return false;
		}
	}
	if(source!=OFF)
	{
		if(validAddress(indexCmd,source,operandSource)==false)
		{
			printf("the opperand is not exist for this command: %s (LINE %d)\n",commandsList[indexCmd].nameCmd,globalVar.lineCounter);
			return false;
		}
	}
	return true;
}
/*check if method for commands is exist*/
boolean validAddress(int indexCmd,addressingType address,operandType operand)
{
	int i;
	if(operand==operandDestinat)
	{
		for(i=0;i<METHOD_ADDRESSING;i++)
		{
			if(commandsList[indexCmd].Destinat[i]==address)
				return true;
		}
	}
	else if(operand==operandSource)
	{
		for(i=0;i<METHOD_ADDRESSING;i++)
		{
			if(commandsList[indexCmd].Source[i]==address)
				return true;
		}
	}
	return false;
}
/*save data .string or .data  to memmory*/
boolean saveData(int type)
{
	int data,res;
	int indexStart;
	if(type==instructData)
	{
		currenArg=strtok(NULL," ,\t\n");
		if(isData(tmpLine,&indexStart)==false)
			return false;
		while(currenArg!=NULL)
		{
			res=sscanf(currenArg,"%d",&data);
			if(res!=1)
			{
				printf("ERROR\n");
				return false;
			}
			dataMemmory[globalVar.dataCounter].word=MASK_BITS & data;
			globalVar.dataCounter++;
			currenArg=strtok(NULL," ,\t\n");
		}
		return true;
	}
	if(type==instructString)
	{
		currenArg=strtok(NULL," \"");
		if(isString(tmpLine,&indexStart)==true)
		{
			while(tmpLine[indexStart]!='"')
			{
				dataMemmory[globalVar.dataCounter].word=tmpLine[indexStart];
				globalVar.dataCounter++;
				indexStart++;
			}
			dataMemmory[globalVar.dataCounter].word=0;
			globalVar.dataCounter++;
			return true;
		}
		else
		{
			printf("invalid define string must be in start '\"' and in end '\"'LINE %d\n",globalVar.lineCounter);
			/*open protocol error*/
			return false;
		}
	}
	return false;
}
/*return the type addressin for current operand*/
addressingType addresing(char *str,cmd commandList[],registers regs[], int *op,operandType lastOperandType,operandType currentOperandType)
{
	table *tmp=NULL;
	int num=-1;
	int i=0;
	int posNum=0;
	char saveNum[5];
	if((tmp=searchByName(labelList,str))!=NULL)
	{
		(*op)=tmp->addressLabel;
		return direct;
	}
	else if(isRegister(regs,str,&globalVar.indexRegs)==true)
	{	
		(*op)=globalVar.indexRegs;
		return directRegister;
	}
	else if(strcmp(str,"$$")==0)
	{
		/*check if the operand is define correct*/
		if(validAddress(globalVar.indexCmd,copy,currentOperandType)==false)
		{
			printf("error invalid addressing for this cmd! (LINE%d)\n",globalVar.lineCounter);
			return false;
		}
		if(lastOperandType==OFF)
		{
			printf("error the addressing $$ cant to be first! (LINE %d)\n",globalVar.lineCounter);
			return OFF;
		}
		else
		{
			(*op)=globalVar.lastOperandCoding;
			return  lastOperandType;
		}
	}
	else if(strncmp(str,"#",1)==0)
	{
		while(str[posNum]!='#');
		posNum++;
		if(str[posNum]==' ')
			while(str[posNum]!=' ');
		while((str[posNum]=='+' && i==0) || (str[posNum]=='-' && i==0) || isdigit(str[posNum]) || str[posNum]=='.')
		{
			if(str[posNum]=='.')
			{
				printf("error the num must be integer!\n");
				return OFF;
			}			
			saveNum[i]=str[posNum];
			i++;
			posNum++;
		}
		sscanf(saveNum,"%d",&num);
		(*op)=num;
		/*check if after '#' there valid number (integer)*/
		return immediate;
	}
	else
	{
		(*op)=-1;
		printf("error addressing!(LINE %d)\n",globalVar.lineCounter);
		return OFF;
	}
}
/*count sveral times  encoding the operations*/
int numEncoding(char *cmd)
{
	int num;
	int i,j;
	char tmp[3];
	for(i=0,j=0;cmd[i]!='\0';i++)
	{
		if(isdigit(cmd[i]))
		{
			tmp[j]=cmd[i];
			j++;
		}
	}
	sscanf(tmp,"%d",&num);
	return num;
}
/*update all address in object list*/
void updateEncoding(relocatable *head,char *name)
{
	object* objectTMP=NULL;
	for(;head!=NULL;head=head->next)
	{
		if(strcmp(name,head->name)==0)
		{
			if((objectTMP=searchByAddress(objectList,head->addressObject))!=NULL)
				if(head->addressLabel==E)/*if define etern label later*/
					objectTMP->machineCode.word=E;
				else	
					objectTMP->machineCode.word|=head->addressLabel<<SHIFT_ARE;
		}		
	}	
}
/*updte all address in relocatable list  label if exist ==true*/
boolean updateAddress(char *name,int address)
{
	int flag=false;
	relocatable *current=relocatableList; 
	for(;current!=NULL;current=current->next)
	{
		if(strcmp(name,current->name)==0)
		{
			current->addressLabel=address;
			flag=true;
		}
	}	
	return flag;	
}

/*reset all global varble*/
void resetSys(sysVar *var)
{
	var->lineCounter=0;
	var->instructionCounter=0;
	var->dataCounter=0;
	var->indexCmd=-1; /*index to  current command */
	var->indexRegs=-1; /*index to  current register */	
	var->lastOperandType=OFF;/*for addressing $$*/
	var->lastOperandCoding=-1; /*for addressing $$*/	
	memset(nameLabelRelo.destinat,0,LABEL_STRLEN);
	memset(nameLabelRelo.source,0,LABEL_STRLEN);
}
