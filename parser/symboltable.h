#ifndef __SYMBOL_TABLE_GUARD__
#define __SYMBOL_TABLE_GUARD__

#include "generic.h"
#include <map>
#include <string>

using namespace std;

class SymbolTable {
	public:
		SymbolTable() : isLocalScope(0) {}

		tree getIdentifier(const string & id);
		tree declareIdentifier(const string & id, tree type, bool isConst = false);
		void registerIdentifier(const string & id, tree t);

		void registerFunction(const string & id, tree t);
		tree getFunction(const string & id);

		void enterLocalScope();
		void leaveLocalScope();
	protected:
		map<string, tree> functions;
		map<string, tree> localScope;
		map<string, tree> globalScope;
		bool isLocalScope;
};

#endif