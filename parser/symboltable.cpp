#include "symboltable.h"

tree SymbolTable::getIdentifier(const string & id) {
	map<string, tree>::iterator it = localScope.find(id);
	if(it != localScope.end()) {
		return it->second;
	}

	it = globalScope.find(id);

	if(it != globalScope.end()) {
		return it->second;
	}

	printf("Identifier %s is not declared\n", id.c_str());
	exit(1);	
}

tree SymbolTable::declareIdentifier(const string & id, tree type, bool isConst) {
	map<string, tree>::iterator it;

	if(isLocalScope) {
		it = localScope.find(id);
		if(it != localScope.end()) {
			printf("Identifier %s already declared\n", id.c_str());
			exit(1);	
		}
	} else {
		it = globalScope.find(id);
		if(it != globalScope.end()) {
			printf("Identifier %s already declared\n", id.c_str());
			exit(1);	
		}
	}

	tree xDecl = NULL_TREE;

	xDecl = Generic::createVariable(id, !isLocalScope, type, isConst);

	if(isLocalScope) {
		localScope[id] = xDecl;
	} else {
		globalScope[id] = xDecl;
	}

	return xDecl;
}

void SymbolTable::registerIdentifier(const string & id, tree t) {
	map<string, tree>::iterator it;

	if(isLocalScope) {
		it = localScope.find(id);
		if(it != localScope.end()) {
			printf("Identifier %s already declared\n", id.c_str());
			exit(1);	
		}
	} else {
		it = globalScope.find(id);
		if(it != globalScope.end()) {
			printf("Identifier %s already declared\n", id.c_str());
			exit(1);	
		}
	}

	if(isLocalScope) {
		localScope[id] = t;
	} else {
		globalScope[id] = t;
	}
}

void SymbolTable::enterLocalScope() {
	isLocalScope = true;
}

void SymbolTable::leaveLocalScope() {
	isLocalScope = false;
	localScope.clear();
}

tree SymbolTable::getFunction(const string & id) {
	map<string, tree>::iterator it = functions.find(id);
	if(it != functions.end()) {
		return it->second;
	}

	printf("Function %s is not declared\n", id.c_str());
	exit(1);	
}

void SymbolTable::registerFunction(const string & id, tree t) {
	map<string, tree>::iterator it;

	it = functions.find(id);
	if(it != functions.end()) {
		printf("Function %s already declared\n", id.c_str());
		exit(1);	
	}

	functions[id] = t;
}