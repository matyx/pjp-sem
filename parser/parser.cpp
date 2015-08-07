#include "parser.h"


Parser::Parser(Lexan * lex) : lex(lex), retVar(NULL_TREE) {
	nextToken = lex->readToken();
	D(nextToken.print(std::cout));
}

bool Parser::parse(tree * main) {
	*main = Start();

	return true;
}

void Parser::error(const char * msg, Token * token = NULL) {
	std::cerr << lex->filename << ":" << (token ? token->line : 0) << " " << msg << endl;
	printf("Syntax error\n");
	exit(1);	
}

void Parser::compare(TokenType type) {
	if(nextToken.type == type) {
		nextToken = lex->readToken();
		D(nextToken.print(std::cout));
	} else {
		char msg[128];
		sprintf(msg, "Syntax error - unexpected %s, expecting %s", symbTable[nextToken.type], symbTable[type]);
		error(msg, & nextToken);
	}
}

tree Parser::Start() {
	tree paramsDecl = NULL_TREE;
	tree paramsTypes = NULL_TREE;
	tree resDecl = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
	tree fnType = build_function_type(TREE_TYPE(resDecl), paramsTypes);
	tree fnDecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("main"), fnType);
	DECL_ARGUMENTS(fnDecl) = paramsDecl;
	DECL_RESULT(fnDecl) = resDecl;

	TREE_STATIC(fnDecl) = true;
	TREE_PUBLIC(fnDecl) = true;

	tree block = build_block(NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);

	compare(kwPROGRAM);
	compare(IDENT);
	compare(SEMICOLON);
	tree globalDecls = Decl();
	tree stmlist = alloc_stmt_list();
	Block(&stmlist);

	tree bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(block), NULL_TREE, block);
	BIND_EXPR_BODY(bind) = stmlist;
	TREE_SIDE_EFFECTS(bind) = true;
	BLOCK_SUPERCONTEXT(block) = fnDecl;

	DECL_INITIAL(fnDecl) = block;
	DECL_SAVED_TREE(fnDecl) = bind;

	compare(STOP);
	compare(EOI);

	return fnDecl;
}

tree Parser::Decl() {
	tree decls = NULL_TREE;

	switch(nextToken.type) {
		case kwCONST:
			decls = Decl_const();
			TREE_CHAIN(decls) = Decl();
		break;
		case kwVAR:
			decls = Decl_var();
			TREE_CHAIN(decls) = Decl();
		break;
		case kwPROCEDURE:
			decls = Decl_proc();
			TREE_CHAIN(decls) = Decl();
		break;
		case kwFUNCTION:
			decls = Decl_func();
			TREE_CHAIN(decls) = Decl();
		break;
		default:
			break;
	}

	return decls;
}

tree Parser::Decl_const() {
	compare(kwCONST);

	tree decls = NULL_TREE;

	const char * id = compareIdentifier();
	decls = symbolTable.declareIdentifier(id, integer_type_node, true);
	compare(EQ);
	DECL_INITIAL(decls) = build_int_cst(integer_type_node, compareNumber());
	TREE_CONSTANT(decls) = true;

	TREE_CHAIN(decls) = Decl_const_l();
	compare(SEMICOLON);

	tree tmp = decls;
	while(1) {
		if(TREE_CHAIN(tmp) == NULL_TREE) break;
		tmp = TREE_CHAIN(tmp);
	}

	TREE_CHAIN(tmp) = Decl_const_ll();

	return decls;
}

tree Parser::Decl_const_l() {
	if(nextToken.type == COMMA) {
		compare(COMMA);
		const char * id = compareIdentifier();
		tree t = symbolTable.declareIdentifier(id, integer_type_node, true);
		compare(EQ);
		DECL_INITIAL(t) = build_int_cst(integer_type_node, compareNumber());
		TREE_CONSTANT(t) = true;
		TREE_CHAIN(t) = Decl_const_l();
		return t;
	}
	return NULL_TREE;
}

