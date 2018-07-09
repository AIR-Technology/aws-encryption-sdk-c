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
#include <aws/cryptosdk/private/raw_aes_mk.h>
#include <aws/cryptosdk/private/enc_context.h>
#include <aws/cryptosdk/private/utils.h>
#include <openssl/evp.h>

int serialize_provider_info_init(struct aws_allocator * alloc,
                                 struct aws_byte_buf * output,
                                 const struct aws_string * master_key_id,
                                 const uint8_t iv[RAW_AES_MK_IV_LEN]) {
    size_t serialized_len = master_key_id->len + RAW_AES_MK_IV_LEN + 8; // 4 for tag len, 4 for iv len
    if (aws_byte_buf_init(alloc, output, serialized_len)) {
        return AWS_OP_ERR;
    }
    output->len = output->capacity;
    struct aws_byte_cursor cur = aws_byte_cursor_from_buf(output);
    if (!aws_byte_cursor_write_from_whole_string(&cur, master_key_id)) goto WRITE_ERR;
    if (!aws_byte_cursor_write_be32(&cur, RAW_AES_MK_TAG_LEN)) goto WRITE_ERR;
    if (!aws_byte_cursor_write_be32(&cur, RAW_AES_MK_IV_LEN)) goto WRITE_ERR;
    if (!aws_byte_cursor_write(&cur, iv, RAW_AES_MK_IV_LEN)) goto WRITE_ERR;

    return AWS_OP_SUCCESS;

WRITE_ERR:
    // We should never get here, because buffer was allocated locally to be long enough.
    aws_byte_buf_clean_up(output);
    return aws_raise_error(AWS_ERROR_SHORT_BUFFER);
}

bool parse_provider_info(struct aws_cryptosdk_mk * mk,
                         struct aws_byte_buf * iv,
                         const struct aws_byte_buf * provider_info) {
    struct raw_aes_mk * self = (struct raw_aes_mk *)mk;
    size_t mkid_len = self->master_key_id->len;
    size_t serialized_len = mkid_len + RAW_AES_MK_IV_LEN + 8;
    if (serialized_len != provider_info->len) return false;

    struct aws_byte_cursor cur = aws_byte_cursor_from_buf(provider_info);

    struct aws_byte_cursor mkid = aws_byte_cursor_advance_nospec(&cur, mkid_len);
    if (!mkid.ptr) goto READ_ERR;
    if (!aws_string_eq_byte_cursor(self->master_key_id, &mkid)) return false;

    uint32_t tag_len, iv_len;
    if (!aws_byte_cursor_read_be32(&cur, &tag_len)) goto READ_ERR;
    if (tag_len != RAW_AES_MK_TAG_LEN) return false;

    if (!aws_byte_cursor_read_be32(&cur, &iv_len)) goto READ_ERR;
    if (iv_len != RAW_AES_MK_IV_LEN) return false;

    *iv = aws_byte_buf_from_array(cur.ptr, cur.len);
    return true;

READ_ERR:
    // We should never get here because we verified cursor was exactly the right length
    aws_raise_error(AWS_ERROR_SHORT_BUFFER);
    return false;
}

static int raw_aes_mk_generate_data_key(struct aws_cryptosdk_mk * mk,
                                        struct aws_cryptosdk_encryption_materials * enc_mat) {
    // TODO: implement
    return AWS_OP_ERR;
}

static int raw_aes_mk_encrypt_data_key(struct aws_cryptosdk_mk * mk,
                                       struct aws_cryptosdk_encryption_materials * enc_mat) {
    // TODO: implement
    return AWS_OP_ERR;
}

static int raw_aes_mk_decrypt_data_key(struct aws_cryptosdk_mk * mk,
                                       struct aws_cryptosdk_decryption_materials * dec_mat,
                                       const struct aws_cryptosdk_decryption_request * request) {
#if 0
    struct raw_aes_mk * self = (struct raw_aes_mk *)mk;

    struct aws_byte_buf aad;
    if (aws_cryptosdk_serialize_enc_context_init(request->alloc, &aad, request->enc_context)) {
        return AWS_OP_ERR;
    }

    const struct aws_array_list * edks = request->encrypted_data_keys;
    size_t num_edks = aws_array_list_length(edks);
    for (size_t edk_idx = 0; edk_idx < num_edks; ++edk_idx) {
        const struct aws_cryptosdk_edk * edk;
        if (aws_array_list_get_at_ptr(edks, (void **)&edk, edk_idx)) {
            aws_byte_buf_clean_up(&aad);
            return AWS_OP_ERR;
        }
        const struct aws_byte_buf * edk_bytes = &edk->enc_data_key; // cipher to decrypt
        const struct aws_byte_buf * provider_info = &edk->provider_info; // deserialize to get master key ID, tag len, IV len & IV
        const struct aws_byte_buf * provider_id = &edk->provider_id; // verify it is same provider ID as given to MK on creation
    }

    EVP_CIPHER_CTC * ctx;
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        // TODO: get error info from openssl?
        return AWS_OP_ERR;
    }
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, self->raw_key_bytes, NULL)) {
        goto OPENSSL_ERR;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    return AWS_OP_SUCCESS;

OPENSSL_ERR:
    // TODO: get error info from openssl?
    EVP_CIPHER_CTX_free(ctx);
    aws_byte_buf_clean_up(&aad);

#endif // #if 0
    return AWS_OP_ERR;
}

static void raw_aes_mk_destroy(struct aws_cryptosdk_mk * mk) {
    struct raw_aes_mk * self = (struct raw_aes_mk *)mk;
    aws_string_destroy((void *)self->master_key_id);
    aws_string_destroy((void *)self->provider_id);
    if (self && self->alloc) aws_mem_release(self->alloc, self);
}

static const struct aws_cryptosdk_mk_vt raw_aes_mk_vt = {
    .vt_size = sizeof(struct aws_cryptosdk_mk_vt),
    .name = "raw AES mk",
    .destroy = raw_aes_mk_destroy,
    .generate_data_key = raw_aes_mk_generate_data_key,
    .encrypt_data_key = raw_aes_mk_encrypt_data_key,
    .decrypt_data_key = raw_aes_mk_decrypt_data_key
};

struct aws_cryptosdk_mk * aws_cryptosdk_raw_aes_mk_new(struct aws_allocator * alloc,
                                                       const uint8_t * master_key_id,
                                                       size_t master_key_id_len,
                                                       const uint8_t * provider_id,
                                                       size_t provider_id_len,
                                                       const uint8_t raw_key_bytes[32]) {
    struct raw_aes_mk * mk = aws_mem_acquire(alloc, sizeof(struct raw_aes_mk));
    if (!mk) return NULL;

    mk->master_key_id = aws_string_from_array_new(alloc, master_key_id, master_key_id_len);
    if (!mk->master_key_id) {
        aws_mem_release(alloc, mk);
        return NULL;
    }
    mk->provider_id = aws_string_from_array_new(alloc, provider_id, provider_id_len);
    if (!mk->provider_id) {
        aws_string_destroy((void *)mk->master_key_id);
        aws_mem_release(alloc, mk);
        return NULL;
    }

    mk->vt = &raw_aes_mk_vt;
    mk->alloc = alloc;
    mk->raw_key_bytes = raw_key_bytes;
    return (struct aws_cryptosdk_mk *)mk;
}
