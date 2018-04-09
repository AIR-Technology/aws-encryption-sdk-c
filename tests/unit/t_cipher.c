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

#include <aws/cryptosdk/private/cipher.h>
#include "testing.h"

static int test_kdf() {
    static const struct data_key key[32] = { {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    } };
    static const uint8_t msgid[16] = {
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
    };


#define ASSERT_KDF(alg_id, ...) \
    do {\
        uint8_t expected[MAX_DATA_KEY_SIZE+1] = {__VA_ARGS__, 0}; \
        struct content_key key_out = { { 0 } }; \
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, \
            aws_cryptosdk_derive_key(&key_out, key, alg_id, msgid)); \
        TEST_ASSERT_INT_EQ(0, memcmp(key_out.keybuf, expected, MAX_DATA_KEY_SIZE)); \
    } while (0)

    ASSERT_KDF(AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE,
                     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f );
    ASSERT_KDF(AES_192_GCM_IV12_AUTH16_KDNONE_SIGNONE,
                     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 );
    ASSERT_KDF(AES_256_GCM_IV12_AUTH16_KDNONE_SIGNONE,
                     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f );
    ASSERT_KDF(AES_128_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
                     0xb0, 0xaf, 0xe9, 0xc5, 0x02, 0xb1, 0xf5, 0xe4, 0x52, 0x42, 0xf9, 0xc4, 0x0a, 0xaa, 0x96, 0x66 );
    ASSERT_KDF(AES_192_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
                     0x8d, 0x5c, 0xd4, 0x89, 0x05, 0xb2, 0x78, 0x19, 0x74, 0xc0, 0x0a, 0xa4, 0x10, 0x28, 0xc9, 0x36, 0xfe, 0x5c, 0xe8, 0xc0, 0xb0, 0x47, 0x38, 0x8d );
    ASSERT_KDF(AES_256_GCM_IV12_AUTH16_KDSHA256_SIGNONE,
                     0xca, 0x63, 0x33, 0x7e, 0x0f, 0x1b, 0x51, 0xe6, 0xd8, 0xea, 0x2b, 0xba, 0x47, 0x68, 0x51, 0xaf, 0x81, 0xb9, 0xa1, 0xab, 0x61, 0x10, 0x65, 0x88, 0xa3, 0x68, 0xde, 0xbf, 0xde, 0x28, 0x15, 0x95 );
#if 0
    ASSERT_KDF(AES_128_GCM_IV12_AUTH16_KDSHA256_SIGEC256,
                     0xab, 0x7b, 0xa1, 0x53, 0x57, 0xc9, 0x60, 0x93, 0x12, 0x20, 0x05, 0x47, 0x6a, 0xdd, 0x8d, 0x20 );
    ASSERT_KDF(AES_192_GCM_IV12_AUTH16_KDSHA384_SIGEC384,
                     0x84, 0x28, 0xbd, 0x00, 0xaa, 0x47, 0xa0, 0x8d, 0xee, 0x53, 0x14, 0x58, 0x42, 0x7d, 0xd1, 0xa3, 0x31, 0x36, 0x67, 0xad, 0x0a, 0xeb, 0x06, 0xdf );
    ASSERT_KDF(AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384,
                     0x9d, 0x46, 0xe2, 0x70, 0x9e, 0x59, 0x3e, 0xba, 0xae, 0x81, 0x70, 0x44, 0x16, 0xaf, 0x5d, 0xf9, 0x0c, 0x57, 0x5f, 0xa4, 0xdc, 0xf0, 0xda, 0x78, 0x11, 0x6b, 0x6b, 0x6d, 0x59, 0x9d, 0xe6, 0x2c );
#endif

    return 0;
}

