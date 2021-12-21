/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
} 

static ScopeList scopes[SIZE];
static int num = -1;
static ScopeList stack[SIZE];
static int top = -1;
static int loc_stack[SIZE];
static loc_top = -1;
static FuncList funcTable[50];
static int fNum = -1;

void st_init(void)
{ ScopeList root = (ScopeList) malloc(sizeof(struct ScopeListRec));
  root->name = "global";
  root->parent = NULL;
  root->depth = 0;
  
  scopes[++num] = root; 
  // insert input, output
  st_push(root);
  
  st_insert("input",Integer,Func,0,addLoc());
  st_newScope("input");
  st_pop();
  st_insert("output",Void,Func,0,addLoc());
  st_newScope("output");
  st_insert("value",Integer,Arg,0,addLoc());
  st_pop();
}

void st_newScope(char * name)
{ ScopeList s = (ScopeList) malloc(sizeof(struct ScopeListRec));
  s->name = name;
  s->parent = stack[top];
  s->depth = s->parent->depth + 1;
  st_push(s);
  scopes[++num] = s;
}

void st_push(ScopeList s)
{ stack[++top] = s;
  loc_stack[++loc_top] = -1;
  //printf("scope : %s, depth : %d\n", stack[top]->name, stack[top]->depth);
}

void st_pop(void)
{ top--;
  popLoc();
}

ScopeList st_get_top(void)
{ return stack[top];
}

int addLoc(void)
{ loc_stack[loc_top] += 1;
  return loc_stack[loc_top];
}

void popLoc(void)
{ loc_top--;
}

void addLine(char * name, int line)
{ BucketList l = st_lookup(name); 
  
  if(l != -1)
  { LineList t = l->lines;
    while(t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = line;
    t->next->next = NULL;
  }
}

int st_getSize(void)
{ return num;
}

ScopeList st_getTable(int index)
{ return scopes[index];
}

ScopeList st_getFunc(char * name)
{ int i;
  for(i=0;i<=num;++i)
  { ScopeList s = scopes[i];
    if((s->depth == 1) && (strcmp(s->name, name) == 0))
      return s;
  }
  return -1;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, ExpType type, SymKind kind, int lineno, int loc )
{ int h = hash(name);
  BucketList l = stack[top]->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->type = type;
    l->kind = kind;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = stack[top]->hashTable[h];
    stack[top]->hashTable[h] = l; }
  else /* found in table, so just add line number */
  { 
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */

BucketList st_lookup( char * name )
{ ScopeList s = stack[top];
  BucketList b = -1;
  while((s != NULL) && ((b = st_lookup_excluding_parent(s, name)) == -1))
    s = s->parent;
  
  return b;
}

BucketList st_lookup_excluding_parent (ScopeList s, char * name)
{ int h = hash(name);
  BucketList l = s->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printAllSymTab(FILE * listing)
{ int i,j;
  for (i=0;i<=num;++i)
  { fprintf(listing, "[Scope : %s (depth = %d)]\n", scopes[i]->name, scopes[i]->depth);
    fprintf(listing, "Symbol Name  Symbol Type  Symbol Kind  Location  Line Numbers\n");
    fprintf(listing, "-----------  -----------  -----------  --------  ------------\n");
    for (j=0;j<SIZE;++j)
    { if(scopes[i]->hashTable[j] != NULL)
      { BucketList l = scopes[i]->hashTable[j];
	while (l != NULL)
        { LineList t = l->lines;
	  fprintf(listing,"%-12s ",l->name);

	  switch(l->type)
	  { case Integer:
	      fprintf(listing,"%-12s ","int ");
	      break;
	    case Void:
	      fprintf(listing,"%-12s ","void ");
	      break;
	    case IntArr:
	      fprintf(listing,"%-12s ","int[] ");
	      break;
	  }

	  switch(l->kind)
	  { case Func:
	      fprintf(listing,"%-12s ","function");
	      break;
	    case Var:
	      fprintf(listing,"%-12s ","variable");
	      break;
	    case Arg:
	      fprintf(listing,"%-12s ","argument");
	      break;
	  }

	  fprintf(listing," %-9d",l->memloc);
	  while (t != NULL)
	  { fprintf(listing,"%2d ",t->lineno);
	    t = t->next;
	  }
	  fprintf(listing,"\n");
	  l = l->next;
	}
      }
    }
    fprintf(listing, "\n");
  }
}
/* printSymTab */
