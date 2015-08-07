#include "lexan.h"

#define ERROR() return error("Unrecognized input")

map<string, TokenType> Token::keywords = setKeywords();

Lexan::Lexan(const char * filename) : filename(filename), lineCounter(1) {
	std::cout << "Opening file " << filename << endl;
	inputFile.open(filename);

	if(!inputFile.good()) {
		printf("Input file error - cannot open file\n");
		exit(1);
	}

	getChar();
}

Token Lexan::readToken() {
	D(cout << "Parsing token..." << endl);

	Token token;
	while(input == WHITESPACE) getChar();

	switch(input) {
		case LETTER:
			token = Token::createIdentifier(lineCounter);
			while(input == NUMBER || input == LETTER) {
				token.appendChar(inputChar);
				getChar();
			}
			token.line = lineCounter;
			token.identifier->shrink();
			return token.checkKeyword();

		case NUMBER:
			if(inputChar == '0') {
				getChar();
				if(inputChar == 'x' || inputChar == 'X') {
					getChar();
					goto hex;
				} else if(input == NUMBER) {
					goto octal;
				} else if(input == WHITESPACE || input == NO_TYPE) {
					return Token::createNumber(0, lineCounter);
				} else {
					ERROR();
				}
			}

			number = 0;
			while(input == NUMBER) {
				number = number * 10 + inputChar - '0';
				getChar();
			}
			if(input == WHITESPACE || input == NO_TYPE) {
				return Token::createNumber(number, lineCounter);
			}
			return error("Cannot parse number");
		break;

		case NO_TYPE:
			switch(inputChar) {
				case '\'':
					D(cout << "Parsing string..." << endl);
					token = Token::createIdentifier(lineCounter);
					token.type = STRING;

					while(1) {
						getChar();

						if(inputChar == '\'') {
							token.identifier->shrink();
							break;
						} else if(input == END) {
							return error("Unexpected end of file in string");
						} else if(inputChar == '\\') {
							getChar();
							switch(inputChar) {
								case '\'':
								case '\\':
									token.appendChar(inputChar);
								break;
								case 'n':
									token.appendChar('\n');
								break;
								case 't':
									token.appendChar('\t');
								break;
								default:
									return error("Unknown escape sequence");
							}
							continue;
						}
						token.appendChar(inputChar);
					}

				break;
				case '&':
					getChar();
					goto octal;
				break;
				case '$':
					getChar();
					goto hex;
				case ',':
					token.type = COMMA;
				break;
				case '.':
					token.type = STOP;
				break;
				case ';':
					token.type = SEMICOLON;
				break;
				case '(':
					token.type = LPAR;
				break;
				case ')':
					token.type = RPAR;
				break;
				case '[':
					token.type = LSQBRAC;
				break;
				case ']':
					token.type = RSQBRAC;
				break;
				case '{':
					token.type = LBRACE;
				break;
				case '}':
					token.type = RBRACE;
				break;
				case '+':
					token.type = PLUS;
				break;
				case '-':
					token.type = MINUS;
				break;
				case '*':
					token.type = TIMES;
				break;
				case '/':
					token.type = DIVIDE;
				break;
				case ':':
					getChar();
					if(inputChar == '=') {
						token.type = ASSIGN;
						break;
					} else if(input == WHITESPACE) {
						token.type = COLON;
						break;
					}
					ERROR();
				case '<':
					getChar();
					if(input == WHITESPACE) {
						token.type = LT;
						break;
					} else if (inputChar == '=') {
						token.type = LTE;
						break;
					} else if (inputChar == '>') {
						token.type = NEQ;
						break;
					}
					ERROR();
				case '>':
					getChar();
					if(input == WHITESPACE) {
						token.type = GT;
						break;
					} else if (inputChar == '=') {
						token.type = GTE;
						break;
					}
					ERROR();

				case '=':
					token.type = EQ;
					break;
			}

			token.line = lineCounter;
			getChar();
			return token;

		case END:
			token.type = EOI;
			token.line = lineCounter;
			getChar();
			return token;

		default:
			ERROR();
	}

	octal:
		number = 0;
		while(input == NUMBER) {
			if(input >= '8') return error("Cannot parse octal number");
			number = number * 8 + inputChar - '0';
			getChar();
		}

		if(input == WHITESPACE || input == NO_TYPE) {
			return Token::createNumber(number, lineCounter);
		}
		return error("Cannot parse octal number");

	hex:
		number = 0;
		while(input == NUMBER || input == LETTER) {
			number *= 16;

			inputChar -= '0';

			if(inputChar > 9) {
				inputChar -= 7;
				if(inputChar > 15) {
					inputChar -= 32;
				}
			}

			if(inputChar < 0 || inputChar > 15) {
				return error("Cannot parse hexadecimal number");
			}
			number += inputChar;
			getChar();
		}

		if(input == WHITESPACE || input == NO_TYPE) {
			return Token::createNumber(number, lineCounter);
		}
		return error("Cannot parse number");

	ERROR();
}

Token Lexan::error(const char * msg) {
	std::cerr << filename << ":" << lineCounter << " " << msg << endl;
	return Token();
}

void Lexan::getChar() {
	inputChar = inputFile.get();

	if(inputChar == '\n') lineCounter++;

	if ( isalpha(inputChar) || inputChar == '_' )
		input = LETTER;
	else if ( isdigit(inputChar) )
		input = NUMBER;
	else if (inputChar == EOF)
		input = END;
	else if ( isspace(inputChar) )
		input = WHITESPACE;
	else
		input = NO_TYPE;
}

Token Token::createNumber(int value, unsigned int line) {
	Token t;
	t.type = NUMB;
	t.line = line;
	t.value = value;
	return t;
}

void Token::print(ostream & out) {
	out << "<" << symbTable[type] << ":" << line;
	if(type == NUMB) {
		out << " " << value;
	} else if (type == IDENT) {
		out << " " << *identifier;
	} else if (type == STRING) {
		out << " '" << *identifier << "'";
	}
	out << ">" << endl;
}

Token Token::createIdentifier(unsigned int line) {
	Token tmp;
	tmp.type = IDENT;
	tmp.line = line;
	tmp.identifier = new Text;
	return tmp;
}

Token & Token::checkKeyword() {
	map<string, TokenType>::iterator it = Token::keywords.find(identifier->text);

	if(it != Token::keywords.end()) {
		type = it->second;
	}

	return *this;
}

void Token::appendChar(char input) {
	identifier->appendChar(input);
}

Text::Text(size_t size) : size(0), maxSize(size) {
	text = new char[size];
}

Text::~Text() {
	delete[] text;
}

void Text::appendChar(char c) {
	text[size++] = c;
	if(size >= maxSize) {
		char * tmp = text;
		maxSize *= 2;
		text = new char[maxSize];
		memcpy(text, tmp, size);
		delete[] tmp;
	}
}

void Text::shrink() {
	if(maxSize > size + 1) {
		char * tmp = text;
		text = new char[size + 1];
		memcpy(text, tmp, size);
		text[size] = 0;
		maxSize = size + 1;
		delete[] tmp;
	}
}

ostream & operator<<(ostream & os, Text & t) {
	os << t.text;
	return os;
}