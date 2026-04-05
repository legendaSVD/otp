#ifndef ERL_NODE_CONTAINER_UTILS_H__
#define ERL_NODE_CONTAINER_UTILS_H__
#include "erl_ptab.h"
#define node_container_node_name(x)	(is_external(x)			\
					 ? external_node_name((x))	\
					 : internal_node_name((x)))
#define node_container_creation(x) 	(is_external(x)			\
					 ? external_creation((x))	\
					 : internal_creation((x)))
#define node_container_dist_entry(x) 	(is_external(x)			\
					 ? external_dist_entry((x))	\
					 : internal_dist_entry((x)))
#define node_container_channel_no(x)	(is_external((x))		\
					 ? external_channel_no((x))	\
					 : internal_channel_no((x)))
#define is_node_container(x)		(is_external((x)) || is_internal((x)))
#define is_not_node_container(x)	(!is_node_container((x)))
#define is_internal(x) 			(is_internal_pid((x))		\
					 || is_internal_port((x))	\
					 || is_internal_ref((x)))
#define is_not_internal(x) 		(!is_internal((x)))
#define internal_node_name(x)		(erts_this_node->sysname)
#define external_node_name(x)		external_node((x))->sysname
#define internal_creation(x)		(erts_this_node->creation)
#define external_creation(x)		(external_node((x))->creation)
#define internal_dist_entry(x)		(erts_this_node->dist_entry)
#define external_dist_entry(x)		(external_node((x))->dist_entry)
#define dist_entry_channel_no(x)				\
  ((x) == erts_this_dist_entry					\
   ? ((Uint) ERST_INTERNAL_CHANNEL_NO)							\
   : (ASSERT(is_atom((x)->sysname)),			        \
      (Uint) atom_val((x)->sysname)))
#define internal_channel_no(x) ((Uint) ERST_INTERNAL_CHANNEL_NO)
#define external_channel_no(x) \
  (dist_entry_channel_no(external_dist_entry((x))))
extern ErtsPTab erts_proc;
#define make_internal_pid(D)		erts_ptab_make_id(&erts_proc, \
							  (D), \
							  _TAG_IMMED1_PID)
#define internal_pid_index(PID)		(ASSERT(is_internal_pid((PID))), \
					 erts_ptab_id2pix(&erts_proc, (PID)))
#define internal_pid_data(PID)		(ASSERT(is_internal_pid((PID))), \
					 erts_ptab_id2data(&erts_proc, (PID)))
#define internal_pid_number(x)		_GET_PID_NUM(internal_pid_data((x)))
#define internal_pid_serial(x)		_GET_PID_SER(internal_pid_data((x)))
#define internal_pid_node_name(x)	(internal_pid_node((x))->sysname)
#define external_pid_node_name(x)	(external_pid_node((x))->sysname)
#define internal_pid_creation(x)	(internal_pid_node((x))->creation)
#define external_pid_creation(x)	(external_pid_node((x))->creation)
#define internal_pid_dist_entry(x)	(internal_pid_node((x))->dist_entry)
#define external_pid_dist_entry(x)	(external_pid_node((x))->dist_entry)
#define internal_pid_channel_no(x)	(internal_channel_no((x)))
#define external_pid_channel_no(x)	(external_channel_no((x)))
#define pid_data_words(x)		(is_internal_pid((x))		\
					 ? internal_pid_data_words((x))	\
					 : external_pid_data_words((x)))
#define pid_number(x)			(is_internal_pid((x))		\
					 ? internal_pid_number((x))	\
					 : external_pid_number((x)))
#define pid_serial(x)			(is_internal_pid((x))		\
					 ? internal_pid_serial((x))	\
					 : external_pid_serial((x)))
#define pid_node(x)			(is_internal_pid((x))		\
					 ? internal_pid_node((x))	\
					 : external_pid_node((x)))
#define pid_node_name(x)		(is_internal_pid((x))		\
					 ? internal_pid_node_name((x))	\
					 : external_pid_node_name((x)))
#define pid_creation(x)			(is_internal_pid((x))		\
					 ? internal_pid_creation((x))	\
					 : external_pid_creation((x)))
#define pid_dist_entry(x)		(is_internal_pid((x))		\
					 ? internal_pid_dist_entry((x))	\
					 : external_pid_dist_entry((x)))
#define pid_channel_no(x)		(is_internal_pid((x))		\
					 ? internal_pid_channel_no((x))	\
					 : external_pid_channel_no((x)))
#define is_pid(x)			(is_internal_pid((x))		\
					 || is_external_pid((x)))
#define is_not_pid(x)			(!is_pid(x))
#define ERTS_MAX_PROCESSES		(ERTS_PTAB_MAX_SIZE-1)
#define ERTS_MAX_INTERNAL_PID_DATA	((UWORD_CONSTANT(1) << _PID_DATA_SIZE) - 1)
#define ERTS_MAX_INTERNAL_PID_NUMBER	((UWORD_CONSTANT(1) << _PID_NUM_SIZE) - 1)
#define ERTS_MAX_INTERNAL_PID_SERIAL	((UWORD_CONSTANT(1) << _PID_SER_SIZE) - 1)
#define ERTS_INTERNAL_PROC_BITS		(_PID_SER_SIZE + _PID_NUM_SIZE)
#define ERTS_INVALID_PID		ERTS_PTAB_INVALID_ID(_TAG_IMMED1_PID)
extern ErtsPTab erts_port;
#define make_internal_port(D)		erts_ptab_make_id(&erts_port, \
							  (D), \
							  _TAG_IMMED1_PORT)
#define internal_port_index(PRT)	(ASSERT(is_internal_port((PRT))), \
					 erts_ptab_id2pix(&erts_port, (PRT)))
