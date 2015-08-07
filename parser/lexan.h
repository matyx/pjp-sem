#ifndef __LEXAN_GUARD__

#define __DEBUG__

#include <fstream>
#include <iostream>
#include <string.h>
#include <map>
#include <cstdlib>

using namespace std;
#define __LEXAN_GUARD__

#ifdef __DEBUG__
#define D(x) x
#else
#define D(x)
#endif

#define TOKEN_COUNT 53

enum TokenType {
	IDENT, NUMB, PLUS, MINUS, TIMES, DIVIDE, MOD,
	EQ, NEQ, LT, GT, LTE, GTE, LPAR, RPAR, ASSIGN, AND, OR, LSQBRAC, RSQBRAC, LBRACE, RBRACE,
	COMMA, SEMICOLON, COLON, STOP,
	STRING, kwINTEGER, kwARRAY, kwOF,
	kwVAR, kwCONST, kwBEGIN, kwEND, kwEXIT, kwIF, kwTHEN, kwELSE,
	kwFOR, kwTO, kwDOWNTO, kwWHILE, kwDO, kwBREAK, kwWRITE, kwWRITELN, kwREAD,
	kwPROGRAM, kwPROCEDURE, kwFUNCTION, kwFORWARD,
	EOI, ERR
};

static const char *symbTable[TOKEN_COUNT] = {
	"IDENT", "NUMB", "PLUS", "MINUS", "TIMES", "DIVIDE", "MOD",
	"EQ", "NEQ", "LT", "GT", "LTE", "GTE", "LPAR", "RPAR", "ASSIGN", "AND", "OR", "LSQBRAC", "RSQBRAC", "LBRACE", "RBRACE",
	"COMMA", "SEMICOLON", "COLON", "STOP",
	"STRING", "kwINTEGER", "kwARRAY", "kwOF",
	"kwVAR", "kwCONST", "kwBEGIN", "kwEND", "kwEXIT", "kwIF", "kwTHEN", "kwELSE",
	"kwFOR", "kwTO", "kwDOWNTO", "kwWHILE", "kwDO", "kwBREAK", "kwWRITE", "kwWRITELN", "kwREAD",
	"kwPROGRAM", "kwPROCEDURE", "kwFUNCTION", "kwFORWARD",
	"EOI", "ERR"
}; //symbol names in the same order as in LexSymbolType

static map<string, TokenType> setKeywords() {
	map<string, TokenType> keywords;

	keywords["program"] = kwPROGRAM;
	keywords["procedure"] = kwPROCEDURE;
	keywords["function"] = kwFUNCTION;
	keywords["forward"] = kwFORWARD;

	keywords["const"] = kwCONST;
	keywords["var"] = kwVAR;

	keywords["integer"] = kwINTEGER;
	keywords["array"] = kwARRAY;
	keywords["of"] = kwOF;

	keywords["if"] = kwIF;
	keywords["then"] = kwTHEN;
	keywords["else"] = kwELSE;
	keywords["begin"] = kwBEGIN;
	keywords["end"] = kwEND;

	keywords["while"] = kwWHILE;
	keywords["do"] = kwDO;
	keywords["break"] = kwBREAK;
	keywords["exit"] = kwEXIT;
	keywords["for"] = kwFOR;
	keywords["to"] = kwTO;
	keywords["downto"] = kwDOWNTO;

	keywords["read"] = kwREAD;
	keywords["write"] = kwWRITE;
	keywords["writeln"] = kwWRITELN;


	keywords["div"] = DIVIDE;
	keywords["mod"] = MOD;
	keywords["and"] = AND;
	keywords["or"] = OR;

	return keywords;
}

class Text {
	public:
		Text(size_t size = 256);
		~Text();
		void appendChar(char c);
		void shrink();

		char * text;
		size_t size;
	private:
		size_t maxSize;
};

ostream & operator<<(ostream & os, Text & t);

class Token {
	public:
		static Token createNumber(int val, unsigned int line);
		static Token createIdentifier(unsigned int line);

		void appendChar(char input);
		Token & checkKeyword();
		void print(ostream & out);


		Token() : type(ERR), identifier(NULL), position(0) { };
		~Token() { };

		TokenType type;

		Text * identifier;
		int value;
		unsigned int line;
		static map<string, TokenType> keywords;
	protected:
		unsigned int position;
};

class Lexan {
	public:
		Lexan(const char * filename);
		~Lexan() {}
		Token readToken();

		const char * filename;
	protected:
		Token error(const char * msg);
		void getChar();

		ifstream inputFile;
		int number;
		int inputChar;
		enum { NUMBER, LETTER, END, WHITESPACE, NO_TYPE } input;
		int lineCounter;
};

#endif