tree Parser::Decl_const_ll() {
	if(nextToken.type == IDENT) {
		const char * id = compareIdentifier();

		tree ret = symbolTable.declareIdentifier(id, integer_type_node, true);

		compare(EQ);
		DECL_INITIAL(ret) = build_int_cst(integer_type_node, compareNumber());
		TREE_CONSTANT(ret) = true;

		TREE_CHAIN(ret) = Decl_const_l();
		compare(SEMICOLON);

		tree tmp = ret;
		while(1) {
			if(TREE_CHAIN(tmp) == NULL_TREE) break;
			tmp = TREE_CHAIN(tmp);
		}

		TREE_CHAIN(tmp) = Decl_const_ll();
		return ret;
	}
	return NULL_TREE;
}

tree Parser::Decl_var() {
	list<string> variables;
	compare(kwVAR);
	variables.push_back(compareIdentifier());
	Decl_var_l(variables);
	compare(COLON);
	tree type = Type();

	tree vars = NULL_TREE;

	vars = symbolTable.declareIdentifier(variables.front(), type);
	variables.pop_front();

	for(int i = 0, cnt = variables.size(); i < cnt; i++) {
		chainon(vars, symbolTable.declareIdentifier(variables.front(), type));
		variables.pop_front();
	}

	compare(SEMICOLON);
	chainon(vars, Decl_var_ll(variables));

	return vars;
}

tree Parser::Decl_var_l(list<string> & variables) {
	if(nextToken.type == COMMA) {
		compare(COMMA);
		variables.push_back(compareIdentifier());
		Decl_var_l(variables);
	}

	return NULL_TREE;
}

tree Parser::Decl_var_ll(list<string> & variables) {
	if(nextToken.type == IDENT) {
		variables.push_back(compareIdentifier());
		Decl_var_l(variables);
		compare(COLON);

		tree type = Type();

		tree vars = NULL_TREE;

		vars = symbolTable.declareIdentifier(variables.front(), type);
		variables.pop_front();

		for(int i = 0, cnt = variables.size(); i < cnt; i++) {
			chainon(vars, symbolTable.declareIdentifier(variables.front(), type));
			variables.pop_front();
		}
		compare(SEMICOLON);
		chainon(vars, Decl_var_ll(variables));
		return vars;
	}

	return NULL_TREE;
}

void Parser::Decl_args(tree * paramsDecl, tree * paramsTypes) {
	if(nextToken.type == IDENT) {
		const char * id = compareIdentifier();
		compare(COLON);

		tree type = Type();

		*paramsDecl = build_decl(UNKNOWN_LOCATION, PARM_DECL, get_identifier( id ), type);
		DECL_ARG_TYPE(*paramsDecl) = type;

		*paramsTypes = chainon( *paramsTypes , tree_cons(NULL_TREE, type, NULL_TREE));

		symbolTable.registerIdentifier(id, *paramsDecl);

		Decl_args_l(paramsDecl, paramsTypes);
	}
}

void Parser::Decl_args_l(tree * paramsDecl, tree * paramsTypes) {
	if(nextToken.type == SEMICOLON) {
		compare(SEMICOLON);
		const char * id = compareIdentifier();
		compare(COLON);

		tree type = Type();

		tree tmp = build_decl(UNKNOWN_LOCATION, PARM_DECL, get_identifier( id ), type);
		DECL_ARG_TYPE(tmp) = type;

		chainon(*paramsDecl, tmp);
		chainon( *paramsTypes , tree_cons(NULL_TREE, type, NULL_TREE));

		symbolTable.registerIdentifier(id, tmp);

		Decl_args_l(paramsDecl, paramsTypes);
	}
}

tree Parser::Decl_proc() {
	compare(kwPROCEDURE);
	const char * id = compareIdentifier();

	symbolTable.enterLocalScope();
	tree fndecl = Decl_proc_l(id);
	symbolTable.leaveLocalScope();

	return fndecl;
}