static int test_decrypt_frame_aad() {
    {
        struct content_key key = { {
            0xdd, 0xd0, 0x36, 0x6d, 0xb2, 0x59, 0xa9, 0xef,
            0x22, 0x6b, 0x03, 0x8c, 0x91, 0xe2, 0x05, 0x1f,
            0
        } };
        uint8_t messageId[] = {
            0x22, 0x9b, 0xf1, 0x19, 0x2e, 0xf2, 0x94, 0x32,
            0x28, 0x72, 0x9d, 0xfd, 0x93, 0x98, 0x9b, 0x45
        };
        uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint8_t zero_buf[sizeof(plaintext)] = {0};
        uint32_t seqno = 1;
        uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        uint8_t ciphertext[] = {
            0x6a, 0x76, 0x63, 0x83, 0xbc, 0x7e, 0x6e, 0x2c,
            0x2d, 0x9e, 0x41
        };
        uint8_t tag[] = {
            0xdf, 0x65, 0x40, 0x39, 0xcc, 0x98, 0xa7, 0xa1,
            0xde, 0x91, 0x60, 0x2e, 0x46, 0x49, 0x23, 0xc1
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));

        // Verify that we are checking the tag
        messageId[0]++;
        TEST_ASSERT_INT_EQ(AWS_OP_ERR,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(zero_buf, actual, sizeof(zero_buf)));
        messageId[0]--;

        tag[0]++;
        TEST_ASSERT_INT_EQ(AWS_OP_ERR,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(zero_buf, actual, sizeof(zero_buf)));
        tag[0]--;
    }

    return 0;
}

