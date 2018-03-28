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

#include "cipher.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <assert.h>

#define MSG_ID_LEN 16

struct alg_properties {
    /*
     * We use pointers to the functions that return EVP_MD instead of the
     * returned EVP_MD * itself to allow us to put static structures in the
     * data or rodata segments.
     */
    const EVP_MD *(*md_ctor)(void);
    const EVP_CIPHER *(*cipher_ctor)(void);

    int dk_len, iv_len, tag_len;
};

static const struct alg_properties *alg_props(enum aws_cryptosdk_alg_id alg_id) {
#define STATIC_ALG_PROPS(alg_id, md_ctor_v, cipher_ctor_v, dk_len_v, iv_len_v, tag_len_v) \
    case alg_id: { \
        static const struct alg_properties props = { \
            .md_ctor = (md_ctor_v), \
            .cipher_ctor = (cipher_ctor_v), \
            .dk_len = (dk_len_v)/8, \
            .iv_len = (iv_len_v), \
            .tag_len = (tag_len_v) \
        }; \
        return &props; \
    }
    switch (alg_id) {
        STATIC_ALG_PROPS(AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE,
            NULL, EVP_aes_128_gcm, 128, 12, 16);
        STATIC_ALG_PROPS(AES_128_GCM_IV12_AUTH16_KDSHA256_SIGEC256,
            EVP_sha256, EVP_aes_128_gcm, 128, 12, 16);
        STATIC_ALG_PROPS(AES_128_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
            EVP_sha256, EVP_aes_128_gcm, 128, 12, 16);
        STATIC_ALG_PROPS(AES_192_GCM_IV12_AUTH16_KDNONE_SIGNONE,
            NULL, EVP_aes_192_gcm, 192, 12, 16);
        STATIC_ALG_PROPS(AES_192_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
            EVP_sha256, EVP_aes_192_gcm, 192, 12, 16);
        STATIC_ALG_PROPS(AES_192_GCM_IV12_AUTH16_KDSHA384_SIGEC384,
            EVP_sha384, EVP_aes_192_gcm, 192, 12, 16);
        STATIC_ALG_PROPS(AES_256_GCM_IV12_AUTH16_KDNONE_SIGNONE,
            NULL, EVP_aes_256_gcm, 256, 12, 16);
        STATIC_ALG_PROPS(AES_256_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
            EVP_sha256, EVP_aes_256_gcm, 256, 12, 16);
        STATIC_ALG_PROPS(AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384,
            EVP_sha384, EVP_aes_256_gcm, 256, 12, 16);
        default:
            return NULL;
    }
#undef STATIC_ALG_PROPS
}

static int dk_len(enum aws_cryptosdk_alg_id alg_id) {
    return alg_props(alg_id)->dk_len;
}


int aws_cryptosdk_derive_key(
    struct content_key *content_key,
    const struct data_key *data_key,
    enum aws_cryptosdk_alg_id alg_id,
    const uint8_t *message_id
) {
    const struct alg_properties *props = alg_props(alg_id);

    aws_cryptosdk_secure_zero(content_key->keybuf, sizeof(content_key->keybuf));

    if (props->md_ctor == NULL) {
        memcpy(content_key->keybuf, data_key->keybuf, props->dk_len);
        return AWS_OP_SUCCESS;
    }

    EVP_PKEY_CTX *pctx;
    size_t outlen = dk_len(alg_id);

    uint8_t info[MSG_ID_LEN + 2];
    info[0] = alg_id >> 8;
    info[1] = alg_id & 0xFF;
    memcpy(&info[2], message_id, sizeof(info) - 2);

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    if (pctx == NULL) return aws_raise_error(AWS_CRYPTOSDK_ERR_CRYPTO_UNKNOWN);

    if (EVP_PKEY_derive_init(pctx) <= 0) goto err;
    if (EVP_PKEY_CTX_set_hkdf_md(pctx, props->md_ctor()) <= 0) goto err;
    if (EVP_PKEY_CTX_set1_hkdf_key(pctx, data_key->keybuf, props->dk_len) <= 0) goto err;
    if (EVP_PKEY_CTX_add1_hkdf_info(pctx, info, sizeof(info)) <= 0) goto err;
    if (EVP_PKEY_derive(pctx, content_key->keybuf, &outlen) <= 0) goto err;

    EVP_PKEY_CTX_free(pctx);

    return AWS_OP_SUCCESS;
err:
    EVP_PKEY_CTX_free(pctx);

    aws_cryptosdk_secure_zero(content_key->keybuf, sizeof(content_key->keybuf));
    return aws_raise_error(AWS_CRYPTOSDK_ERR_CRYPTO_UNKNOWN);
}

