#include "generic.h"

/*
tree Generic::createInt(const string & id, bool isGlobal, bool isConst) {
	tree xDecl = build_decl(UNKNOWN_LOCATION, (isConst ? CONST_DECL : VAR_DECL), get_identifier(id.c_str()), integer_type_node);

	TREE_ADDRESSABLE(xDecl) = true;
	TREE_USED(xDecl) = true;

	if (isGlobal) {
		TREE_STATIC(xDecl) = true;
		TREE_PUBLIC(xDecl) = true;
	}

	DECL_INITIAL(xDecl) = build_int_cst(type, 0);

	if(isGlobal) {
		register_global_variable_declaration(xDecl);
	}

	return xDecl;
}
*/

tree Generic::createVariable(const string & id, bool isGlobal, tree type, bool isConst) {
	tree xDecl = build_decl(UNKNOWN_LOCATION, (isConst ? CONST_DECL : VAR_DECL), get_identifier(id.c_str()), type);

	TREE_ADDRESSABLE(xDecl) = true;
	TREE_USED(xDecl) = true;

	if (isGlobal) {
		TREE_STATIC(xDecl) = true;
		TREE_PUBLIC(xDecl) = true;
	}

	//DECL_INITIAL(xDecl) = build_int_cst(type, 0);

	if(isGlobal) {
		register_global_variable_declaration(xDecl);
	}

	return xDecl;
}

tree Generic::createCall ( tree fndecl, int arg_cnt, tree * args_vec ) {
	tree call = build_call_expr_loc_array( UNKNOWN_LOCATION, fndecl , arg_cnt , args_vec ) ;
	TREE_USED(call) = true;
	return call;
}

tree Generic::createPrint (tree int_expr, bool newLine) {

	tree str;

	//UGLY HACK
	if(TREE_TYPE(build_string_literal (1+1, "a")) == TREE_TYPE(int_expr)) {
		if(newLine) {
			str = build_string_literal (4+1, "%s\n");
		} else {
			str = build_string_literal (2+1, "%s");
		}
	} else {
		if(newLine) {
			str = build_string_literal (4+1, "%d\n");
		} else {
			str = build_string_literal (2+1, "%d");
		}
	}

	tree fndecl =  build_fn_decl ("printf", build_function_type_list (integer_type_node, build_pointer_type (char_type_node), integer_type_node, NULL_TREE));
	DECL_EXTERNAL (fndecl) = true;
	DECL_ARTIFICIAL (fndecl) = false;
	TREE_STATIC (fndecl) = false;
	DECL_CONTEXT (fndecl) = NULL_TREE;

	tree * args_vec = XNEWVEC(tree, 2);
	args_vec[0] = str;
	args_vec[1] = int_expr;

	tree call = build_call_expr_loc_array( UNKNOWN_LOCATION, fndecl , 2 , args_vec ) ;
	TREE_USED(call) = true;
	return call;

	/*
		va_list va;
		va_start ( va, arg_cnt );
		tree * args_vec = XNEWVEC(tree, arg_cnt);
		int i;
		for (int i=0; i<arg_cnt; i++)
		 args_vec[i] = va_arg(va,tree);
		va_end ( va );
		tree call = build_call_expr_loc_array( UNKNOWN_LOCATION, fndecl , arg_cnt , args_vec ) ;
		TREE_USED(call) = true;
		return call;
		*/
	/*

	tree * args_vec = XNEWVEC( tree, 2 );
	args_vec[0] = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(str)), str);
	args_vec[1] = int_expr;

	tree params = NULL_TREE;
	chainon( params, tree_cons (NULL_TREE, TREE_TYPE(args_vec[0]), NULL_TREE) );
	chainon( params, tree_cons (NULL_TREE, TREE_TYPE(args_vec[1]), NULL_TREE) );

	// function parameters
	tree param_decl = NULL_TREE;

	tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;

	tree fntype = build_function_type( TREE_TYPE(resdecl), params );
	tree fndecl = build_decl( UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("printf"), fntype );
	DECL_ARGUMENTS(fndecl) = param_decl;

	DECL_RESULT( fndecl ) = resdecl;

	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;
	DECL_EXTERNAL( fndecl ) = true;

	location_t loc = DECL_SOURCE_LOCATION (int_expr);

	tree call = build_call_expr_loc_array( loc, fndecl, 2, args_vec );
	SET_EXPR_LOCATION(call, loc);
	TREE_USED(call) = true;
	return call;
	*/

}


tree Generic::createFunctionDecl(const char *name, bool external, tree function_type) {
	tree fndecl =  build_fn_decl (name, function_type);
	DECL_EXTERNAL (fndecl) = external;
	DECL_ARTIFICIAL (fndecl) = false;
	TREE_STATIC (fndecl) = !external;
	DECL_CONTEXT (fndecl) = NULL_TREE;

	return fndecl;
}

tree Generic::createScan(tree var_expr) {
	tree str = build_string_literal( 2 + 1, "%d");

	tree * args_vec = XNEWVEC( tree, 2 );
	args_vec[0] = str;
	args_vec[1] = build1(ADDR_EXPR, build_pointer_type(TREE_TYPE(var_expr)), var_expr);

	tree params = NULL_TREE;
	chainon( params, tree_cons (NULL_TREE, TREE_TYPE(args_vec[0]), NULL_TREE) );
	chainon( params, tree_cons (NULL_TREE, TREE_TYPE(args_vec[1]), NULL_TREE) );

	// function parameters
	tree param_decl = NULL_TREE;

	tree resdecl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, integer_type_node);
	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;

	tree fntype = build_function_type( TREE_TYPE(resdecl), params );
	tree fndecl = build_decl( UNKNOWN_LOCATION, FUNCTION_DECL, get_identifier("scanf"), fntype );
	DECL_ARGUMENTS(fndecl) = param_decl;

	DECL_RESULT( fndecl ) = resdecl;

	DECL_ARTIFICIAL(resdecl) = true;
	DECL_IGNORED_P(resdecl) = true;
	DECL_EXTERNAL( fndecl ) = true;

	tree call = build_call_expr_loc_array( UNKNOWN_LOCATION, fndecl, 2, args_vec );
	//SET_EXPR_LOCATION(call, UNKNOWN_LOCATION);
	TREE_USED(call) = true;

	return call;
}