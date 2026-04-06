#ifndef YCF_AST_H
#define YCF_AST_H
#include "ycf_utils.h"
#include "ycf_symbol.h"
#include "ycf_string.h"
#include "ycf_node.h"
typedef struct {
  bool success;
  ycf_symbol* next_symbol;
  ycf_node* result;
} ycf_parse_result;
void string_item_list_print(ycf_string_item_list n);
int ycf_symbol_list_get_item_position(ycf_symbol_list* list, ycf_symbol* node);
ycf_symbol* ycf_symbol_shallow_copy(ycf_symbol* n);
ycf_symbol* ycf_symbol_list_get_item_at_position(ycf_symbol_list* list, int pos);
void ycf_symbol_list_append(ycf_symbol_list* list, ycf_symbol* node);
void ycf_symbol_list_prepend(ycf_symbol_list* list, ycf_symbol* node);
void ycf_symbol_list_insert_before(ycf_symbol_list* list, ycf_symbol* before_this, ycf_symbol* to_insert);
void ycf_symbol_list_insert_after(ycf_symbol_list* list, ycf_symbol* after_this, ycf_symbol* to_insert);
void ycf_symbol_list_remove(ycf_symbol_list* list, ycf_symbol* to_remove);
void ycf_symbol_list_replace(ycf_symbol_list* list, ycf_symbol* to_replace, ycf_symbol* replace_with);
void ycf_symbol_list_concat(ycf_symbol_list* list1, ycf_symbol_list* list2);
ycf_symbol_list ycf_symbol_list_empty();
ycf_symbol_list ycf_symbol_list_shallow_copy(ycf_symbol_list n);
ycf_symbol_list ycf_symbol_list_copy_append(ycf_symbol_list list, ycf_symbol* node);
ycf_symbol_list ycf_symbol_list_copy_prepend(ycf_symbol_list list, ycf_symbol* node);
ycf_symbol_list ycf_symbol_list_copy_insert_before(ycf_symbol_list list, ycf_symbol* before_this, ycf_symbol* to_insert);
ycf_symbol_list ycf_symbol_list_copy_insert_after(ycf_symbol_list list, ycf_symbol* after_this, ycf_symbol* to_insert);
ycf_symbol_list ycf_symbol_list_copy_remove(ycf_symbol_list list, ycf_symbol* to_remove);
ycf_symbol_list ycf_symbol_list_copy_replace(ycf_symbol_list list, ycf_symbol* to_replace, ycf_symbol* replace_with);
ycf_symbol_list ycf_symbol_list_copy_concat(ycf_symbol_list list1, ycf_symbol_list list2);
ycf_node* ycf_node_scope_new(ycf_symbol* start,
                             ycf_node_list declaration_nodes,
                             ycf_node_list other_nodes,
                             ycf_symbol* end);
ycf_parse_result parse_expression(ycf_symbol* symbols);
void print_abstract_syntax_tree(ycf_node* node);
void print_node_code_paran_expression(ycf_node_parentheses_expression e, ycf_string_printable_buffer* b);
void print_node_code_expression(ycf_node_expression e, ycf_string_printable_buffer* b);
void ast_add_yield_code_generated_define(ycf_node* source_out_tree, bool debug_mode);
void print_symbol_list(ycf_symbol_list* l, ycf_string_printable_buffer* b);
ycf_node* ycf_node_find_function(ycf_node* c_file_node, char* fun_name);
ycf_node_list ycf_node_get_all_definitions_in_function(ycf_node_function* f);
ycf_node* ast_get_ast_with_yieldified_function(ycf_node* source_tree,
                                               ycf_node* header_tree,
                                               char* yielding_function_name,
                                               ycf_string_item_list* all_yielding_function_names,
                                               bool auto_yield,
                                               bool recusive_auto_yield,
                                               bool debug_mode,
                                               bool only_yielding_funs,
                                               ycf_node** only_yielding_funs_tree,
                                               bool static_aux_funs);
void print_abstract_syntax_tree(ycf_node* node);
void print_node_code_expression(ycf_node_expression e, ycf_string_printable_buffer* b);
void print_node_list_code(ycf_node* n, ycf_string_printable_buffer* b);
void ast_add_yield_code_generated_define(ycf_node* source_out_tree, bool debug_mode);
#endif