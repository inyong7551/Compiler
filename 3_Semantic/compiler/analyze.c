/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#define SIZE 50
#include "globals.h"
#include "symtab.h"
#include "analyze.h"

static int flag = 0;
static int idx = 3;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else
  { return;
  }
}

/* flushProc is a pop procedure to
 * stack when compound statement node
 * is done
 */
static void flushProc(TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
    { switch (t->kind.stmt)
        case CompoundK:
	  st_pop();
	  break;
	default:
	  break;
    }
    break;
  }
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case DecK:
      switch (t->kind.dec)
      { case FunK:
	  if (st_lookup(t->attr.name) == -1)
	  { //printf("Function Dec : %s\n",t->attr.name);
	    st_insert(t->attr.name,t->type,Func,t->lineno,addLoc());
	    st_newScope(t->attr.name);
	    flag = 1;
	  }
	  else
	    scopeError(t, "Already Defined Function");
	  break;
	case VarK:
	  if (st_lookup_excluding_parent(st_get_top(), t->attr.name) == -1)
          { //printf("Variable Dec : %s\n",t->attr.name);
	    st_insert(t->attr.name,t->type,Var,t->lineno,addLoc());
	  }
	  else
	    scopeError(t, "Already Defined Variable");
	  break;
	default:
	  break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      {	case CompoundK:
	{ //printf("CompoundK\n");
	  ScopeList t = st_get_top();
	  if(flag == 0)
	    st_newScope(t->name);
	  else flag = 0;
          break;
	}
	default:
	  break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case CallK:
	case VariableK:
	  if(st_lookup(t->attr.name) == -1)
	    scopeError(t, "Not Defined Symbol");
	  else
	    addLine(t->attr.name,t->lineno);
	  break;
	default:
	  break;
       }
       break;
    case ParamK:
      if(t->type == Integer)
      { st_insert(t->attr.name,t->type,Arg,t->lineno,addLoc());
        //printf("Parameter Declaration : %s\n", t->attr.name);
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ st_init();
  traverse(syntaxTree,insertNode,flushProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printAllSymTab(listing);
  }
}

static void scopeError(TreeNode * t, char * message)
{ fprintf(listing, "Scope error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ ScopeList s = st_getTable(idx);
  switch (t->nodekind)
  { case DecK:
      switch (t->kind.dec)
      { case VarK:
	 if(t->type == Void)
	   typeError(t, "Void Variable Error");
	 break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case AssignK:
	  if(t->child[1]->type != Integer)
	    typeError(t, "assignment of non-integer value");
	  else
 	    t->type = t->child[0]->type;
	  break;
	case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
	  { 
            typeError(t,"Op applied to non-integer"); }
          t->type = Integer;
          break;
	case VariableK:
	{ BucketList l = -1;
	  // upper tracking
	  while((s != NULL) && ((l = st_lookup_excluding_parent(s,t->attr.name)) == -1))
	    s = s->parent;

	  if (l != -1)
	    t->type = l->type;
	  else typeError(t, "Undefined Variable Error"); // Undefined Variable Error
	  // Array Indexing Error
	  if (t->child[0] != NULL)
	  { if(t->child[0]->type != Integer)
	      typeError(t, "Array Indexing Error");
	    t->type = Integer;
	  }
	  break;
	}
	case CallK:
	{ s = st_getTable(0);
	  BucketList l = st_lookup_excluding_parent(s,t->attr.name);
	  if(l != -1)
	    t->type = l->type;
	  else
	  { scopeError(t,"Undefined Function Error"); // Undefined Function Error
	    break; }

	  TreeNode * arg = t->child[0];
	  ScopeList f = st_getFunc(t->attr.name);
	  if(callCheck(arg, f) == -1) // Invalid Function Call Error
	    typeError(t, "Invalid Function Call Error");
	}
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
	case IfelseK:
          if (t->child[0]->type == Void)
            typeError(t->child[0],"if condition is void");
          break;
        case WhileK:
          if (t->child[0]->type == Void)
            typeError(t->child[1],"while condition is void");
          break;
	case CompoundK:
	  // go to next scope
	  idx++;
	  break;
	case ReturnK:
	{ char * funName = s->name;
	  BucketList l = st_lookup_excluding_parent(st_getTable(0), s->name);
	  if(l->type != t->child[0]->type)
	    typeError(t, "Invalid Return Type Error"); 
	  break;
	}
        default:
          break;
      }
      break;
    default:
      break;

  }
}

int callCheck(TreeNode * arg, ScopeList f)
{ int cnt = 0; // the number of table's argument
  int cnt_ = 0; // the number of call's argument
  int i,j;
  TreeNode * t = arg;
  for(i=0;i<SIZE;++i)
  { if(f->hashTable[i] != NULL) 
    { BucketList l = f->hashTable[i];
      if(l->kind == Arg)
      { cnt++;
	for(j=0;j<l->memloc;++j)
	  t = t->sibling;
	if(t->type != l->type)
	  return -1;
      }
      t = arg;
    }
  }
  t = arg;  

  while(t != NULL)
  { cnt_++;
    t = t->sibling;
  }

  if(cnt != cnt_) // when the number of call's args != the number of table's args
    return -1;

  return 0;
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
