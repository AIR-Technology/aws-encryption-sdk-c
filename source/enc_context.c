/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <aws/cryptosdk/enc_context.h>
#include <aws/cryptosdk/error.h>

int aws_array_list_compare_hash_elements_by_key_string(const void * elem_a, const void * elem_b) {
    const struct aws_hash_element * a = (const struct aws_hash_element *)elem_a;
    const struct aws_hash_element * b = (const struct aws_hash_element *)elem_b;
    const struct aws_string * key_a = (const struct aws_string *)a->key;
    const struct aws_string * key_b = (const struct aws_string *)b->key;
    return aws_string_compare(key_a, key_b);
}

int aws_hash_table_get_elems_array_init(struct aws_allocator * alloc,
                                        struct aws_array_list * elems,
                                        const struct aws_hash_table * map) {
    size_t entry_count = aws_hash_table_get_entry_count(map);
    if (aws_array_list_init_dynamic(elems, alloc, entry_count, sizeof(struct aws_hash_element))) {
        return AWS_OP_ERR;
    }

    for (struct aws_hash_iter iter = aws_hash_iter_begin(map);
         !aws_hash_iter_done(&iter); aws_hash_iter_next(&iter)) {
        if (aws_array_list_push_back(elems, (void **) &iter.element)) {
            aws_array_list_clean_up(elems);
            return AWS_OP_ERR;
        }
    }
    if (aws_array_list_length(elems) != entry_count) {
        aws_array_list_clean_up(elems);
        return aws_raise_error(AWS_ERROR_UNKNOWN);
    }
    return AWS_OP_SUCCESS;
}

int aws_cryptosdk_serialize_enc_context_init(struct aws_allocator * alloc,
                                             struct aws_byte_buf * output,
                                             const struct aws_hash_table * enc_context) {
    size_t num_elems = aws_hash_table_get_entry_count(enc_context);
    if (num_elems > UINT16_MAX) return aws_raise_error(AWS_CRYPTOSDK_ERR_SERIALIZATION);

    struct aws_array_list elems;
    if (aws_hash_table_get_elems_array_init(alloc, & elems, enc_context)) return AWS_OP_ERR;

    aws_array_list_sort(&elems, aws_array_list_compare_hash_elements_by_key_string);

    size_t serialized_len = 2; // First two bytes are the number of key-value pairs.
    for (int idx = 0; idx < num_elems; ++idx) {
        struct aws_hash_element elem;
        if (aws_array_list_get_at(&elems, (void *)&elem, idx)) {
            aws_array_list_clean_up(&elems);
            return AWS_OP_ERR;
        }
        serialized_len += 4; // Two bytes each for key length and value length fields.
        size_t key_len = ((const struct aws_string *)elem.key)->len;
        size_t value_len = ((const struct aws_string *)elem.value)->len;
        if (key_len > UINT16_MAX || value_len > UINT16_MAX) goto SER_ERR;
        serialized_len += key_len + value_len;
    }
    // This limit is not strictly necessary, but other Encryption SDK implementations use it.
    if (serialized_len > UINT16_MAX) goto SER_ERR;

    if (aws_byte_buf_init(alloc, output, serialized_len)) {
        aws_array_list_clean_up(&elems);
        return AWS_OP_ERR;
    }
    output->len = serialized_len;
    struct aws_byte_cursor cur = aws_byte_cursor_from_buf(output);

    if (!aws_byte_cursor_write_be16(&cur, num_elems)) goto WRITE_ERR;

    for (int idx = 0; idx < num_elems; ++idx) {
        struct aws_hash_element elem;
        if (aws_array_list_get_at(&elems, (void *)&elem, idx)) {
            aws_array_list_clean_up(&elems);
            return AWS_OP_ERR;
        }
        const struct aws_string * key = (const struct aws_string *)elem.key;
        const struct aws_string * value = (const struct aws_string *)elem.value;
        if (!aws_byte_cursor_write_be16(&cur, key->len)) goto WRITE_ERR;
        if (!aws_byte_cursor_write_from_whole_string(&cur, key)) goto WRITE_ERR;
        if (!aws_byte_cursor_write_be16(&cur, value->len)) goto WRITE_ERR;
        if (!aws_byte_cursor_write_from_whole_string(&cur, value)) goto WRITE_ERR;
    }
    aws_array_list_clean_up(&elems);
    return AWS_OP_SUCCESS;

SER_ERR:
    aws_array_list_clean_up(&elems);
    return aws_raise_error(AWS_CRYPTOSDK_ERR_SERIALIZATION);

WRITE_ERR:
    // We should never get here, because buffer was allocated locally to be long enough.
    aws_array_list_clean_up(&elems);
    aws_byte_buf_clean_up(output);
    return aws_raise_error(AWS_ERROR_SHORT_BUFFER);
}
