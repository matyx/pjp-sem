#ifndef __GENERIC_GUARD__
#define __GENERIC_GUARD__

#include "../sfe-lang.h"
#include <string>

using namespace std;

enum Type {
	undef, Integer, Array, constInt, intFunction, procedure
};

class Generic {
public:
	static tree createInt(const string & id, bool isGlobal = false, bool isConst = false);
	static tree createVariable(const string & id, bool isGlobal, tree type, bool isConst = false);

	static tree createCall(tree fndecl, int arg_cnt, tree * args_vec);

	static tree createPrint(tree int_expr, bool newLine = false);
	static tree createScan(tree var_expr);

	static tree createFunctionDecl(const char *name, bool external, tree function_type);

	static tree createBreakIf(tree cond);
};

#endif