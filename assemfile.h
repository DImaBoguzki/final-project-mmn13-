#ifndef ASSEMFILE_H
#define ASSEMFILE_H
#include <stdio.h>
void writeObjectFile(char *nameFile, object* head,memmoryWord data[],int IC,int DC);
void writeEntryFile(char *nameFile, table* head);
void writeLabelFile(char *nameFile, char *typeFile, table* headExtern, relocatable *relocatableList);
#endif
