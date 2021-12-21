# Compiler
2021년 가을학기 HYU CSE의 컴파일러설계 수강 중 진행한 C-MINUS Compiler Project
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