tree Parser::Decl_proc_l(const char * id) {
	bool declared = false;
	tree fndecl = NULL_TREE;
	tree resDecl = NULL_TREE;

	tree fntype = NULL_TREE;

	tree paramsDecl = NULL_TREE;
	tree paramsTypes = NULL_TREE;
	
	resDecl = build_decl(BUILTINS_LOCATION , RESULT_DECL, NULL_TREE , void_type_node);

	switch(nextToken.type) {
		case SEMICOLON:
			compare(SEMICOLON);

			fntype = build_function_type(TREE_TYPE(resDecl), paramsTypes);
			fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id), fntype);
			DECL_ARGUMENTS(fndecl) = paramsDecl;
			DECL_RESULT(fndecl) = resDecl;
			TREE_STATIC(fndecl) = true;
			TREE_PUBLIC(fndecl) = true;
		break;
		case LPAR:
			compare(LPAR);

			Decl_args(&paramsDecl, &paramsTypes);

			fntype = build_function_type(TREE_TYPE(resDecl), paramsTypes);

			fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id), fntype);
			DECL_ARGUMENTS(fndecl) = paramsDecl;
			DECL_RESULT(fndecl) = resDecl;
			TREE_STATIC(fndecl) = true;
			TREE_PUBLIC(fndecl) = true;

			compare(RPAR);
			compare(SEMICOLON);
		break;
		default:
			break;
	}

	if(forwardDecl.find(id) != forwardDecl.end()) {
		fndecl = forwardDecl[id];
		declared = true;
		forwardDecl.erase(id);
	}

	Decl_proc_ll(fndecl, id, declared);
	return fndecl;
}

void Parser::Decl_proc_ll(tree fndecl, const char * id, bool declared) {
	tree stmlist = NULL_TREE;
	tree blockTree = NULL_TREE;
	tree decls = NULL_TREE;
	tree bind = NULL_TREE;

	switch(nextToken.type) {
		case kwFORWARD:
			compare(kwFORWARD);

			forwardDecl[id] = fndecl;
			symbolTable.registerFunction(id, fndecl);

			compare(SEMICOLON);
		break;
		case kwCONST:
		case kwVAR:
		case kwPROCEDURE:
		case kwFUNCTION:
		case kwBEGIN:
			if(!declared) symbolTable.registerFunction(id, fndecl);

			stmlist = alloc_stmt_list();
			decls = Decl();
			if(decls != NULL_TREE) {
				append_to_statement_list(build1(DECL_EXPR, void_type_node, decls), &stmlist);
			}

			blockTree = build_block(decls, NULL_TREE, NULL_TREE, NULL_TREE);
			TREE_USED(blockTree) = true;

			Block(&stmlist);

			bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(blockTree), NULL_TREE, blockTree);
			BIND_EXPR_BODY(bind) = stmlist;
			TREE_SIDE_EFFECTS(bind) = true;

			BLOCK_SUPERCONTEXT(blockTree) = fndecl;
			DECL_INITIAL(fndecl) = blockTree;
			DECL_SAVED_TREE(fndecl) = bind;

			compare(SEMICOLON);
			register_global_function_declaration(fndecl);
		break;
		default:
			return;
	}
}

tree Parser::Decl_func() {
	compare(kwFUNCTION);
	const char * id = compareIdentifier();

	symbolTable.enterLocalScope();
	tree fndecl = Decl_func_l(id);
	symbolTable.leaveLocalScope();

	retVar = NULL_TREE;

	return fndecl;
}