static int test_decrypt_frame_all_algos() {
    {
        static const uint8_t key_buf[] = {
            0x55, 0xa8, 0xb2, 0xbb, 0x44, 0x27, 0x10, 0x31,
            0xb7, 0x34, 0x3b, 0x80, 0x08, 0xfe, 0xad, 0x52
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x1a, 0x66, 0xd8, 0xe0, 0x0d, 0x30, 0xeb, 0x3d,
            0x14, 0xaf, 0x5b, 0xb6, 0x69, 0xc4, 0x09, 0xbc
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0xeb, 0xe5, 0xf4, 0xb4, 0x7f, 0xee, 0x77, 0x3d,
            0xe9, 0xa3, 0x72
        };
        static const uint8_t tag[] = {
            0x0f, 0xb0, 0x07, 0xd4, 0x37, 0x0d, 0x75, 0x2a,
            0xe2, 0x52, 0xef, 0xff, 0xa0, 0x87, 0x7e, 0x5e
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0x39, 0xd1, 0x1b, 0xa6, 0x29, 0xb3, 0x35, 0x98,
            0x7a, 0x21, 0x8c, 0x04, 0xec, 0x4f, 0x2b, 0xa6,
            0xd4, 0x8e, 0x66, 0x5b, 0x71, 0x8a, 0xf7, 0xe5
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0xbc, 0xd0, 0xec, 0x61, 0x0c, 0x3d, 0xdc, 0x3b,
            0xb8, 0x32, 0x8f, 0xb4, 0xd0, 0x38, 0xf9, 0x3b
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x2e, 0x8f, 0x8c, 0x82, 0x12, 0x7f, 0x4a, 0x1a,
            0xdb, 0xd3, 0x26
        };
        static const uint8_t tag[] = {
            0x71, 0xb6, 0x78, 0x58, 0x38, 0x3f, 0xc6, 0xc9,
            0x31, 0xc0, 0xa8, 0x13, 0xeb, 0x17, 0x79, 0x50
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_192_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0xca, 0x1d, 0xe0, 0x19, 0x55, 0x7b, 0x92, 0x6b,
            0x86, 0x8c, 0x25, 0xeb, 0x28, 0x1c, 0x3f, 0x14,
            0x42, 0x33, 0xfc, 0xbd, 0x1d, 0xce, 0xd6, 0x71,
            0xd0, 0x63, 0x0c, 0xbb, 0x61, 0x56, 0xfd, 0xef
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0xd0, 0x79, 0x31, 0x33, 0x4f, 0xfe, 0xd2, 0xe1,
            0x75, 0x3e, 0x59, 0xcd, 0x33, 0xf2, 0x78, 0x4a
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0xd2, 0x7f, 0xc1, 0x11, 0x1a, 0xc7, 0x09, 0x90,
            0x05, 0xf4, 0x9a
        };
        static const uint8_t tag[] = {
            0x9d, 0x3f, 0x3a, 0xd7, 0x66, 0x3e, 0xb5, 0x71,
            0x0b, 0xa4, 0x89, 0x3e, 0x39, 0x53, 0xd1, 0xfa
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_256_GCM_IV12_AUTH16_KDNONE_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0xac, 0x20, 0xca, 0x57, 0x42, 0x2e, 0x14, 0x64,
            0x5a, 0x03, 0x19, 0x19, 0x7d, 0xa1, 0x19, 0x14
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x42, 0x19, 0xa0, 0x32, 0x0a, 0xc4, 0xbd, 0xe8,
            0x89, 0xe4, 0xcd, 0x55, 0x1d, 0xe5, 0x14, 0x71
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0xaf, 0x5e, 0x27, 0x89, 0x98, 0x3b, 0xea, 0x45,
            0x17, 0x70, 0x7a
        };
        static const uint8_t tag[] = {
            0x10, 0x7f, 0x33, 0x2e, 0x49, 0x73, 0xc1, 0x25,
            0x1d, 0xb4, 0x12, 0x99, 0xf2, 0xf6, 0xb7, 0x8b
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDSHA256_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0xbe, 0xca, 0x0b, 0x92, 0xa3, 0xd9, 0xa6, 0x15,
            0x5b, 0x3d, 0x8d, 0x7f, 0x03, 0x76, 0x66, 0x5b,
            0xba, 0xf5, 0x03, 0x9b, 0xd7, 0x99, 0xa7, 0x7a,
            0xc8, 0x91, 0x33, 0x4f, 0x8a, 0x13, 0xd6, 0xa3
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x61, 0xd1, 0xcc, 0xf2, 0x3e, 0x1c, 0x8f, 0xa3,
            0x6e, 0x56, 0x5f, 0xf7, 0xd0, 0x29, 0x7a, 0xc6
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x82, 0x3f, 0x90, 0xb0, 0x40, 0xe6, 0x04, 0x3d,
            0xb3, 0x7a, 0x7b
        };
        static const uint8_t tag[] = {
            0xc6, 0xff, 0x63, 0xda, 0x4d, 0xd6, 0x00, 0xee,
            0xe1, 0xdf, 0xd8, 0xb2, 0xcf, 0x44, 0xae, 0xc3
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_256_GCM_IV12_AUTH16_KDSHA256_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0x3e, 0x77, 0x37, 0x02, 0x14, 0x76, 0x44, 0x7e,
            0xb6, 0xe1, 0x42, 0x8a, 0xea, 0xc5, 0xf4, 0xe7,
            0xd0, 0xe8, 0xb8, 0xd9, 0x48, 0x53, 0x8c, 0xbe
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0xdf, 0xd3, 0x03, 0x84, 0xc4, 0x55, 0x8a, 0x17,
            0xb5, 0x29, 0xd8, 0x2e, 0x75, 0x3e, 0x2e, 0x81
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x0c, 0x95, 0x2c, 0x37, 0xf5, 0xe3, 0x9e, 0xcf,
            0x0c, 0x56, 0x4c
        };
        static const uint8_t tag[] = {
            0x5b, 0xf9, 0x29, 0x2d, 0xae, 0x2e, 0x12, 0x1a,
            0x86, 0xcd, 0xdd, 0x9e, 0x58, 0xc2, 0xd3, 0x00
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_192_GCM_IV12_AUTH16_KDSHA256_SIGNONE, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
#if 0
    {
        static const uint8_t key_buf[] = {
            0x7f, 0x2b, 0x62, 0x45, 0x36, 0x2c, 0x5c, 0x46,
            0x5c, 0x5e, 0x0f, 0xb3, 0x36, 0xeb, 0x27, 0xf1,
            0x70, 0x11, 0xf8, 0xa7, 0x9f, 0xe2, 0xa7, 0xaf
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x15, 0x3f, 0xb2, 0x29, 0xaa, 0xc8, 0x33, 0x8a,
            0x96, 0x92, 0x90, 0x8e, 0x13, 0x02, 0x2b, 0x91
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x16, 0xc0, 0x32, 0x5d, 0x28, 0xea, 0x49, 0xb4,
            0x4e, 0x8e, 0x1d
        };
        static const uint8_t tag[] = {
            0x9e, 0xcf, 0xc8, 0xf6, 0xe5, 0xea, 0xb9, 0x74,
            0x43, 0x7a, 0x6d, 0x11, 0xb6, 0xe7, 0x38, 0x56
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_192_GCM_IV12_AUTH16_KDSHA384_SIGEC384, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0x2e, 0x7a, 0x9a, 0x94, 0x88, 0xaa, 0x53, 0xc6,
            0x82, 0x74, 0xcc, 0x7e, 0xe3, 0x7e, 0x46, 0xf1
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x36, 0xbb, 0xf9, 0x10, 0xff, 0xe7, 0xf2, 0xf7,
            0x6c, 0x41, 0x26, 0x2d, 0xa7, 0x44, 0xa3, 0xd2
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x21, 0xf6, 0xd1, 0x07, 0x8e, 0x4d, 0x2f, 0x95,
            0x2e, 0x9b, 0x9e
        };
        static const uint8_t tag[] = {
            0xe1, 0xa0, 0x0e, 0xde, 0x53, 0x7b, 0xea, 0xf4,
            0xfc, 0x90, 0xc9, 0x77, 0x11, 0x74, 0xf9, 0x27
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_128_GCM_IV12_AUTH16_KDSHA256_SIGEC256, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
    {
        static const uint8_t key_buf[] = {
            0xd7, 0xad, 0x1e, 0x05, 0x32, 0x2e, 0x46, 0x62,
            0x0e, 0xcc, 0xfc, 0x7a, 0xd6, 0xc8, 0x33, 0xf0,
            0x77, 0x16, 0xe4, 0x64, 0x1f, 0xa8, 0xb0, 0xe7,
            0x34, 0x1b, 0x14, 0x09, 0x67, 0x12, 0xd7, 0xb6
        };
        struct content_key key = { { 0 } };
        memcpy(key.keybuf, key_buf, sizeof(key_buf));
        static const uint8_t messageId[] = {
            0x80, 0xbc, 0x85, 0x15, 0x81, 0x79, 0x67, 0x8f,
            0x5a, 0x33, 0x5d, 0x04, 0xda, 0x0d, 0x83, 0x32
        };
        static const uint8_t plaintext[] = {
            0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f,
            0x72, 0x6c, 0x64
        };
        uint32_t seqno = 1;
        static const uint8_t iv[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01
        };
        static const uint8_t ciphertext[] = {
            0x3a, 0xed, 0xa7, 0x27, 0x9f, 0xc0, 0xaa, 0xe2,
            0x22, 0x34, 0x97
        };
        static const uint8_t tag[] = {
            0x76, 0x60, 0x23, 0xbf, 0xf3, 0x7e, 0x0b, 0x80,
            0x8d, 0xe1, 0xe9, 0x59, 0x32, 0x26, 0x79, 0xe7
        };
        uint8_t actual[11] = {0};
        struct aws_byte_buf in = { .buffer = (void*)ciphertext, .len = sizeof(ciphertext) };
        struct aws_byte_buf out = { .buffer = (void *)actual, .len = sizeof(actual) };
        TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
                aws_cryptosdk_decrypt_body(
                    &out, &in, AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384, messageId, seqno, iv, &key, tag, FRAME_TYPE_FRAME));
        TEST_ASSERT_INT_EQ(0, memcmp(plaintext, actual, sizeof(plaintext)));
    }
#endif
    return 0;
}

static int testHeaderAuth(const uint8_t *header, size_t headerlen, const uint8_t *authtag, size_t taglen, const uint8_t *key, enum aws_cryptosdk_alg_id alg) {
    struct content_key derived_key;
    struct data_key data_key;

    // We assume our test vector keys are appropriately sized
    memcpy(data_key.keybuf, key, sizeof(data_key.keybuf));

    // XXX: Properly parse header instead of blindly getting the message ID from it
    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS,
        aws_cryptosdk_derive_key(&derived_key, &data_key, alg, header + 4));

    struct aws_byte_buf headerbuf = { .buffer = (uint8_t *)header, .len = headerlen };
    struct aws_byte_buf authbuf = { .buffer = (uint8_t *)authtag, .len = taglen };

    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, aws_cryptosdk_verify_header(alg, &derived_key, &authbuf, &headerbuf));

    uint8_t badheader[headerlen];
    headerbuf.buffer = badheader;

    for (int i = 0; i < headerlen * 8; i++) {
        memcpy(badheader, header, headerlen);
        badheader[i/8] ^= (1 << (i % 8));

        TEST_ASSERT_INT_EQ(AWS_OP_ERR, aws_cryptosdk_verify_header(alg, &derived_key, &authbuf, &headerbuf));
    }

    uint8_t badtag[taglen];
    headerbuf.buffer = (uint8_t *)header;
    authbuf.buffer = badtag;

    for (int i = 0; i < taglen * 8; i++) {
        memcpy(badtag, authtag, taglen);
        badtag[i/8] ^= (1 << (i % 8));

        TEST_ASSERT_INT_EQ(AWS_OP_ERR, aws_cryptosdk_verify_header(alg, &derived_key, &authbuf, &headerbuf));
    }

    return 0;
}

static int test_verify_header() {
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x00, 0x14, 0xfb, 0xb2, 0xad, 0xb6,
            0xc9, 0x67, 0xe1, 0x8f, 0xe2, 0x24, 0x9b, 0x07,
            0xda, 0xf0, 0x72, 0x76, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x3f, 0x45, 0x60,
            0x69, 0xf5, 0x3c, 0xdc, 0x73, 0x32, 0x2b, 0x1e,
            0x27, 0x6c, 0x39, 0x25
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0x62, 0x96, 0xd9, 0x52, 0x67, 0x10, 0xfd, 0xc7,
            0xa1, 0xb7, 0xa5, 0xcd, 0xe4, 0xe0, 0x76, 0x4c
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_128_GCM_IV12_AUTH16_KDNONE_SIGNONE)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x00, 0x46, 0x06, 0x10, 0xdc, 0x69,
            0x68, 0x0a, 0x52, 0x01, 0xdf, 0x15, 0x4d, 0x41,
            0xc6, 0x5a, 0x18, 0x89, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xeb, 0x18, 0xa3, 0x61,
            0xab, 0x25, 0x07, 0xfb, 0x2c, 0x12, 0x8b, 0x5f,
            0x58, 0x8f, 0x06, 0xed
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0x3c, 0x5c, 0xae, 0xc5, 0x38, 0xcf, 0x0a, 0x06,
            0x13, 0x01, 0x11, 0x0e, 0x4d, 0x66, 0xda, 0xff,
            0xf0, 0x2a, 0xbd, 0x55, 0x2c, 0xbc, 0xa9, 0xa5
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_192_GCM_IV12_AUTH16_KDNONE_SIGNONE)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x00, 0x78, 0x55, 0x36, 0xbe, 0xea,
            0xe3, 0x64, 0x37, 0xa9, 0xb1, 0xbe, 0x2e, 0x62,
            0x1b, 0x08, 0x1e, 0x3c, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xaa, 0x7f, 0x16, 0xc9,
            0x79, 0x88, 0xcf, 0x34, 0x9f, 0x6d, 0xa2, 0x41,
            0x73, 0xd2, 0x8e, 0x66
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0xde, 0x57, 0x2a, 0xb1, 0xb7, 0xdf, 0x4f, 0x97,
            0x27, 0xbc, 0x87, 0x7b, 0xcf, 0x80, 0x94, 0xe5,
            0x9f, 0x14, 0x54, 0x8d, 0xd3, 0x4b, 0x67, 0xc2,
            0x5e, 0x0b, 0xcb, 0xad, 0xa1, 0x30, 0xa2, 0xe8
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_256_GCM_IV12_AUTH16_KDNONE_SIGNONE)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x01, 0x14, 0x17, 0xb3, 0xa9, 0x64,
            0x34, 0x89, 0x7d, 0x60, 0xcd, 0x7f, 0xf2, 0x85,
            0x41, 0x4d, 0xc0, 0x3d, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xb5, 0x4e, 0x19, 0x82,
            0x2c, 0x09, 0xc9, 0x82, 0x09, 0xc7, 0x63, 0x0c,
            0x7f, 0x4c, 0xc6, 0xf7
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0xeb, 0x70, 0x9d, 0xf0, 0x34, 0x8a, 0x04, 0x09,
            0x14, 0x33, 0x5d, 0x9e, 0x48, 0x75, 0xec, 0xaa
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_128_GCM_IV12_AUTH16_KDSHA256_SIGNONE)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x01, 0x46, 0x82, 0x3b, 0x71, 0x2d,
            0x5b, 0x84, 0xd3, 0x8a, 0xda, 0x97, 0xdd, 0x97,
            0x33, 0x99, 0x0f, 0x7a, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xe0, 0x4d, 0x90, 0x00,
            0x33, 0x87, 0x88, 0xa9, 0x5c, 0x22, 0x56, 0xc3,
            0xcf, 0xa3, 0xc1, 0x87
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0xe2, 0xa2, 0x8f, 0x25, 0x30, 0x0c, 0xe8, 0x16,
            0xdb, 0x42, 0xdb, 0xa9, 0xbc, 0xdc, 0xac, 0x63,
            0xf3, 0x31, 0x7b, 0xb7, 0xd9, 0xce, 0xc7, 0xf8
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_192_GCM_IV12_AUTH16_KDSHA256_SIGNONE)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x01, 0x78, 0xd4, 0xe9, 0xf9, 0xde,
            0xc5, 0xca, 0x9a, 0x36, 0x27, 0x03, 0x9d, 0xd3,
            0x1d, 0xfd, 0xbd, 0x29, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10,
            0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x98, 0xa1, 0xb5, 0x29,
            0xcf, 0x56, 0x8f, 0xdf, 0x67, 0x08, 0x49, 0xd7,
            0xa7, 0xe3, 0xa6, 0xcc
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0x4e, 0x8d, 0x8f, 0xc6, 0xbe, 0x87, 0x39, 0x6f,
            0x73, 0x57, 0x61, 0xda, 0x35, 0x93, 0x30, 0xbf,
            0x0f, 0xa9, 0x5e, 0xea, 0x97, 0xa7, 0x19, 0xf0,
            0x42, 0xef, 0x50, 0x95, 0x58, 0x95, 0xd6, 0x5d
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_256_GCM_IV12_AUTH16_KDSHA256_SIGNONE)) return 1;
    }
