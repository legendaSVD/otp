#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <erl_nif.h>
#define ASN1_OK 0
#define ASN1_ERROR -1
#define ASN1_COMPL_ERROR 1
#define ASN1_DECODE_ERROR 2
#define ASN1_TAG_ERROR -3
#define ASN1_LEN_ERROR -4
#define ASN1_INDEF_LEN_ERROR -5
#define ASN1_VALUE_ERROR -6
#define ASN1_CLASS 0xc0
#define ASN1_FORM 0x20
#define ASN1_CLASSFORM (ASN1_CLASS | ASN1_FORM)
#define ASN1_TAG 0x1f
#define ASN1_LONG_TAG 0x7f
#define ASN1_INDEFINITE_LENGTH 0x80
#define ASN1_SHORT_DEFINITE_LENGTH 0
#define ASN1_PRIMITIVE 0
#define ASN1_CONSTRUCTED 0x20
#define ASN1_NOVALUE 0
#define ASN1_SKIPPED 0
#define ASN1_OPTIONAL 1
#define ASN1_CHOOSEN 2
#define CEIL(X,Y) ((X-1) / Y + 1)
#define INVMASK(X,M) (X & (M ^ 0xff))
#define MASK(X,M) (X & M)
static int per_complete(ErlNifBinary *, unsigned char *, int);
static int per_insert_octets(int, unsigned char **, unsigned char **, int *);
static int per_insert_octets_except_unused(int, unsigned char **, unsigned char **,
	int *, int);
static int per_insert_octets_as_bits_exact_len(int, int, unsigned char **,
	unsigned char **, int *);
static int per_insert_octets_as_bits(int, unsigned char **, unsigned char **, int *);
static int per_pad_bits(int, unsigned char **, int *);
static int per_insert_least_sign_bits(int, unsigned char, unsigned char **, int *);
static int per_insert_most_sign_bits(int, unsigned char, unsigned char **, int *);
static int per_insert_bits_as_bits(int, int, unsigned char **, unsigned char **, int *);
static int per_insert_octets_unaligned(int, unsigned char **, unsigned char **, int);
static int per_realloc_memory(ErlNifBinary *, int, unsigned char **);
static int ber_decode_begin(ErlNifEnv *, ERL_NIF_TERM *, unsigned char *, int,
	unsigned int *);
static int ber_decode(ErlNifEnv *, ERL_NIF_TERM *, unsigned char *, int *, int);
static int ber_decode_tag(ErlNifEnv *, ERL_NIF_TERM *, unsigned char *, int, int *);
static int ber_decode_value(ErlNifEnv*, ERL_NIF_TERM *, unsigned char *, int *, int,
	int);
typedef struct ber_encode_mem_chunk mem_chunk_t;
static int ber_encode(ErlNifEnv *, ERL_NIF_TERM , mem_chunk_t **, unsigned int *);
static void ber_free_chunks(mem_chunk_t *chunk);
static mem_chunk_t *ber_new_chunk(unsigned int length);
static int ber_check_memory(mem_chunk_t **curr, unsigned int needed);
static int ber_encode_tag(ErlNifEnv *, ERL_NIF_TERM , unsigned int ,
	mem_chunk_t **, unsigned int *);
