
# [Project 1 Lexer]


**C-Minus Scanner Implementation**

1. Implementation Method 1 : C code (scan.c) - ?쐁minus_cimpl??
- Recognize token by **DFA**
1. Implementation Method 2 : Lex(Flex) (cminus.l) - ?쐁minus_lex??
- Specify lexical patterns by **Regular expression**

[Compilation method and environment]

VirtualBox Linux : Ubuntu 18.04

# Method 1 : C code


 목표는 C-Minus의 Lexical convention을 accept하는 DFA를 구현하는 것이다. 직접 구현해야 하는 부분은 multi-character symbol, ID였다.

```c
typedef enum
	{START, INCOMMENT, INCOMPARE, INNUM, INID, DONE}
	StateType;i
```

```c
case INCOMPARE :
{
	int before = tokenString[tokenStringIndex-1];
	if(before != "!"){
		state = DONE; // <, >, = 만으로도 DONE state에 도달할 수 있음
		switch(before){
			case '<':
				if(c == '='){
					currentToken = LE;
				} else {
					ungetNextChar();
					save = FALSE;
					currentToken = LT;
				}
				break;
			case ...
		}
	} else {
		state = DONE;
		if(c == '=') currentToken = NE;
		else {
			ungetNextChar();
			save = FALSE;
			currentToken = ERROR; // ! 만으로는 Symbol을 완성할 수 없으므로 Error
		}
	}
	break;
}
```

- INCOMPARE : START에서 >, <, !, = 를 입력 받아 이동한 State
- INCOMMENT : START에서 / 를 입력 받은 후 lineBuf의 다음 Character가 * 일때 이동한 State 
    - 실제 코드를 보면 START에서 lineBuf[linepose]를 통해 다음 Character를 보고, INCOMMENT에서 getNextChar()를 다시 호출하여 다음 Character를 보는 것을 알 수 있는데 이는 Comment 특성 상 실제 코드에 문법적인 것을 의미하지 않기 때문에 그렇게 설계하였다.
- ID의 경우 letter(letter | digit)* 이다. 이 경우 INID에서 if statement의 조건을
    - !isalpha(c) && !isdigit(c)로 구현하였다.




# Method 2 : Lex (Flex)


Tiny.l 을 수정하여 C-Minus Convention에 대한 Lexical Analyzer를 구현하는 것이 목표이다. Rule Section에서 각 Convention에 맞는 Token을 Return하고 /* 일 경우 */가 입력될 때까지 무시하도록 하였다.

```c
"*/"    { char c;
	  do
	  { c = input();
	    if(c == EOF) break;
	    if(c == '\n') lineno++;
	  } while (c != "*" || input() != '/');
         }
```

# [Project 2 Parser]

**C-Minus Parser Implementation**

Implementation Method : Yacc (Bison) - "cminus_parser"

[Compilation method and environment]

VirtualBox Linux: Ubuntu 18.0.4

# Implementation

## 1. globals.h

Abstract Syntax Tree를 구성하는 Node의 유형을 다음과 같이 정의하였다. 또한 아래에 맞게 struct TreeNode 또한 수정하였다.

```c
typedef enum {StmtK, ExpK, DecK, TypeK, ParamK, OptK} NodeKind;
typedef enum {CompoundK, IfK, IfelseK, WhileK, ReturnK} StmtKind;
typedef enum {AssignK, VariableK, OpK, ConstK, CallK, IdK} ExpKind;
typedef enum {VarK, FunK} DecKind;
typedef enum {IntK, VoidK} TypeKind;
typedef enum {SingleK, ArrayK} ParamKind;
typedef enum {RelopK, AddopK, MulopK} OptKind;
```

## 2. util.c

새로 설정한 Node type에 알맞는 생성 함수를 추가하고, printTree에서는 Node type에 알맞은 출력형식을 따랐다. 자세한 내용은 코드 참조.
## 3. cminus.y

Token과 그에 따른 Priority와 Associativity는 다음과 같이 정의했다. 기준은 C-Minus를 C의 속한다고 판단해 C의 Priority와 Associativity를 따랐다.
```c
%token IF ELSE WHILE RETURN INT VOID
%token ID NUM
%token LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI
%left TIMES OVER
%left PLUS MINUS
%left LT LE GT GE
%left EQ NE
%right ASSIGN
%left COMMA
%token ERROR
```

**Type**의 경우, INT/VOID는 tree→type을 통해 구분하고 단일 변수 / 배열은 Param Node에서는 tree→nodekind로 구분하고 나머지는 tree→child[0]에 따라 구분하였다.

**ID, Type, Operator에 대한** 정보를 저장하기 위해 하위 Node를 새로 설정하고 Reduce시, 정보만 상위 Node에 저장하고 Free 함수를 통해 메모리를 비우게 하였다.

자세한 내용은 코드 참조.

# [Project 3 Semantic Analyzer]


**C-Minus Semantic Analyzer Implementation**

[Goal] 

1. Hierarchical Symbol Table 구현
2. Semantic Error 검출

[Compilation method and environment]

VirtualBox Linux: Ubuntu 18.0.4

# Implementation

## 1. Symbol Table

