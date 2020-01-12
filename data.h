#ifndef DATA_H
#define DATA_H

#define MEMORY_SIZE 1000
#define BASE_LINE 100
#define MAX_BITS 12
#define REGISTER 8
#define LABEL_STRLEN 30
#define LINE_STR 80
#define COMMAND_NUM 15


typedef struct _table table;
struct _table
{
	char name[LABEL_STRLEN];
	int addressLabel; /*number of line in dec*/
	char flag[5]; /*code or data*/
	table *next; /*pointer to next node*/
};

typedef struct _memmoryWord
{
	int word : MAX_BITS; /*12 bits*/	
}memmoryWord;


/*machine command save binnary code.*/ 
typedef struct _object object;
struct _object
{
	int address;
	memmoryWord machineCode; /*12 bits*/
	
	object * next;
};
typedef struct _relocatable relocatable;
struct _relocatable 
{
	char name[LABEL_STRLEN];
	int addressObject;
	int addressLabel;
	relocatable *next;
};

table *addNewNode(table *head,table *newLabel);
relocatable *addNewRelocatable(relocatable *head, relocatable *newRelocatable);
table *searchByName(table *head, char *name);
object *searchByAddress(object *head, int address);
object *addNewObject(object *head, object *newHead);
void freeObjectList();
void freeLabelList();
void freeExternList();
void freeEntryList();
void freeRelocaTableList();
void FreeExternListwithAddress();


#endif