static int ber_encode_length(size_t , mem_chunk_t **, unsigned int *);
static int per_complete(ErlNifBinary *out_binary, unsigned char *in_buf,
	int in_buf_len) {
    int counter = in_buf_len;
    int buf_space = in_buf_len;
    int buf_size = in_buf_len;
    unsigned char *in_ptr, *ptr;
    int unused = 8;
    int no_bits, no_bytes, in_unused, desired_len, ret, saved_mem, needed,
	    pad_bits;
    unsigned char val;
    in_ptr = in_buf;
    ptr = out_binary->data;
    *ptr = 0x00;
    while (counter > 0) {
	counter--;
	switch (*in_ptr) {
	case 0:
	    if (unused == 1) {
		unused = 8;
		*++ptr = 0x00;
		buf_space--;
	    } else
		unused--;
	    break;
	case 1:
	    if (unused == 1) {
		*ptr = *ptr | 1;
		unused = 8;
		*++ptr = 0x00;
		buf_space--;
	    } else {
		*ptr = *ptr | (1 << (unused - 1));
		unused--;
	    }
	    break;
	case 2:
	    if (unused != 8) {
		*++ptr = 0x00;
		buf_space--;
		unused = 8;
	    }
	    break;
	case 10:
	    no_bits = (int) *(++in_ptr);
	    val = *(++in_ptr);
	    counter -= 2;
	    if ((ret = per_insert_least_sign_bits(no_bits, val, &ptr, &unused))
		    == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 20:
	    no_bytes = (int) *(++in_ptr);
	    counter -= (no_bytes + 1);
	    if ((counter < 0)
		    || (ret = per_insert_octets(no_bytes, &in_ptr, &ptr,
			    &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 21:
	    no_bytes = (int) *(++in_ptr);
	    no_bytes = no_bytes << 8;
	    no_bytes = no_bytes | (int) *(++in_ptr);
	    counter -= (2 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets(no_bytes, &in_ptr, &ptr,
			    &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 30:
	    in_unused = (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    counter -= (2 + no_bytes);
	    ret = -4711;
	    if ((counter < 0)
		    || (ret = per_insert_octets_except_unused(no_bytes, &in_ptr,
			    &ptr, &unused, in_unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 31:
	    in_unused = (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    no_bytes = no_bytes << 8;
	    no_bytes = no_bytes | (int) *(++in_ptr);
	    counter -= (3 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets_except_unused(no_bytes, &in_ptr,
			    &ptr, &unused, in_unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 40:
	    desired_len = (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    pad_bits = desired_len - no_bytes - unused;
	    needed = (pad_bits > 0) ? CEIL(pad_bits,8) : 0;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (2 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets_as_bits_exact_len(desired_len,
			    no_bytes, &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 41:
	    desired_len = (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    no_bytes = no_bytes << 8;
	    no_bytes = no_bytes | (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (3 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets_as_bits_exact_len(desired_len,
			    no_bytes, &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 42:
	    desired_len = (int) *(++in_ptr);
	    desired_len = desired_len << 8;
	    desired_len = desired_len | (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (3 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets_as_bits_exact_len(desired_len,
			    no_bytes, &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 43:
	    desired_len = (int) *(++in_ptr);
	    desired_len = desired_len << 8;
	    desired_len = desired_len | (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    no_bytes = no_bytes << 8;
	    no_bytes = no_bytes | (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (4 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_octets_as_bits_exact_len(desired_len,
			    no_bytes, &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 45:
	    desired_len = (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (2 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_bits_as_bits(desired_len, no_bytes,
			    &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 46:
	    desired_len = (int) *(++in_ptr);
	    desired_len = desired_len << 8;
	    desired_len = desired_len | (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (3 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_bits_as_bits(desired_len, no_bytes,
			    &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	case 47:
	    desired_len = (int) *(++in_ptr);
	    desired_len = desired_len << 8;
	    desired_len = desired_len | (int) *(++in_ptr);
	    no_bytes = (int) *(++in_ptr);
	    no_bytes = no_bytes << 8;
	    no_bytes = no_bytes | (int) *(++in_ptr);
	    saved_mem = buf_space - counter;
	    needed = CEIL((desired_len-unused),8) - no_bytes;
	    if (saved_mem < needed) {
		buf_size += needed;
		buf_space += needed;
		if (per_realloc_memory(out_binary, buf_size, &ptr) == ASN1_ERROR
		)
		    return ASN1_ERROR;
	    }
	    counter -= (4 + no_bytes);
	    if ((counter < 0)
		    || (ret = per_insert_bits_as_bits(desired_len, no_bytes,
			    &in_ptr, &ptr, &unused)) == ASN1_ERROR
		    )
		return ASN1_ERROR;
	    buf_space -= ret;
	    break;
	default:
	    return ASN1_ERROR;
	}
	in_ptr++;
    }
    if ((unused == 8) && (ptr != out_binary->data))
	return (ptr - out_binary->data);
    else {
	ptr++;
	return (ptr - out_binary->data);
    }
}
static int per_realloc_memory(ErlNifBinary *binary, int amount, unsigned char **ptr) {
    int i = *ptr - binary->data;
    if (!enif_realloc_binary(binary, amount)) {
	return ASN1_ERROR;
    } else {
	*ptr = binary->data + i;
    }
    return ASN1_OK;
}
static int per_insert_most_sign_bits(int no_bits, unsigned char val,
	unsigned char **output_ptr, int *unused) {
    unsigned char *ptr = *output_ptr;
    if (no_bits < *unused) {
	*ptr = *ptr | (val >> (8 - *unused));
	*unused -= no_bits;
    } else if (no_bits == *unused) {
	*ptr = *ptr | (val >> (8 - *unused));
	*unused = 8;
	*++ptr = 0x00;
    } else {
	*ptr = *ptr | (val >> (8 - *unused));
	*++ptr = 0x00;
	*ptr = *ptr | (val << *unused);
	*unused = 8 - (no_bits - *unused);
    }
    *output_ptr = ptr;
    return ASN1_OK;
}
static int per_insert_least_sign_bits(int no_bits, unsigned char val,
	unsigned char **output_ptr, int *unused) {
    unsigned char *ptr = *output_ptr;
    int ret = 0;
    if (no_bits < *unused) {
	*ptr = *ptr | (val << (*unused - no_bits));
	*unused -= no_bits;
    } else if (no_bits == *unused) {
	*ptr = *ptr | val;
	*unused = 8;
	*++ptr = 0x00;
	ret++;
    } else {
	*ptr = *ptr | (val >> (no_bits - *unused));
	*++ptr = 0x00;
	ret++;
	*ptr = *ptr | (val << (8 - (no_bits - *unused)));
	*unused = 8 - (no_bits - *unused);
    }
    *output_ptr = ptr;
    return ret;
}
static int per_pad_bits(int no_bits, unsigned char **output_ptr, int *unused) {
    unsigned char *ptr = *output_ptr;
    int ret = 0;
    while (no_bits > 0) {
	if (*unused == 1) {
	    *unused = 8;
	    *++ptr = 0x00;
	    ret++;
	} else
	    (*unused)--;
	no_bits--;
    }
    *output_ptr = ptr;
    return ret;
}
static int per_insert_bits_as_bits(int desired_no, int no_bytes,
	unsigned char **input_ptr, unsigned char **output_ptr, int *unused) {
    unsigned char *in_ptr = *input_ptr;
    unsigned char val;
    int no_bits, ret;
    if (desired_no == (no_bytes * 8)) {
	if (per_insert_octets_unaligned(no_bytes, &in_ptr, output_ptr, *unused)
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
	ret = no_bytes;
    } else if (desired_no < (no_bytes * 8)) {
	if (per_insert_octets_unaligned(desired_no / 8, &in_ptr, output_ptr,
		*unused) == ASN1_ERROR
	)
	    return ASN1_ERROR;
	val = *++in_ptr;
	no_bits = desired_no % 8;
	per_insert_most_sign_bits(no_bits, val, output_ptr, unused);
	ret = CEIL(desired_no,8);
    } else {
	if (per_insert_octets_unaligned(no_bytes, &in_ptr, output_ptr, *unused)
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
	per_pad_bits(desired_no - (no_bytes * 8), output_ptr, unused);
	ret = CEIL(desired_no,8);
    }
    *input_ptr = in_ptr;
    return ret;
}
static int per_insert_octets_as_bits_exact_len(int desired_len, int in_buff_len,
	unsigned char **in_ptr, unsigned char **ptr, int *unused) {
    int ret = 0;
    int ret2 = 0;
    if (desired_len == in_buff_len) {
	if ((ret = per_insert_octets_as_bits(in_buff_len, in_ptr, ptr, unused))
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
    } else if (desired_len > in_buff_len) {
	if ((ret = per_insert_octets_as_bits(in_buff_len, in_ptr, ptr, unused))
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
	if ((ret2 = per_pad_bits(desired_len - in_buff_len, ptr, unused))
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
    } else {
	if ((ret = per_insert_octets_as_bits(desired_len, in_ptr, ptr, unused))
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
	*in_ptr += (in_buff_len - desired_len);
    }
    return (ret + ret2);
}
static int per_insert_octets_as_bits(int no_bytes, unsigned char **input_ptr,
	unsigned char **output_ptr, int *unused) {
    unsigned char *in_ptr = *input_ptr;
    unsigned char *ptr = *output_ptr;
    int used_bits = 8 - *unused;
    while (no_bytes > 0) {
	switch (*++in_ptr) {
	case 0:
	    if (*unused == 1) {
		*unused = 8;
		*++ptr = 0x00;
	    } else
		(*unused)--;
	    break;
	case 1:
	    if (*unused == 1) {
		*ptr = *ptr | 1;
		*unused = 8;
		*++ptr = 0x00;
	    } else {
		*ptr = *ptr | (1 << (*unused - 1));
		(*unused)--;
	    }
	    break;
	default:
	    return ASN1_ERROR;
	}
	no_bytes--;
    }
    *input_ptr = in_ptr;
    *output_ptr = ptr;
    return ((used_bits + no_bytes) / 8);
}
static int per_insert_octets(int no_bytes, unsigned char **input_ptr,
	unsigned char **output_ptr, int *unused) {
    unsigned char *in_ptr = *input_ptr;
    unsigned char *ptr = *output_ptr;
    int ret = 0;
    if (*unused != 8) {
	*++ptr = 0x00;
	ret++;
	*unused = 8;
    }
    while (no_bytes > 0) {
	*ptr = *(++in_ptr);
	*++ptr = 0x00;
	no_bytes--;
    }
    *input_ptr = in_ptr;
    *output_ptr = ptr;
    return (ret + no_bytes);
}
static int per_insert_octets_unaligned(int no_bytes, unsigned char **input_ptr,
	unsigned char **output_ptr, int unused) {
    unsigned char *in_ptr = *input_ptr;
    unsigned char *ptr = *output_ptr;
    int n = no_bytes;
    unsigned char val;
    while (n > 0) {
	if (unused == 8) {
	    *ptr = *++in_ptr;
	    *++ptr = 0x00;
	} else {
	    val = *++in_ptr;
	    *ptr = *ptr | val >> (8 - unused);
	    *++ptr = 0x00;
	    *ptr = val << unused;
	}
	n--;
    }
    *input_ptr = in_ptr;
    *output_ptr = ptr;
    return no_bytes;
}
static int per_insert_octets_except_unused(int no_bytes, unsigned char **input_ptr,
	unsigned char **output_ptr, int *unused, int in_unused) {
    unsigned char *in_ptr = *input_ptr;
    unsigned char *ptr = *output_ptr;
    int val, no_bits;
    int ret = 0;
    if (in_unused == 0) {
	if ((ret = per_insert_octets_unaligned(no_bytes, &in_ptr, &ptr, *unused))
		== ASN1_ERROR
		)
	    return ASN1_ERROR;
    } else {
	if ((ret = per_insert_octets_unaligned(no_bytes - 1, &in_ptr, &ptr, *unused))
		!= ASN1_ERROR) {
	    val = (int) *(++in_ptr);
	    no_bits = 8 - in_unused;
	    if (no_bits < *unused) {
		*ptr = *ptr | (val >> (8 - *unused));
		*unused = *unused - no_bits;
	    } else if (no_bits == *unused) {
		*ptr = *ptr | (val >> (8 - *unused));
		*++ptr = 0x00;
		ret++;
		*unused = 8;
	    } else {
		*ptr = *ptr | (val >> (8 - *unused));
		*++ptr = 0x00;
		ret++;
		*ptr = *ptr | (val << *unused);
		*unused = 8 - (no_bits - *unused);
	    }
	} else
	    return ASN1_ERROR;
    }
    *input_ptr = in_ptr;
    *output_ptr = ptr;
    return ret;
}
static int ber_decode_begin(ErlNifEnv* env, ERL_NIF_TERM *term, unsigned char *in_buf,
	int in_buf_len, unsigned int *err_pos) {
    int maybe_ret;
    int ib_index = 0;
    unsigned char *rest_data;
    ERL_NIF_TERM decoded_term, rest;
    if ((maybe_ret = ber_decode(env, &decoded_term, in_buf, &ib_index,
	    in_buf_len)) <= ASN1_ERROR)
    {
	*err_pos = ib_index;
	return maybe_ret;
    };
    rest_data = enif_make_new_binary(env, in_buf_len - ib_index, &rest);
    memcpy(rest_data, in_buf+ib_index, in_buf_len - ib_index);
    *term = enif_make_tuple2(env, decoded_term, rest);
    return ASN1_OK;
}
static int ber_decode(ErlNifEnv* env, ERL_NIF_TERM *term, unsigned char *in_buf,
	int *ib_index, int in_buf_len) {
    int maybe_ret;
    int form;
    ERL_NIF_TERM tag, value;
    if ((*ib_index + 2) > in_buf_len)
	return ASN1_VALUE_ERROR;
    if ((form = ber_decode_tag(env, &tag, in_buf, in_buf_len, ib_index))
	    <= ASN1_ERROR
	    )
	return form;
    if (*ib_index >= in_buf_len) {
	return ASN1_TAG_ERROR;
    }
    if ((maybe_ret = ber_decode_value(env, &value, in_buf, ib_index, form,
	    in_buf_len)) <= ASN1_ERROR
    )
	return maybe_ret;
    *term = enif_make_tuple2(env, tag, value);
    return ASN1_OK;
}
static int ber_decode_tag(ErlNifEnv* env, ERL_NIF_TERM *tag, unsigned char *in_buf,
	int in_buf_len, int *ib_index) {
    int tag_no, tmp_tag, form;
    tag_no = ((MASK(in_buf[*ib_index],ASN1_CLASS)) << 10);
    form = (MASK(in_buf[*ib_index],ASN1_FORM));
    if ((tmp_tag = (int) INVMASK(in_buf[*ib_index],ASN1_CLASSFORM)) < 31) {
	*tag = enif_make_uint(env, tag_no | tmp_tag);
	(*ib_index)++;
    } else {
	if ((*ib_index + 3) > in_buf_len)
	    return ASN1_VALUE_ERROR;
	(*ib_index)++;
	if ((tmp_tag = (int) in_buf[*ib_index]) >= 128) {
	    tag_no = tag_no | (MASK(tmp_tag,ASN1_LONG_TAG) << 7);
	    (*ib_index)++;
	}
        tmp_tag = (int) in_buf[*ib_index];
	if (tmp_tag >= 128) {
	    return ASN1_TAG_ERROR;
        }
	tag_no = tag_no | tmp_tag;
	(*ib_index)++;
	*tag = enif_make_uint(env, tag_no);
    }
    return form;
}
static int ber_decode_value(ErlNifEnv* env, ERL_NIF_TERM *value, unsigned char *in_buf,
	int *ib_index, int form, int in_buf_len) {
    int maybe_ret;
    unsigned int len = 0;
    unsigned int lenoflen = 0;
    unsigned char *tmp_out_buff;
    ERL_NIF_TERM term = 0, curr_head = 0;
    maybe_ret = (int) (ErlNifSInt) ((char *)value - (char *)ib_index);
    maybe_ret = maybe_ret < 0 ? -maybe_ret : maybe_ret;
    if (maybe_ret >= sizeof(void *) * 8192)
        return ASN1_ERROR;
    if (((in_buf[*ib_index]) & 0x80) == ASN1_SHORT_DEFINITE_LENGTH) {
	len = in_buf[*ib_index];
    } else if (in_buf[*ib_index] == ASN1_INDEFINITE_LENGTH) {
	(*ib_index)++;
	curr_head = enif_make_list(env, 0);
	if (*ib_index+1 >= in_buf_len || form == ASN1_PRIMITIVE) {
	    return ASN1_INDEF_LEN_ERROR;
	}
	while (!(in_buf[*ib_index] == 0 && in_buf[*ib_index + 1] == 0)) {
	    maybe_ret = ber_decode(env, &term, in_buf, ib_index, in_buf_len);
	    if (maybe_ret <= ASN1_ERROR) {
		return maybe_ret;
	    }
	    curr_head = enif_make_list_cell(env, term, curr_head);
	    if (*ib_index+1 >= in_buf_len) {
		return ASN1_INDEF_LEN_ERROR;
	    }
	}
	enif_make_reverse_list(env, curr_head, value);
	(*ib_index) += 2;
	return ASN1_OK;
    } else {
	lenoflen = (in_buf[*ib_index] & 0x7f);
	if (lenoflen > (in_buf_len - (*ib_index + 1)))
	    return ASN1_LEN_ERROR;
	len = 0;
	while (lenoflen--) {
	    (*ib_index)++;
	    if (!(len < (1 << (sizeof(len) - 1) * 8)))
		return ASN1_LEN_ERROR;
	    len = (len << 8) + in_buf[*ib_index];
	}
    }
    if (len > (in_buf_len - (*ib_index + 1)))
	return ASN1_VALUE_ERROR;
    (*ib_index)++;
    if (form == ASN1_CONSTRUCTED) {
	int end_index = *ib_index + len;
	if (end_index > in_buf_len)
	    return ASN1_LEN_ERROR;
	curr_head = enif_make_list(env, 0);
	while (*ib_index < end_index) {
	    if ((maybe_ret = ber_decode(env, &term, in_buf, ib_index,
                   end_index )) <= ASN1_ERROR
	    )
		return maybe_ret;
	    curr_head = enif_make_list_cell(env, term, curr_head);
	}
	enif_make_reverse_list(env, curr_head, value);
    } else {
	if ((*ib_index + len) > in_buf_len)
	    return ASN1_LEN_ERROR;
	tmp_out_buff = enif_make_new_binary(env, len, value);
	memcpy(tmp_out_buff, in_buf + *ib_index, len);
	*ib_index = *ib_index + len;
    }
    return ASN1_OK;
}
struct ber_encode_mem_chunk {
    mem_chunk_t *next;
    int length;
    char *top;
    char *curr;
};
static int ber_encode(ErlNifEnv *env, ERL_NIF_TERM term, mem_chunk_t **curr, unsigned int *count) {
    const ERL_NIF_TERM *tv;
    unsigned int form;
    int arity;
    if (!enif_get_tuple(env, term, &arity, &tv))
	return ASN1_ERROR;
    form = enif_is_list(env, tv[1]) ? ASN1_CONSTRUCTED : ASN1_PRIMITIVE;
    switch (form) {
    case ASN1_PRIMITIVE: {
	ErlNifBinary value;
	if (!enif_inspect_binary(env, tv[1], &value))
	    return ASN1_ERROR;
	if (ber_check_memory(curr, value.size))
	    return ASN1_ERROR;
	memcpy((*curr)->curr - value.size + 1, value.data, value.size);
	(*curr)->curr -= value.size;
	*count += value.size;
	if (ber_encode_length(value.size, curr, count))
	    return ASN1_ERROR;
	break;
    }
    case ASN1_CONSTRUCTED: {
	ERL_NIF_TERM head, tail;
	unsigned int tmp_cnt;
	if(!enif_make_reverse_list(env, tv[1], &head))
	    return ASN1_ERROR;
	if (!enif_get_list_cell(env, head, &head, &tail)) {
	    if (enif_is_empty_list(env, tv[1])) {
		*((*curr)->curr) = 0;
		(*curr)->curr -= 1;
		(*count)++;
		break;
	    } else
		return ASN1_ERROR;
	}
	do {
	    tmp_cnt = 0;
	    if (ber_encode(env, head, curr, &tmp_cnt)) {
		return ASN1_ERROR;
	    }
	    *count += tmp_cnt;
	} while (enif_get_list_cell(env, tail, &head, &tail));
	if (ber_check_memory(curr, *count)) {
	    return ASN1_ERROR;
	}
	if (ber_encode_length(*count, curr, count)) {
	    return ASN1_ERROR;
	}
	break;
    }
    }
    if (ber_check_memory(curr, 3))
	return ASN1_ERROR;
    if (ber_encode_tag(env, tv[0], form, curr, count))
	return ASN1_ERROR;
    return ASN1_OK;
}
static int ber_encode_tag(ErlNifEnv *env, ERL_NIF_TERM tag, unsigned int form,
	mem_chunk_t **curr, unsigned int *count) {
    unsigned int class_tag_no, head_tag;
    if (!enif_get_uint(env, tag, &class_tag_no))
	return ASN1_ERROR;
    head_tag = form | ((class_tag_no & 0x30000) >> 10);
    class_tag_no = class_tag_no & 0xFFFF;
    if (class_tag_no <= 30) {
	*(*curr)->curr = head_tag | class_tag_no;
	(*curr)->curr -= 1;
	(*count)++;
	return ASN1_OK;
    } else {
	*(*curr)->curr = class_tag_no & 127;
	class_tag_no = class_tag_no >> 7;
	(*curr)->curr -= 1;
	(*count)++;
	while (class_tag_no > 0) {
	    *(*curr)->curr = (class_tag_no & 127) | 0x80;
	    class_tag_no >>= 7;
	    (*curr)->curr -= 1;
	    (*count)++;
	}
	*(*curr)->curr = head_tag | 0x1F;
	(*curr)->curr -= 1;
	(*count)++;
	return ASN1_OK;
    }
}
static int ber_encode_length(size_t size, mem_chunk_t **curr, unsigned int *count) {
    if (size < 128) {
	if (ber_check_memory(curr, 1u))
	    return ASN1_ERROR;
	*(*curr)->curr = size;
	(*curr)->curr -= 1;
	(*count)++;
    } else {
	int chunks = 0;
	if (ber_check_memory(curr, 8))
	    return ASN1_ERROR;
	while (size > 0)
	{
	    *(*curr)->curr = size & 0xFF;
	    size >>= 8;
	    (*curr)->curr -= 1;
	    (*count)++;
	    chunks++;
	}
	*(*curr)->curr = chunks | 0x80;
	(*curr)->curr -= 1;
	(*count)++;
    }
    return ASN1_OK;
}
static mem_chunk_t *ber_new_chunk(unsigned int length) {
    mem_chunk_t *new = enif_alloc(sizeof(mem_chunk_t));
    if (new == NULL)
	return NULL;
    new->next = NULL;
    new->top = enif_alloc(sizeof(char) * length);
    if (new->top == NULL) {
	enif_free(new);
	return NULL;
    }
    new->curr = new->top + length - 1;
    new->length = length;
    return new;
}
static void ber_free_chunks(mem_chunk_t *chunk) {
    mem_chunk_t *curr, *next = chunk;
    while (next != NULL) {
	curr = next;
	next = curr->next;
	enif_free(curr->top);
	enif_free(curr);
    }
}
static int ber_check_memory(mem_chunk_t **curr, unsigned int needed) {
    mem_chunk_t *new;
    if ((*curr)->curr-needed >= (*curr)->top)
	return ASN1_OK;
    if ((new = ber_new_chunk((*curr)->length > needed ? (*curr)->length * 2 : (*curr)->length + needed)) == NULL)
	return ASN1_ERROR;
    new->next = *curr;
    *curr = new;
    return ASN1_OK;
}
static ERL_NIF_TERM encode_per_complete(ErlNifEnv* env, int argc,
	const ERL_NIF_TERM argv[]) {
    ERL_NIF_TERM err_code;
    ErlNifBinary in_binary;
    ErlNifBinary out_binary;
    int complete_len;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &in_binary))
	return enif_make_badarg(env);
    if (!enif_alloc_binary(in_binary.size, &out_binary))
	return enif_make_atom(env, "alloc_binary_failed");
    if (in_binary.size == 0)
	return enif_make_binary(env, &out_binary);
    if ((complete_len = per_complete(&out_binary, in_binary.data,
	    in_binary.size)) <= ASN1_ERROR) {
	enif_release_binary(&out_binary);
	if (complete_len == ASN1_ERROR
	)
	    err_code = enif_make_uint(env, '1');
	else
	    err_code = enif_make_uint(env, 0);
	return enif_make_tuple2(env, enif_make_atom(env, "error"), err_code);
    }
    if (complete_len < out_binary.size)
	enif_realloc_binary(&out_binary, complete_len);
    return enif_make_binary(env, &out_binary);
}
static ERL_NIF_TERM
make_ber_error_term(ErlNifEnv* env, unsigned int return_code,
		    unsigned int err_pos)
{
    ERL_NIF_TERM reason;
    ERL_NIF_TERM t;
    switch (return_code) {
    case ASN1_TAG_ERROR:
	reason = enif_make_atom(env, "invalid_tag");
	break;
    case ASN1_LEN_ERROR:
    case ASN1_INDEF_LEN_ERROR:
	reason = enif_make_atom(env, "invalid_length");
	break;
    case ASN1_VALUE_ERROR:
	reason = enif_make_atom(env, "invalid_value");
	break;
    default:
	reason = enif_make_atom(env, "unknown");
	break;
    }
    t = enif_make_tuple2(env, reason, enif_make_int(env, err_pos));
    return enif_make_tuple2(env, enif_make_atom(env, "error"), t);
}
static ERL_NIF_TERM decode_ber_tlv_raw(ErlNifEnv* env, int argc,
	const ERL_NIF_TERM argv[]) {
    ErlNifBinary in_binary;
    ERL_NIF_TERM return_term;
    unsigned int err_pos = 0, return_code;
    if (!enif_inspect_iolist_as_binary(env, argv[0], &in_binary))
	return enif_make_badarg(env);
    return_code = ber_decode_begin(env, &return_term, in_binary.data,
				   in_binary.size, &err_pos);
    if (return_code != ASN1_OK) {
	return make_ber_error_term(env, return_code, err_pos);
    }
    return return_term;
}
static ERL_NIF_TERM encode_ber_tlv(ErlNifEnv* env, int argc,
	const ERL_NIF_TERM argv[]) {
    ErlNifBinary out_binary;
    unsigned int length = 0, pos = 0;
    int encode_err;
    mem_chunk_t *curr, *top;
    ERL_NIF_TERM err_code;
    curr = ber_new_chunk(40);
    if (!curr) {
        err_code = enif_make_atom(env,"oom");
        goto err;
    }
    encode_err = ber_encode(env, argv[0], &curr, &length);
    if (encode_err <= ASN1_ERROR) {
	err_code = enif_make_int(env, encode_err);
	goto err;
    }
    if (!enif_alloc_binary(length, &out_binary)) {
        err_code = enif_make_atom(env,"oom");
        goto err;
    }
    top = curr;
    while (curr != NULL) {
	length = curr->length - (curr->curr-curr->top) -1;
	if (length > 0)
	    memcpy(out_binary.data + pos, curr->curr+1, length);
	pos += length;
	curr = curr->next;
    }
    ber_free_chunks(top);
    return enif_make_binary(env, &out_binary);
 err:
    ber_free_chunks(curr);
    return enif_make_tuple2(env, enif_make_atom(env, "error"), err_code);
}
static int is_ok_load_info(ErlNifEnv* env, ERL_NIF_TERM load_info) {
    int i;
    return enif_get_int(env, load_info, &i) && i == 1;
}
static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {
    if (!is_ok_load_info(env, load_info))
	return -1;
    return 0;
}
static int upgrade(ErlNifEnv* env, void** priv_data, void** old_priv_data,
	ERL_NIF_TERM load_info) {
    if (!is_ok_load_info(env, load_info))
	return -1;
    return 0;
}
static void unload(ErlNifEnv* env, void* priv_data) {
}
static ErlNifFunc nif_funcs[] =  {
    { "encode_per_complete", 1, encode_per_complete },
    { "decode_ber_tlv_raw", 1, decode_ber_tlv_raw },
    { "encode_ber_tlv", 1, encode_ber_tlv },
};
ERL_NIF_INIT(asn1rt_nif, nif_funcs, load, NULL, upgrade, unload)