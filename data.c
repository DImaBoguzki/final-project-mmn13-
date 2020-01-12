#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data.h"
#include "assembler.h"
#include "assemtools.h"

/*extern from assembler.c for free functions*/
extern object *objectList; /*object list */
extern relocatable *relocatableList; /*relocatable list*/
extern table *labelList;  /*label list, all label in program*/
extern table *externList; /*extern label list*/
extern table *entryList; /*entry label list*/
extern table *externListwithAddress; /*extern label list*/

table *addNewNode(table *head, table *newLable)
{
	table *tmp=head;
	newLable->next=NULL;
	if(newLable==NULL)
		return NULL;
	if(head!=NULL)	
	{
		while(true)
		{
			if(tmp->next==NULL)
			{
				tmp->next=newLable;
				break;
			}
			tmp=tmp->next;
		}
	}
	else
		head=newLable;
	return head;
}

table *searchByName(table *head, char *name)
{
	table *current=head;
	for(;current!=NULL;current=current->next)
	{
		if(strcmp(name,current->name)==0)
			return current;
	}
	return NULL;
}

object *searchByAddress(object *head, int address)
{
	object *current=head;
	for(;current!=NULL;current=current->next)
	{
		if(current->address==address)
			return current;
	}
	return NULL;		
}

object *addNewObject(object *head, object *newObject)
{
	object *tmp=head;
	newObject->next=NULL;
	if(newObject==NULL)
		return NULL;
	if(head!=NULL)	
	{
		while(true)
		{
			if(tmp->next==NULL)
			{
				tmp->next=newObject;
				break;
			}
			tmp=tmp->next;
		}
	}
	else
		head=newObject;
	return head;
}
relocatable *addNewRelocatable(relocatable *head, relocatable *newRelocatable)
{
	relocatable *tmp=head;
	newRelocatable->next=NULL;
	if(newRelocatable==NULL)
		return NULL;
	if(head!=NULL)	
	{
		while(true)
		{
			if(tmp->next==NULL)
			{
				tmp->next=newRelocatable;
				break;
			}
			tmp=tmp->next;
		}
	}
	else
		head=newRelocatable;
	return head;
}

void freeObjectList()
{
	object* tmp=objectList;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		objectList=objectList->next;
		free(tmp);
		tmp=objectList;
	}
}
void freeLabelList()
{
	table* tmp=labelList;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		labelList=labelList->next;
		free(tmp);
		tmp=labelList;
	}
}
void freeExternList()
{
	table* tmp=externList;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		externList=externList->next;
		free(tmp);
		tmp=externList;
	}
}
void freeEntryList()
{
	table* tmp=entryList;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		entryList=entryList->next;
		free(tmp);
		tmp=entryList;
	}
}
void freeRelocaTableList()
{
	relocatable* tmp=relocatableList;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		relocatableList=relocatableList->next;
		free(tmp);
		tmp=relocatableList;
	}	
}
void FreeExternListwithAddress()
{
	table* tmp=externListwithAddress;
	if(tmp==NULL)
		return;
	while(tmp!=NULL)
	{
		externListwithAddress=externListwithAddress->next;
		free(tmp);
		tmp=externListwithAddress;
	}	
}