#define internal_port_data(PRT)		(ASSERT(is_internal_port((PRT))), \
					 erts_ptab_id2data(&erts_port, (PRT)))
#define internal_port_number(x) ((Uint64) _GET_PORT_NUM(internal_port_data((x))))
#define internal_port_node_name(x)	(internal_port_node((x))->sysname)
#define external_port_node_name(x)	(external_port_node((x))->sysname)
#define internal_port_creation(x)	(internal_port_node((x))->creation)
#define external_port_creation(x)	(external_port_node((x))->creation)
#define internal_port_dist_entry(x)	(internal_port_node((x))->dist_entry)
#define external_port_dist_entry(x)	(external_port_node((x))->dist_entry)
#define internal_port_channel_no(x)	(internal_channel_no((x)))
#define external_port_channel_no(x)	(external_channel_no((x)))
#define port_data_words(x)		(is_internal_port((x))		\
					 ? internal_port_data_words((x))\
					 : external_port_data_words((x)))
#define port_number(x)			(is_internal_port((x))		\
					 ? internal_port_number((x))	\
					 : external_port_number((x)))
#define port_node(x)			(is_internal_port((x))		\
					 ? internal_port_node((x))	\
					 : external_port_node((x)))
#define port_node_name(x)		(is_internal_port((x))		\
					 ? internal_port_node_name((x))	\
					 : external_port_node_name((x)))
#define port_creation(x)		(is_internal_port((x))		\
					 ? internal_port_creation((x))	\
					 : external_port_creation((x)))
#define port_dist_entry(x)		(is_internal_port((x))		\
					 ? internal_port_dist_entry((x))\
					 : external_port_dist_entry((x)))
#define port_channel_no(x)		(is_internal_port((x))		\
					 ? internal_port_channel_no((x))\
					 : external_port_channel_no((x)))
#define is_port(x)			(is_internal_port((x))		\
					 || is_external_port((x)))
#define is_not_port(x)			(!is_port(x))
#define ERTS_MAX_PORTS			(ERTS_PTAB_MAX_SIZE-1)
#define ERTS_MAX_PORT_DATA		((UWORD_CONSTANT(1) << _PORT_DATA_SIZE) - 1)
#define ERTS_MAX_INTERNAL_PORT_NUMBER	((UWORD_CONSTANT(1) << _PORT_NUM_SIZE) - 1)
#define ERTS_MAX_V3_PORT_NUMBER		((UWORD_CONSTANT(1) << 28) - 1)
#define ERTS_PORTS_BITS			(_PORT_NUM_SIZE)
#define ERTS_INVALID_PORT		ERTS_PTAB_INVALID_ID(_TAG_IMMED1_PORT)
#define internal_ref_no_numbers(x)      (is_internal_pid_ref((x))       \
                                         ? ERTS_PID_REF_NUMBERS         \
                                         : ERTS_REF_NUMBERS)
#define internal_ref_numbers(x)		(is_internal_magic_ref((x))     \
					 ? internal_magic_ref_numbers((x)) \
                                         : internal_non_magic_ref_numbers((x)))
#if defined(ARCH_64)
#define external_ref_no_numbers(x)					\
  (external_ref_data((x))[0])
#define external_thing_ref_no_numbers(thing)			        \
  (external_thing_ref_data(thing)[0])
#define external_ref_numbers(x)						\
  (&external_ref_data((x))[1])
#define external_thing_ref_numbers(thing)				\
  (&external_thing_ref_data(thing)[1])
#else
#define external_ref_no_numbers(x)	(external_ref_data_words((x)))
#define external_thing_ref_no_numbers(t) (external_thing_ref_data_words((t)))
#define external_ref_numbers(x)		(external_ref_data((x)))
#define external_thing_ref_numbers(t)   (external_thing_ref_data((t)))
#endif
#define internal_ref_node_name(x)	(internal_ref_node((x))->sysname)
#define external_ref_node_name(x)	(external_ref_node((x))->sysname)
#define internal_ref_creation(x)	(internal_ref_node((x))->creation)
#define external_ref_creation(x)	(external_ref_node((x))->creation)
#define internal_ref_dist_entry(x)	(internal_ref_node((x))->dist_entry)
#define external_ref_dist_entry(x)	(external_ref_node((x))->dist_entry)
#define internal_ref_channel_no(x)	(internal_channel_no((x)))
#define external_ref_channel_no(x)	(external_channel_no((x)))
#define ref_no_numbers(x)		(is_internal_ref((x))		\
					 ? internal_ref_no_numbers((x))\
					 : external_ref_no_numbers((x)))
#define ref_numbers(x)			(is_internal_ref((x))		\
					 ? internal_ref_numbers((x))	\
					 : external_ref_numbers((x)))
#define ref_node(x)			(is_internal_ref((x))		\
					 ? internal_ref_node(x)		\
					 : external_ref_node((x)))
#define ref_node_name(x)		(is_internal_ref((x))		\
					 ? internal_ref_node_name((x))	\
					 : external_ref_node_name((x)))
#define ref_creation(x)			(is_internal_ref((x))		\
					 ? internal_ref_creation((x))	\
					 : external_ref_creation((x)))
#define ref_dist_entry(x)		(is_internal_ref((x))		\
					 ? internal_ref_dist_entry((x))	\
					 : external_ref_dist_entry((x)))
#define ref_channel_no(x)		(is_internal_ref((x))		\
					 ? internal_ref_channel_no((x))	\
					 : external_ref_channel_no((x)))
#define is_ref(x)			(is_internal_ref((x))		\
					 || is_external_ref((x)))
#define is_not_ref(x)			(!is_ref(x))
#endif