tree Parser::Decl_func_l(const char * id) {
	bool declared = false;
	tree fndecl = NULL_TREE;
	tree resDecl = NULL_TREE;
	tree resType = NULL_TREE;

	tree fntype = NULL_TREE;

	tree paramsDecl = NULL_TREE;
	tree paramsTypes = NULL_TREE;

	if(forwardDecl.find(id) != forwardDecl.end()) {
		fndecl = forwardDecl[id];
		declared = true;
		forwardDecl.erase(id);
	}

	switch(nextToken.type) {
		case COLON:
			compare(COLON);
			resType = Type();

			resDecl = build_decl(BUILTINS_LOCATION , RESULT_DECL, NULL_TREE , resType);
			retVar = resDecl;

			compare(SEMICOLON);

			fntype = build_function_type(TREE_TYPE(resDecl), paramsTypes);
			fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id), fntype);
			DECL_ARGUMENTS(fndecl) = paramsDecl;
			DECL_RESULT(fndecl) = resDecl;
			TREE_STATIC(fndecl) = true;
			TREE_PUBLIC(fndecl) = true;		
		break;
		case LPAR:
			compare(LPAR);

			Decl_args(&paramsDecl, &paramsTypes);
			compare(RPAR);

			compare(COLON);
			resType = Type();
			resDecl = build_decl(BUILTINS_LOCATION , RESULT_DECL, NULL_TREE , resType);
			//resDecl = build_decl(BUILTINS_LOCATION , RESULT_DECL, NULL_TREE , integer_type_node);
			retVar = resDecl;

			fntype = build_function_type(TREE_TYPE(resDecl), paramsTypes);

			fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier(id), fntype);
			DECL_ARGUMENTS(fndecl) = paramsDecl;
			DECL_RESULT(fndecl) = resDecl;
			TREE_STATIC(fndecl) = true;
			TREE_PUBLIC(fndecl) = true;

			compare(SEMICOLON);
		break;
		default:
			break;
	}

	if(forwardDecl.find(id) != forwardDecl.end()) {
		fndecl = forwardDecl[id];
		declared = true;
		forwardDecl.erase(id);
	}

	Decl_func_ll(fndecl, id, resType, declared);

	return fndecl;
}

void Parser::Decl_func_ll(tree fndecl, const char * id, tree retType, bool declared) {
	tree retValue = NULL_TREE;
	tree stmlist = NULL_TREE;
	tree blockTree = NULL_TREE;
	tree decls = NULL_TREE;
	tree bind = NULL_TREE;

	switch(nextToken.type) {
		case kwFORWARD:
			compare(kwFORWARD);

			forwardDecl[id] = fndecl;
			symbolTable.registerFunction(id, fndecl);

			compare(SEMICOLON);
		break;
		case kwCONST:
		case kwVAR:
		case kwPROCEDURE:
		case kwFUNCTION:
		case kwBEGIN:
			if(!declared) symbolTable.registerFunction(id, fndecl);

			stmlist = alloc_stmt_list();

			symbolTable.registerIdentifier(id, retVar);
			append_to_statement_list(build1(DECL_EXPR, void_type_node, retVar), &stmlist);

			decls = Decl();
			if(decls != NULL_TREE) {
				append_to_statement_list(build1(DECL_EXPR, void_type_node, decls), &stmlist);
			}

			blockTree = build_block(decls, NULL_TREE, NULL_TREE, NULL_TREE);
			TREE_USED(blockTree) = true;

			Block(&stmlist);

			append_to_statement_list(build1(RETURN_EXPR, void_type_node, retVar), &stmlist);

			bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(blockTree), NULL_TREE, blockTree);
			BIND_EXPR_BODY(bind) = stmlist;
			TREE_SIDE_EFFECTS(bind) = true;

			BLOCK_SUPERCONTEXT(blockTree) = fndecl;
			DECL_INITIAL(fndecl) = blockTree;
			DECL_SAVED_TREE(fndecl) = bind;

			compare(SEMICOLON);
			register_global_function_declaration(fndecl);		
		break;
		default:
			return;
	}
}

void Parser::Block(tree * stmlist) {
	compare(kwBEGIN);
	Command_l(Command(), stmlist);

	compare(kwEND);
}