int aws_cryptosdk_verify_header(
    enum aws_cryptosdk_alg_id alg_id,
    const struct content_key *content_key,
    const struct aws_byte_buf *authtag,
    const struct aws_byte_buf *header
) {
    const struct alg_properties *props = alg_props(alg_id);

    if (authtag->len != props->iv_len + props->tag_len) {
        return aws_raise_error(AWS_CRYPTOSDK_ERR_BAD_CIPHERTEXT);
    }

    const uint8_t *iv = authtag->buffer;
    const uint8_t *tag = authtag->buffer + props->iv_len;
    int result = AWS_CRYPTOSDK_ERR_CRYPTO_UNKNOWN;
    EVP_CIPHER_CTX *ctx = NULL;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto out;
    if (!EVP_DecryptInit_ex(ctx, props->cipher_ctor(), NULL, NULL, NULL)) goto out;
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, props->iv_len, NULL)) goto out;
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, content_key->keybuf, iv)) goto out;

    int outlen;
    if (!EVP_DecryptUpdate(ctx, NULL, &outlen, header->buffer, header->len)) goto out;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, props->tag_len, (void *)tag)) goto out;

    uint8_t finalbuf;
    if (!EVP_DecryptFinal_ex(ctx, &finalbuf, &outlen)) {
        result = AWS_CRYPTOSDK_ERR_BAD_CIPHERTEXT;
        goto out;
    }
    if (outlen != 0) {
        abort(); // wrong output size - potentially smashed stack
    }

    result = AWS_ERROR_SUCCESS;
out:
    if (ctx) EVP_CIPHER_CTX_free(ctx);

    if (result == AWS_ERROR_SUCCESS) {
        return AWS_OP_SUCCESS;
    } else {
        return aws_raise_error(result);
    }
}

static int update_frame_aad(
    EVP_CIPHER_CTX *ctx,
    const uint8_t *message_id,
    int body_frame_type,
    uint32_t seqno,
    uint64_t data_size
) {
    const char *aad_string;

    switch (body_frame_type) {
        case FRAME_TYPE_SINGLE: aad_string = "AWSKMSEncryptionClient Single Block"; break;
        case FRAME_TYPE_FRAME: aad_string = "AWSKMSEncryptionClient Frame"; break;
        case FRAME_TYPE_FINAL: aad_string = "AWSKMSEncryptionClient Final Frame"; break;
        default:
            return aws_raise_error(AWS_ERROR_UNKNOWN);
    }

    int aad_len;

    if (!EVP_DecryptUpdate(ctx, NULL, &aad_len, message_id, MSG_ID_LEN)) return 0;
    if (!EVP_DecryptUpdate(ctx, NULL, &aad_len, (const uint8_t *)aad_string, strlen(aad_string))) return 0;

    seqno = htonl(seqno);
    if (!EVP_DecryptUpdate(ctx, NULL, &aad_len, (const uint8_t *)&seqno, sizeof(seqno))) return 0;

    uint32_t size[2];

    size[0] = htonl(data_size >> 32);
    size[1] = htonl(data_size & 0xFFFFFFFFUL);

    return EVP_DecryptUpdate(ctx, NULL, &aad_len, (const uint8_t *)size, sizeof(size));
}


int aws_cryptosdk_decrypt_body(
    struct aws_byte_buf *out,
    const struct aws_byte_buf *in,
    enum aws_cryptosdk_alg_id alg_id,
    const uint8_t *message_id,
    uint32_t seqno,
    const uint8_t *iv,
    const struct content_key *key,
    const uint8_t *tag,
    int body_frame_type
) {
    const struct alg_properties *props = alg_props(alg_id);

    if (in->len != out->len) {
        return aws_raise_error(AWS_ERROR_SHORT_BUFFER);
    }

    EVP_CIPHER_CTX *ctx = NULL;
    struct aws_byte_buf outp = *out;
    struct aws_byte_buf inp = *in;
    int result = AWS_CRYPTOSDK_ERR_CRYPTO_UNKNOWN;

    if (!(ctx = EVP_CIPHER_CTX_new())) goto out;
    if (!EVP_DecryptInit_ex(ctx, props->cipher_ctor(), NULL, NULL, NULL)) goto out;
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, props->iv_len, NULL)) goto out;
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key->keybuf, iv)) goto out;

    if (!update_frame_aad(ctx, message_id, body_frame_type, seqno, in->len)) goto out;

    while (inp.len) {
        int in_len = inp.len > INT_MAX ? INT_MAX : inp.len;
        int pt_len;

        if (!EVP_DecryptUpdate(ctx, outp.buffer, &pt_len, inp.buffer, in_len)) goto out;
        inp.buffer += in_len;
        inp.len -= in_len;
        outp.buffer += pt_len;
        outp.len -= pt_len;
    }

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, props->tag_len, (void *)tag)) goto out;

    int pt_len_final;
    if (!EVP_DecryptFinal_ex(ctx, outp.buffer, &pt_len_final)) {
        result = AWS_CRYPTOSDK_ERR_BAD_CIPHERTEXT;
        goto out;
    }
    if (pt_len_final != outp.len) {
        abort(); // wrong output size?
    }

    result = AWS_ERROR_SUCCESS;
out:
    if (ctx) EVP_CIPHER_CTX_free(ctx);

    if (result == AWS_ERROR_SUCCESS) {
        return AWS_OP_SUCCESS;
    } else {
        memset(out->buffer, 0, out->len);
        return aws_raise_error(result);
    }
}

int aws_cryptosdk_genrandom(uint8_t *buf, size_t len) {
    int rc = RAND_bytes(buf, len);

    if (rc != 1) {
        aws_cryptosdk_secure_zero(buf, len);
        return aws_raise_error(AWS_CRYPTOSDK_ERR_BAD_CIPHERTEXT);
    }

    return AWS_OP_SUCCESS;
}
