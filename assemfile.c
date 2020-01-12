#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "assemtools.h"
#include "assembler.h"

/*write all word of memmory in file*/
void writeObjectFile(char *nameFile, object* head,memmoryWord data[],int IC,int DC)
{
	int i;
	FILE *objectFile=NULL;
	char newName[(strlen(nameFile)+4)];
	strcpy(newName,nameFile);
	strcat(newName,".ob");
	objectFile=fopen(newName,"w");	
	if(objectFile==NULL)
		exit(1);			
	fprintf(objectFile,"\t\t%d  %d\n",ConvertToMachineCodeBase4(IC),ConvertToMachineCodeBase4(DC));
	while(head!=NULL)
	{
		fprintf(objectFile,"%d\t\t%.6d\n",ConvertToMachineCodeBase4(head->address),ConvertToMachineCodeBase4(head->machineCode.word));	
		head=head->next;
	}
	if(DC>0)
	{
		for(i=0;i<DC;i++)
			fprintf(objectFile,"%d\t\t%.6d\n",ConvertToMachineCodeBase4((IC+i+BASE_LINE)),ConvertToMachineCodeBase4(data[i].word));
	}
	fclose(objectFile);
}
void writeLabelFile(char *nameFile, char *typeFile, table* headExtern, relocatable *relocatableList)
{
	FILE *newFile=NULL;
	char newName[(strlen(nameFile)+5)];
	strcpy(newName,nameFile);
	strcat(newName,typeFile);
	newFile=fopen(newName,"w");	
	if(newFile==NULL)
		exit(1);
	while(headExtern!=NULL)
	{
		if(headExtern->addressLabel!=0)
			fprintf(newFile,"%s\t\t%d\n",headExtern->name,ConvertToMachineCodeBase4(headExtern->addressLabel));	
		headExtern=headExtern->next;		
	}
	/*if extern label efine in mid program assembler, maybe there address of extern in relocat able List*/
	while(relocatableList!=NULL)
	{
		if(relocatableList->addressLabel==E)
			fprintf(newFile,"%s\t\t%d\n",relocatableList->name,ConvertToMachineCodeBase4(relocatableList->addressObject));
		relocatableList=relocatableList->next;			
	}
	fclose(newFile);
}