tree Parser::Command() {
	tree ret = NULL_TREE;
	tree cond = NULL_TREE;
	tree cmd = NULL_TREE;
	tree else_stmt = NULL_TREE;
	tree stmlist;

	const char * id = NULL;

	switch(nextToken.type) {
		case IDENT:
			id = compareIdentifier();
			ret = Ident(id);
		break;
		case kwBREAK:
			compare(kwBREAK);
			ret = build1(EXIT_EXPR, void_type_node, build_int_cst(integer_type_node, 1));
		break;
		case kwEXIT:
			if(retVar != NULL_TREE) {
				ret = build1(RETURN_EXPR, void_type_node, retVar);
			} else {
				ret = build1(RETURN_EXPR, void_type_node, NULL_TREE);
			}
			compare(kwEXIT);
		break;
		case kwBEGIN:
			ret = alloc_stmt_list();
			Block(&ret);
		break;
		case kwFOR:
			compare(kwFOR);
			cond = symbolTable.getIdentifier(compareIdentifier());
			compare(ASSIGN);
			cmd = alloc_stmt_list();
			stmlist = alloc_stmt_list();
			append_to_statement_list(build2(MODIFY_EXPR, TREE_TYPE(cond), cond, Expr()), &cmd);

			For_l(&stmlist, cond);
			append_to_statement_list(build1(LOOP_EXPR, void_type_node, stmlist), &cmd);

			ret = cmd;
		break;
		case kwWHILE:
			compare(kwWHILE);
			cond = Expr();
			compare(kwDO);
			stmlist = alloc_stmt_list();
			append_to_statement_list(build1(EXIT_EXPR, void_type_node, build2(EQ_EXPR, integer_type_node, cond, build_int_cst(integer_type_node, 0))), &stmlist);
			append_to_statement_list( Command(), &stmlist);
			ret = build1(LOOP_EXPR, void_type_node, stmlist);
		break;
		case kwWRITE:
			compare(kwWRITE);
			compare(LPAR);
			ret = Generic::createPrint(Expr());
			compare(RPAR);
		break;
		case kwREAD:
			compare(kwREAD);
			compare(LPAR);
			ret = Generic::createScan(Expr());
			compare(RPAR);
		break;
		case kwWRITELN:
			compare(kwWRITELN);
			compare(LPAR);
			ret = Generic::createPrint(Expr(), true);
			compare(RPAR);
		break;
		case kwIF:
			compare(kwIF);
			cond = Expr();
			compare(kwTHEN);
			cmd = Command();
			else_stmt = Else();
			ret = build3(COND_EXPR, void_type_node, cond, cmd, else_stmt);
		break;
		default:
			break;
	}

	return ret;
}

void Parser::Command_l(tree ret, tree * stmlist) {
	if(nextToken.type == SEMICOLON) {
		compare(SEMICOLON);
		append_to_statement_list(ret, stmlist);
		Command_l(Command(), stmlist);
	}
}

tree Parser::Else() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case kwELSE:
			compare(kwELSE);
			ret = Command();
		break;
		default:
			break;
	}
	return ret;
}

tree Parser::Ident(const char * id) {
	tree ret = NULL_TREE;
	tree var = NULL_TREE;
	tree ident = NULL_TREE;

	tree tmp = NULL_TREE;
	tree * args_vec;
	int argc = 0;
	int i = 0;
	list<tree> args;

	switch(nextToken.type) {
		case ASSIGN:
			compare(ASSIGN);
			var = symbolTable.getIdentifier(id);
			ret = build2(MODIFY_EXPR, TREE_TYPE(var), var, Expr());
		break;
		case LPAR:
			compare(LPAR);
			Args(args);

			argc = args.size();
			args_vec = XNEWVEC(tree, argc);
			for(i = 0; i < argc; i++) {
				args_vec[i] = args.front();
				args.pop_front();
			}
			ret = Generic::createCall(symbolTable.getFunction(id), argc, args_vec);

			compare(RPAR);
		break;
		case LSQBRAC:
			compare(LSQBRAC);
			ident = symbolTable.getIdentifier(id);
			var = build4(ARRAY_REF, TREE_TYPE(TREE_TYPE( ident )), ident, Expr(), NULL_TREE, NULL_TREE);
			compare(RSQBRAC);
			compare(ASSIGN);
			ret = build2(MODIFY_EXPR, TREE_TYPE(var), var, Expr());
		break;
		default:
			break;
	}

	return ret;
}

void Parser::Args(list<tree> & args) {
	switch(nextToken.type) {
		case LPAR:
		case MINUS:
		case IDENT:
		case NUMB:
			args.push_back(Expr());
			Args_l(args);
		break;
		default:
			break;
	}
	return;
}