Hierarchical Symbol Table을 구현하기 위해 다음과 같은 사항을 고려했다.

- Original Compiler의 Symbol Table
- Symbol Table이 생성되는 조건
- Hierarchical Symbol Table의 특성과 구현

### (1) Original Compiler의 Symbol Table

- 기존의 Symbol Table은 소스 코드 하나에 하나의 Symbol Table을 가정하고 구현되어 있다.
- 하나의 Symbol은 하나의 BucketList에 할당되며, hashTable이라는 전역 변수를 두고 할당된다.
- 따라서 Hierarchical Symbol Table을 구현하기 위해 BucketList의 상위 계층인 ScopeList를 만들었다.

```c
// symtab.c
typedef struct ScopeListRec {
	char * name; // Scope의 이름
	BucketList hashTable[SIZE]; // ScopeList의 하위 계층
	struct ScopeListRec * parent; // Scope의 parent scope
	int depth; // global depth = 0 기준 child scope + 1
} * ScopeList
```

### (2) Symbol Table이 생성되는 조건

- Symbol Table이 만들어지는 조건은 크게 두가지이다.
    1. **Function Declaration**
    2. **Variable Declaration**
- 이전 2_Parser에서 구현한대로, Function Declaration은 TreeNode가 FunK일때, Variable Declaration은 CompoundK일때 만들어진다.
- 그러므로 analysis.c의 insertNode(TreeNode * t)에서 FunK일때, CompoundK일때 새로운 Scope를 생성하도록 하였다.
- FunK 일때, child[0]이 arguments, child[1]이 compound_stmt이다. 위의 사항대로 단순히 Scope를 생성하면 하나의 Function에 두 개의 Scope가 연속적으로 생길 수 있으므로 flag를 두어 FunK일때 set하고, CompoundK 일때 flag가 올라가 있으면 flag를 내리게 하였다.

### (3) Hierarchical Symbol Table의 특징과 구현

- AST를 traverse하며 Hierarchical Symbol Table을 생성하는 과정에서 , Symbol Table이 완성되기 전에 새로운 Scope를 형성하기도 한다.
- 이는 **Stack** 구조를 통해 구현할 수 있다고 판단하였고, 전체 결과를 담을 전역 변수도 두었다.
- 또한 Scope 생성 뿐만이 아니라, 만들어진 ScopeList에 쌓이는 location을 추가하기 위해 location에 해당하는 stack도 만들어 주었다.

```c
// symtab.c
static ScopeList scopes[SIZE]; // Hierarchical Symbol Table 결과
static int num = -1;
static ScopeList stack[SIZE]; // Building Table에 사용되는 Stack 1
static int top = -1;
static int loc_stack[SIZE]; // Building Table에 사용되는 Stack 2
static loc_top = -1;

// symtab.h
void st_init(void); //int input(void), void output(int data)와 같은 것들
void st_newScope(char * name); // FunK, CompoundK 시, 실행
void st_push(ScopeList s); // stack에 필요한 함수들
void st_pop(void);
ScopeList st_get_top(void);
int addLoc(void); // 새로운 Symbol 등록시 실행되는 함수
void popLoc(void); // st_pop()시, loc_stack에서도 pop이 필요

```

## 2. Checking Semantic Error

### (0) AST Traversal

- Hierarchical Symbol Table을 저장하고 있는 전역변수인 scopes는, pre-order의 순서로 저장되어 있다.
- type checking은 input, output 다음 scope부터 진행하면 된다.
- 이런 점을 종합했을 때, checkNode(TreeNode * t)에서 t→kind.stmt가 CompoundK 일때, analyze.c의 전역변수로 지정한 static int idx = 3 의 값을 하나씩 증가시켜주면, 해당 node가 생성된 곳부터 lookup 함수를 호출할 수 있다.

### (1) Undefined / Redefined Variables / Functions

- Undefined/Redefined Variables/Functions Error를 검출
- insertNode에서,
    - Function Declaration, Variable Declaration의 경우, st_lookup의 반환값이 -1이 아닌 경우 **Redefined Error**
    - Call Expression, Variable Expression의 경우, st_lookup의 반환값이 -1인 경우 **Undefined Error**
- typeCheck에서,
    - VariableK 일때, CallK 일때 check

### (2) Array Index Check

- typeCheck에서,
    - VariableK 일때, t→child[0]이 NULL이 아니고 type이 Integer가 아닐 경우 Error

### (3) Function Call Check

- int callCheck(TreeNode * arg, ScopeList f)에서 구현
    - cnt는 table 내에서 Arg인 Bucket의 개수, cnt_는 function call에 들어간 argument의 개수
    - BucketList에서 kind가 Arg이고, memloc이 N이라면, arg = arg→silbing을 N번 반복하면 위치가 동기회된다고 볼수 있다.
    - type이 일치하지 않거나 마지막 cnt와 cnt_가 같지 않을 경우 -1을 반환

### (4) Type Check

- Original Compiler의 Traversal을 C-Minus Specification에 맞추어 이용

## 3. Output Format

- 출력 양식은 자유라고 알고 있어, 그냥 모든 Scope를 출력하게 하였습니다.