#if 0
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x02, 0x14, 0xb6, 0xaf, 0xf6, 0xd4,
            0x6f, 0x51, 0x0d, 0xd4, 0x8e, 0xd1, 0xdc, 0x5b,
            0x2f, 0x1d, 0x0f, 0x7e, 0x00, 0x47, 0x00, 0x01,
            0x00, 0x15, 0x61, 0x77, 0x73, 0x2d, 0x63, 0x72,
            0x79, 0x70, 0x74, 0x6f, 0x2d, 0x70, 0x75, 0x62,
            0x6c, 0x69, 0x63, 0x2d, 0x6b, 0x65, 0x79, 0x00,
            0x2c, 0x41, 0x6a, 0x35, 0x5a, 0x31, 0x47, 0x6d,
            0x5a, 0x72, 0x6f, 0x56, 0x37, 0x52, 0x4f, 0x34,
            0x6a, 0x7a, 0x45, 0x6b, 0x57, 0x50, 0x39, 0x4a,
            0x7a, 0x4e, 0x37, 0x4c, 0x76, 0x64, 0x58, 0x56,
            0x55, 0x63, 0x35, 0x54, 0x55, 0x6b, 0x35, 0x71,
            0x4c, 0x76, 0x62, 0x74, 0x4a, 0x00, 0x01, 0x00,
            0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xfe, 0x7c, 0x51, 0xf9,
            0x01, 0xe4, 0x91, 0x68, 0xb3, 0x6e, 0xd6, 0xde,
            0x3c, 0x01, 0x32, 0x01
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0x51, 0x31, 0x83, 0xbe, 0xf7, 0xb7, 0x21, 0xaa,
            0x40, 0x01, 0x79, 0xb6, 0x28, 0x9d, 0x6b, 0x49
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_128_GCM_IV12_AUTH16_KDSHA256_SIGEC256)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x03, 0x46, 0x1a, 0xd8, 0x19, 0xb3,
            0x23, 0xd7, 0xca, 0x21, 0xcd, 0xbc, 0xb9, 0xcb,
            0x58, 0x5d, 0x23, 0xd8, 0x00, 0x5f, 0x00, 0x01,
            0x00, 0x15, 0x61, 0x77, 0x73, 0x2d, 0x63, 0x72,
            0x79, 0x70, 0x74, 0x6f, 0x2d, 0x70, 0x75, 0x62,
            0x6c, 0x69, 0x63, 0x2d, 0x6b, 0x65, 0x79, 0x00,
            0x44, 0x41, 0x39, 0x76, 0x61, 0x75, 0x45, 0x43,
            0x67, 0x47, 0x4f, 0x2b, 0x61, 0x72, 0x4b, 0x57,
            0x59, 0x67, 0x52, 0x44, 0x46, 0x6c, 0x73, 0x62,
            0x6c, 0x39, 0x49, 0x4f, 0x30, 0x35, 0x67, 0x6f,
            0x34, 0x4a, 0x45, 0x51, 0x75, 0x67, 0x62, 0x5a,
            0x79, 0x4b, 0x69, 0x38, 0x73, 0x79, 0x69, 0x33,
            0x65, 0x36, 0x46, 0x37, 0x42, 0x61, 0x67, 0x38,
            0x73, 0x47, 0x2f, 0x74, 0x42, 0x56, 0x46, 0x32,
            0x50, 0x41, 0x51, 0x3d, 0x3d, 0x00, 0x01, 0x00,
            0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x09, 0xe1, 0x8f, 0x93,
            0xa5, 0x68, 0xc9, 0x94, 0x1c, 0x33, 0x43, 0x28,
            0x90, 0x52, 0x30, 0x09
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0x10, 0xa3, 0x3d, 0xbb, 0x6b, 0xa7, 0xfa, 0xb8,
            0xd1, 0x7c, 0xee, 0xd1, 0x1c, 0xf5, 0xc7, 0xa9,
            0x6d, 0x02, 0x43, 0xb2, 0x64, 0x84, 0xc0, 0x62
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_192_GCM_IV12_AUTH16_KDSHA384_SIGEC384)) return 1;
    }
    {
        static const uint8_t header[] = {
            0x01, 0x80, 0x03, 0x78, 0x84, 0xae, 0x67, 0x58,
            0x7a, 0x6e, 0xe7, 0xdc, 0xb7, 0xd3, 0x39, 0xe8,
            0x4d, 0x0b, 0xc2, 0xef, 0x00, 0x5f, 0x00, 0x01,
            0x00, 0x15, 0x61, 0x77, 0x73, 0x2d, 0x63, 0x72,
            0x79, 0x70, 0x74, 0x6f, 0x2d, 0x70, 0x75, 0x62,
            0x6c, 0x69, 0x63, 0x2d, 0x6b, 0x65, 0x79, 0x00,
            0x44, 0x41, 0x35, 0x34, 0x74, 0x6e, 0x55, 0x76,
            0x39, 0x4c, 0x42, 0x37, 0x4a, 0x44, 0x54, 0x76,
            0x47, 0x36, 0x2f, 0x35, 0x4c, 0x5a, 0x36, 0x70,
            0x65, 0x53, 0x6a, 0x46, 0x77, 0x77, 0x79, 0x62,
            0x68, 0x45, 0x6f, 0x4b, 0x6f, 0x48, 0x30, 0x7a,
            0x61, 0x79, 0x48, 0x57, 0x64, 0x64, 0x77, 0x6b,
            0x36, 0x34, 0x62, 0x31, 0x76, 0x6a, 0x67, 0x5a,
            0x71, 0x4b, 0x38, 0x71, 0x59, 0x54, 0x79, 0x41,
            0x44, 0x50, 0x41, 0x3d, 0x3d, 0x00, 0x01, 0x00,
            0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x10, 0x00
        };
        static const uint8_t authtag[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xb5, 0xc1, 0x2a, 0x88,
            0xb2, 0xc3, 0xf7, 0x54, 0x8f, 0xb6, 0xa9, 0x35,
            0x67, 0xd3, 0xd9, 0xde
        };
        static const uint8_t key[MAX_DATA_KEY_SIZE] = {
            0xe3, 0xce, 0xce, 0x1b, 0x53, 0xe2, 0x4f, 0x3b,
            0xcc, 0xa3, 0xc4, 0xb0, 0x1d, 0x77, 0x2d, 0x52,
            0x51, 0x9a, 0x9f, 0x36, 0xa3, 0x58, 0x0b, 0x27,
            0x3d, 0x10, 0xdd, 0xc8, 0xa4, 0xda, 0x2f, 0x63
        };
        if (testHeaderAuth(header, sizeof(header), authtag, sizeof(authtag), key, AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384)) return 1;
    }
#endif
    return 0;
}

static int test_random() {
    // There's not too much we can test for a RNG, but at least do a simple sanity test...
    uint8_t buf1[16] = {0};
    uint8_t buf2[sizeof(buf1)] = {0};

    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, aws_cryptosdk_genrandom(buf1, sizeof(buf1)));
    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, aws_cryptosdk_genrandom(buf2, sizeof(buf2)));

    TEST_ASSERT_INT_EQ(0, !memcmp(buf1, buf2, sizeof(buf1)));

    return 0;
}

struct test_case cipher_test_cases[] = {
    { "cipher", "test_kdf", test_kdf },
    { "cipher", "test_decrypt_frame_aad", test_decrypt_frame_aad },
    { "cipher", "test_decrypt_frame_all_algos", test_decrypt_frame_all_algos },
    { "cipher", "test_verify_header", test_verify_header },
    { "cipher", "test_random", test_random },
    { NULL }
};