void Parser::Args_l(list<tree> & args) {
	switch(nextToken.type) {
		case COMMA:
			compare(COMMA);
			args.push_back(Expr());
			Args_l(args);
		break;
		default:
			break;
	}
	return;
}

tree Parser::Expr() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case STRING:
			ret = build_string_literal(nextToken.identifier->size + 1, nextToken.identifier->text);
			compare(STRING);
		break;
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr7_l(Expr6());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr7_l(tree ret) {
	switch(nextToken.type) {
		case OR:
			compare(OR);
			ret = build2(TRUTH_ORIF_EXPR, integer_type_node, ret, Expr7_l(Expr6()));
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr6() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr6_l(Expr5());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr6_l(tree ret) {
	if(nextToken.type == AND) {
		compare(AND);
		ret = build2(TRUTH_ANDIF_EXPR, integer_type_node, ret, Expr6_l(Expr5()));
	}
	return ret;
}

tree Parser::Expr5() {
	tree ret = NULL_TREE;

	switch(nextToken.type) {
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr5_l(Expr4());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr5_l(tree ret) {
	switch(nextToken.type) {
		case EQ:
			compare(EQ);
			ret = build2(EQ_EXPR, integer_type_node, ret, Expr5_l(Expr4()));
		break;
		case NEQ:
			compare(NEQ);
			ret = build2(NE_EXPR, integer_type_node, ret, Expr5_l(Expr4()));
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr4() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr4_l(Expr3());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr4_l(tree ret) {
	switch(nextToken.type) {
		case LT:
			compare(LT);
			ret = build2(LT_EXPR, integer_type_node, ret, Expr4_l(Expr3()));
		break;
		case LTE:
			compare(LTE);
			ret = build2(LE_EXPR, integer_type_node, ret, Expr4_l(Expr3()));
		break;
		case GTE:
			compare(GTE);
			ret = build2(GE_EXPR, integer_type_node, ret, Expr4_l(Expr3()));
		break;
		case GT:
			compare(GT);
			ret = build2(GT_EXPR, integer_type_node, ret, Expr4_l(Expr3()));
		break;
		default:
			break;
	}
	return ret;
}

tree Parser::Expr3() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr3_l(Expr2());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr3_l(tree ret) {
	switch(nextToken.type) {
		case PLUS:
			compare(PLUS);
			ret = build2(PLUS_EXPR, integer_type_node, ret, Expr3_l(Expr2()));
		break;
		case MINUS:
			compare(MINUS);
			ret = build2(MINUS_EXPR, integer_type_node, ret, Expr3_l(Expr2()));
		break;
		default:
			break;
	}
	return ret;
}

tree Parser::Expr2() {
	tree ret = NULL_TREE;

	switch(nextToken.type) {
		case IDENT:
		case NUMB:
		case LPAR:
		case MINUS:
			ret = Expr2_l(Expr1());
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr2_l(tree ret) {
	switch(nextToken.type) {
		case TIMES:
			compare(TIMES);
			ret = build2(MULT_EXPR, integer_type_node, ret, Expr2_l(Expr1()));
		break;
		case DIVIDE:
			compare(DIVIDE);
			ret = build2(TRUNC_DIV_EXPR, integer_type_node, ret, Expr2_l(Expr1()));
		break;
		case MOD:
			compare(MOD);
			ret = build2(TRUNC_MOD_EXPR, integer_type_node, ret, Expr2_l(Expr1()));
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Expr1() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case MINUS:
			compare(MINUS);
			ret = build1( NEGATE_EXPR, integer_type_node, Factor() );
		break;
		case IDENT:
		case NUMB:
		case LPAR:
			ret = Factor();
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Factor() {
	tree ret = NULL_TREE;
	switch(nextToken.type) {
		case IDENT:
			ret = Factor_l(compareIdentifier());
		break;
		case NUMB:
			return build_int_cst( integer_type_node , compareNumber());
		break;
		case LPAR:
			compare(LPAR);
			ret = Expr();
			compare(RPAR);
		break;
		default:
			break;
	}

	return ret;
}

tree Parser::Factor_l(const string & id) {
	tree ret = NULL_TREE;
	tree * args_vec;
	int argc = 0;
	int i = 0;
	list<tree> args;
	tree ident = NULL_TREE;

	switch(nextToken.type) {
		case LSQBRAC:
			compare(LSQBRAC);
			ident = symbolTable.getIdentifier(id);
			ret = build4(ARRAY_REF, TREE_TYPE(TREE_TYPE( ident )), ident, Expr(), NULL_TREE, NULL_TREE);
			compare(RSQBRAC);
			return ret;
		break;
		case LPAR:
			compare(LPAR);
			Args(args);

			argc = args.size();
			args_vec = XNEWVEC(tree, argc);
			for(i = 0; i < argc; i++) {
				args_vec[i] = args.front();
				args.pop_front();
			}
			ret = Generic::createCall(symbolTable.getFunction(id), argc, args_vec);

			compare(RPAR);
			return ret;
		break;
		default:
			return symbolTable.getIdentifier(id);
	}

	return NULL_TREE;
}

tree Parser::Type() {
	tree ret = NULL_TREE;
	int from = 0;
	int to = 0;
	tree index_type = NULL_TREE;

	switch(nextToken.type) {
		case kwINTEGER:
			compare(kwINTEGER);
			ret = integer_type_node;
		break;
		case kwARRAY:
			compare(kwARRAY);
			compare(LSQBRAC);

			from = compareNumber();

			compare(STOP);
			compare(STOP);

			to = compareNumber();

			compare(RSQBRAC);
			compare(kwOF);
			compare(kwINTEGER);
			//return integer_type_node;
			//index_type = build_index_type(size_int(12));

			index_type = build_range_type(integer_type_node, build_int_cst( integer_type_node, from), build_int_cst( integer_type_node, to));
			ret = build_array_type(integer_type_node, index_type);

		break;
		default:
			break;
	}
	return ret;
}

void Parser::For_l(tree * stmlist, tree var) {
	tree ret = NULL_TREE;
	tree until = NULL_TREE;

	switch(nextToken.type) {
		case kwTO:
			compare(kwTO);
			until = Expr();
			compare(kwDO);
			append_to_statement_list( build1(EXIT_EXPR, void_type_node, build2(GT_EXPR, TREE_TYPE(var), var, until)), stmlist);
			append_to_statement_list(Command(), stmlist);
			append_to_statement_list( build2(POSTINCREMENT_EXPR, TREE_TYPE(var), var, build_int_cst(integer_type_node, 1)), stmlist );
		break;
		case kwDOWNTO:
			compare(kwDOWNTO);
			until = Expr();
			compare(kwDO);
			append_to_statement_list( build1(EXIT_EXPR, void_type_node, build2(LT_EXPR, TREE_TYPE(var), var, until)), stmlist);
			append_to_statement_list(Command(), stmlist);
			append_to_statement_list( build2(POSTDECREMENT_EXPR, TREE_TYPE(var), var, build_int_cst(integer_type_node, 1)), stmlist );

		break;
		default:
			break;
	}
}

const char * Parser::compareIdentifier() {
	if(nextToken.type != IDENT) {
		char msg[128];
		sprintf(msg, "Syntax error - unexpected %s, expecting %s", symbTable[nextToken.type], symbTable[IDENT]);
		error(msg, &nextToken);
	}

	const char * t = nextToken.identifier->text;
	nextToken = lex->readToken();
	D(nextToken.print(std::cout));
	return t;
}

int Parser::compareNumber() {
	if(nextToken.type != NUMB && nextToken.type != MINUS) {
		char msg[128];
		sprintf(msg, "Syntax error - unexpected %s, expecting %s", symbTable[nextToken.type], symbTable[NUMB]);
		error(msg, &nextToken);
	}

	bool negate = false;

	if(nextToken.type == MINUS) {
		compare(MINUS);
		negate = true;
	}

	int t = nextToken.value;
	nextToken = lex->readToken();
	D(nextToken.print(std::cout));

	if(negate) return -t;
	return t;
}
