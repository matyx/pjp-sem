#include "lexan.h"
#include "generic.h"
#include "symboltable.h"
#include <list>

using namespace std;

class Parser {
public:
	Parser(Lexan * lex);
	bool parse(tree * main);

protected:
	Lexan * lex;
	Token nextToken;
	SymbolTable symbolTable;
	map<string, tree> forwardDecl;
	tree retVar;

	tree Start();

	tree Decl();
	tree Decl_const();
	tree Decl_const_l();
	tree Decl_const_ll();
	tree Decl_var();
	tree Decl_var_l(list<string> & variables);
	tree Decl_var_ll(list<string> & variables);

	void Decl_args(tree * paramsDecl, tree * paramsTypes);
	void Decl_args_l(tree * paramsDecl, tree * paramsTypes);

	tree Decl_proc();
	tree Decl_proc_l(const char * id);
	void Decl_proc_ll(tree fndecl, const char * id, bool declared = false);

	tree Decl_func();
	tree Decl_func_l(const char * id);
	void Decl_func_ll(tree fndecl, const char * id, tree retType, bool declared = false);

	void Block(tree * stmlist);
	tree Command();
	void Command_l(tree ret, tree * stmlist);
	tree Ident(const char * id);
	tree Else();

	void Args(list<tree> & args);
	void Args_l(list<tree> & args);

	tree Expr1();
	tree Expr2();
	tree Expr2_l(tree);
	tree Expr3();
	tree Expr3_l(tree);
	tree Expr4();
	tree Expr4_l(tree);
	tree Expr5();
	tree Expr5_l(tree);
	tree Expr6();
	tree Expr6_l(tree);
	tree Expr7_l(tree);
	tree Expr();

	tree Factor();
	tree Factor_l(const string & id);

	tree Type();
	void For_l(tree * stmlist, tree var);

	void compare(TokenType type);
	const char * compareIdentifier();
	int compareNumber();
	void error(const char * msg, Token * token);
};