#include "sfe-lang.h"
#include "sfe1.h"

struct GTY(()) lang_identifier {
	struct tree_identifier common;
};

union GTY((desc("TREE_CODE(&%h.node)"))) lang_tree_node {
	union GTY((tag ("1"))) tree_node node;
};

struct GTY(()) lang_type {
	char dummy;
};

/* Language-specific declaration information.  */

struct GTY(()) lang_decl {
	char dummy;
};

struct GTY(()) language_function {
	char dummy;
};

vec<tree, va_gc> * sfe_global_decls_vec = NULL;

/* language hooks */
struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

/* Initialization routine for the sfe1 (compiler) options */
void sfe_init_options (unsigned int, struct cl_decoded_option *) {
  /* CL_sfe is automatically generated by the GCC build process */
  return;
}

/* handle specific option - called by opts.c */
bool sfe_handle_option (size_t scode, const char *arg,  int value ATTRIBUTE_UNUSED, int kind ATTRIBUTE_UNUSED, location_t loc ATTRIBUTE_UNUSED, const struct cl_option_handlers *handlers ATTRIBUTE_UNUSED) {
	enum opt_code code = (enum opt_code) scode;
	printf("Option: %s\n", arg); //zde by se měly řešit argumenty. Viz gcc/c-family/c-opts.c

	return true;
}

/* The language hooks gets called whenever all options and arguments are
 * parsed/read in the by options sub-component. This might be used for
 * further validation checks
 *
 * If this function returns NOT 0, the middle-end/back-end of GCC WON'T
 * be called (this is for example used to emit pre-processing code only in
 * the C/C++ front-end)
 */
bool sfe_post_options (const char **) {
	flag_excess_precision_cmdline = EXCESS_PRECISION_FAST;

	return 0;
}

/* This language hook gets called in decode_options to determine the front-end language mask.
 * It should return the corresponding CL_*, in this case CL_sfe.
 */
unsigned int sfe_option_lang_mask(void) {
	return CL_sfe;
}

/* language dependent parser setup  */
bool sfe_init (void) {
	build_common_tree_nodes (flag_signed_char, false);

	return true;
}

// copy of c_genericize function from c-family/c-gimplify.c
void tree_dump_original (tree fndecl) {
	FILE *dump_orig;
	int local_dump_flags;
	struct cgraph_node *cgn;

	/* Dump the C-specific tree IR.  */
	dump_orig = dump_begin (TDI_original, &local_dump_flags);
	if (dump_orig) {
		fprintf (dump_orig, "\n;; Function %s", lang_hooks.decl_printable_name (fndecl, 2));
		fprintf (dump_orig, " (%s)\n", (!DECL_ASSEMBLER_NAME_SET_P (fndecl) ? "null" : IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (fndecl))));
		fprintf (dump_orig, ";; enabled by -%s\n", dump_flag_name (TDI_original));
		fprintf (dump_orig, "\n");

		if (local_dump_flags & TDF_RAW) dump_node (DECL_SAVED_TREE (fndecl), TDF_SLIM | local_dump_flags, dump_orig);
		else {
			struct function fn;
			fn.decl = fndecl;
			fn.curr_properties = 0;
			fn.cfg = NULL;
			DECL_STRUCT_FUNCTION(fndecl) = &fn;
			dump_function_to_file(fndecl, dump_orig, 0);
			DECL_STRUCT_FUNCTION(fndecl) = NULL;
		}
		fprintf (dump_orig, "\n");
		dump_end (TDI_original, dump_orig);
	}

	/* Dump all nested functions now.  */
	cgn = cgraph_node::get_create (fndecl);
	for (cgn = cgn->nested; cgn ; cgn = cgn->next_nested) tree_dump_original (cgn->decl);
}

void register_global_function_declaration(tree functionDecl) {
	vec_safe_push( sfe_global_decls_vec, functionDecl );

	tree_dump_original(functionDecl);

	// Prepare the function for the GCC middle-end
	gimplify_function_tree(functionDecl);
	cgraph_node::finalize_function(functionDecl, false);
}

void register_global_variable_declaration(tree variableDecl) {
	vec_safe_push( sfe_global_decls_vec, variableDecl );
}

#include "parser/parser.h"

/* parsing language hook */
void sfe_parse_file () {
	tree mainFunc = NULL_TREE;
	for ( unsigned i = 0; i < num_in_fnames ; i ++) {
		Lexan lex(in_fnames[i]);
		Parser parser(&lex);
		if(parser.parse(&mainFunc)) {
			printf("Syntax OK\n");
		} else {
			printf("Syntax ERROR\n");
		}

		printf("######################################\n");
	}

	register_global_function_declaration(mainFunc);

	return;
}

/* language dependent wrapup */
void sfe_finish (void) {
}

static bool sfe_mark_addressable (tree /*exp*/) {
	gcc_unreachable ();
}

static tree sfe_type_for_size (unsigned /*precision*/, int /*unsignedp*/) {
	gcc_unreachable ();
}

static tree sfe_type_for_mode (enum machine_mode /*mode*/, int /*unsignedp*/) {
	gcc_unreachable ();
}

static tree pushdecl (tree /*decl*/) {
	gcc_unreachable ();
}

static tree getdecls (void) {
	return NULL;
}

/* flush global declarations */
void sfe_write_globals (void) {

	tree *vec = vec_safe_address(sfe_global_decls_vec);
	int len = vec_safe_length(sfe_global_decls_vec);
	wrapup_global_declarations (vec, len);
	emit_debug_global_declarations (vec, len);
	vec_free (sfe_global_decls_vec);

	symtab->finalize_compilation_unit( );
}

static bool global_bindings_p (void) {
	gcc_unreachable ();
}

tree convert (tree /*type*/, tree /*expr*/) {
	return NULL;
}

#include "debug.h"
#include "gt-sfe-sfe1.h"
#include "gtype-sfe.h"