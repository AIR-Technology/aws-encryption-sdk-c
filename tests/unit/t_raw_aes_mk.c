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
#include "testing.h"

/**
 * Provider info serialization/deserialization tests.
 */

AWS_STATIC_STRING_FROM_LITERAL(ser_master_key_id, "Master key id");
static const uint8_t iv[RAW_AES_MK_IV_LEN] =
{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb};

static const uint8_t serialized_provider_info[] = {
    'M', 'a', 's', 't', 'e', 'r', ' ', 'k', 'e', 'y', ' ', 'i', 'd',
    0x00, 0x00, 0x00, RAW_AES_MK_TAG_LEN << 3,
    0x00, 0x00, 0x00, RAW_AES_MK_IV_LEN,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb};

AWS_STATIC_STRING_FROM_LITERAL(ser_provider_id, "Provider id");

static const uint8_t raw_key_bytes[32];

int serialize_valid_provider_info() {

    struct aws_allocator * alloc = aws_default_allocator();
    struct aws_byte_buf provider_info;

    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, serialize_provider_info_init(alloc,
                                                                    &provider_info,
                                                                    ser_master_key_id,
                                                                    iv));

    TEST_ASSERT_BUF_EQ(provider_info,
                       'M', 'a', 's', 't', 'e', 'r', ' ', 'k', 'e', 'y', ' ', 'i', 'd',
                       0x00, 0x00, 0x00, RAW_AES_MK_TAG_LEN << 3,
                       0x00, 0x00, 0x00, RAW_AES_MK_IV_LEN,
                       0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb);

    aws_byte_buf_clean_up(&provider_info);
    return 0;
}

int parse_valid_provider_info() {
    struct aws_cryptosdk_mk * mk = aws_cryptosdk_raw_aes_mk_new(aws_default_allocator(),
                                                                aws_string_bytes(ser_master_key_id),
                                                                ser_master_key_id->len,
                                                                aws_string_bytes(ser_provider_id),
                                                                ser_provider_id->len,
                                                                raw_key_bytes);
    TEST_ASSERT_ADDR_NOT_NULL(mk);

    struct aws_byte_buf iv_output;
    struct aws_byte_buf ser_prov_info = aws_byte_buf_from_array(serialized_provider_info,
                                                                sizeof(serialized_provider_info));
    TEST_ASSERT(parse_provider_info(mk, &iv_output, &ser_prov_info));

    TEST_ASSERT_BUF_EQ(iv_output, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb);

    aws_cryptosdk_mk_destroy(mk);
    return 0;
}

/**
 * Raw AES MK decrypt data key tests
 */

const uint8_t test_vector_master_key_id[] = "asdfhasiufhiasuhviawurhgiuawrhefiuawhf";
const uint8_t test_vector_provider_id[] = "static-random";
const uint8_t test_vector_wrapping_key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};

int decrypt_data_key_empty_encryption_context() {
  struct aws_allocator * alloc = aws_default_allocator();

  struct aws_cryptosdk_mk * mk = aws_cryptosdk_raw_aes_mk_new(alloc,
							      test_vector_master_key_id,
							      sizeof(test_vector_master_key_id) - 1,
							      test_vector_provider_id,
							      sizeof(test_vector_provider_id) - 1,
							      test_vector_wrapping_key);
  TEST_ASSERT_ADDR_NOT_NULL(mk);

  struct aws_hash_table enc_context;
  TEST_ASSERT_INT_EQ(aws_hash_table_init(&enc_context, alloc, 1, aws_hash_string, aws_string_eq, aws_string_destroy, aws_string_destroy),
		     AWS_OP_SUCCESS);

  struct aws_cryptosdk_decryption_request req;
  req.alloc = alloc;
  req.alg = AES_256_GCM_IV12_AUTH16_KDSHA256_SIGNONE;
  req.enc_context = &enc_context;

  aws_array_list_init_dynamic(&req.encrypted_data_keys, alloc, 1, sizeof(struct aws_cryptosdk_edk));

  // first 32 bytes: encrypted data key, last 16 bytes GCM tag
  const uint8_t edk_bytes[] = {0x54, 0x2b, 0xf0, 0xdc, 0x35, 0x20, 0x07, 0x38, 0xe4, 0x9e, 0x34, 0xfa, 0xa6, 0xbf, 0x11, 0xed,
			       0x45, 0x40, 0x97, 0xfd, 0xb8, 0xe3, 0x36, 0x75, 0x5c, 0x03, 0xbb, 0x9f, 0xa4, 0x42, 0x9e, 0x66,
			       0x44, 0x7c, 0x39, 0xf7, 0x7f, 0xfe, 0xbc, 0xa5, 0x98, 0x70, 0xe9, 0xa8, 0xc9, 0xb5, 0x7f, 0x6f};

    const uint8_t edk_provider_info[] =
      "asdfhasiufhiasuhviawurhgiuawrhefiuawhf" // master key ID
      "\x00\x00\x00\x80" // GCM tag length in bits
      "\x00\x00\x00\x0c" // IV length in bytes
      "\xbe\xa0\xfb\xd0\x0e\xee\x0d\x94\xd9\xb1\xb3\x93"; // IV

    struct aws_cryptosdk_edk edk;
    edk.enc_data_key = aws_byte_buf_from_array(edk_bytes, sizeof(edk_bytes));
    edk.provider_id = aws_byte_buf_from_array(test_vector_provider_id, sizeof(test_vector_provider_id) - 1);
    edk.provider_info = aws_byte_buf_from_array(edk_provider_info, sizeof(edk_provider_info) - 1);

    aws_array_list_push_back(&req.encrypted_data_keys, (void *) &edk);

    struct aws_cryptosdk_decryption_materials * dec_mat = aws_cryptosdk_decryption_materials_new(alloc, req.alg);
    TEST_ASSERT_ADDR_NOT_NULL(dec_mat);

    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, aws_cryptosdk_mk_decrypt_data_key(mk, dec_mat, &req));
    TEST_ASSERT_ADDR_NOT_NULL(dec_mat->unencrypted_data_key.buffer);
    TEST_ASSERT_BUF_EQ(dec_mat->unencrypted_data_key,
		       0xdd, 0xc2, 0xf6, 0x5f, 0x96, 0xa2, 0xda, 0x96, 0x86, 0xea, 0xd6, 0x58, 0xfe, 0xe9, 0xc0, 0xc3,
		       0xb6, 0xd4, 0xb1, 0x92, 0xf2, 0xba, 0x50, 0x93, 0x21, 0x97, 0x62, 0xab, 0x7d, 0x25, 0x9f, 0x2c);

    aws_cryptosdk_decryption_materials_destroy(dec_mat);
    aws_array_list_clean_up(&req.encrypted_data_keys);
    aws_hash_table_clean_up(&enc_context);
    aws_cryptosdk_mk_destroy(mk);
    return 0;
}

int decrypt_data_key_with_signature() {
    struct aws_allocator * alloc = aws_default_allocator();

    struct aws_cryptosdk_mk * mk = aws_cryptosdk_raw_aes_mk_new(alloc,
                                                                test_vector_master_key_id,
                                                                sizeof(test_vector_master_key_id) - 1,
                                                                test_vector_provider_id,
                                                                sizeof(test_vector_provider_id) - 1,
                                                                test_vector_wrapping_key);
    TEST_ASSERT_ADDR_NOT_NULL(mk);

    AWS_STATIC_STRING_FROM_LITERAL(enc_context_key, "aws-crypto-public-key");
    AWS_STATIC_STRING_FROM_LITERAL(enc_context_val, "AguATtjJFzJnlpNXdyDG7e8bfLZYRx+vxdAmYz+ztVBYyFhsMpchjz9ev3MdXQMD9Q==");
    struct aws_hash_table enc_context;
    TEST_ASSERT_INT_EQ(aws_hash_table_init(&enc_context, alloc, 1, aws_hash_string, aws_string_eq, aws_string_destroy, aws_string_destroy),
                       AWS_OP_SUCCESS);
    struct aws_hash_element * elem;
    TEST_ASSERT_INT_EQ(aws_hash_table_create(&enc_context, (void *)enc_context_key, &elem, NULL), AWS_OP_SUCCESS);
    elem->value = (void *)enc_context_val;

    struct aws_cryptosdk_decryption_request req;
    req.alloc = alloc;
    // FIXME: change to correct algorithm after it is implemented.
    // This test data was made in mode AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384, so it includes the public key in the
    // encryption context. The only thing this test is using the algorithm for is getting the length of the data key,
    // so as long as we specify another AES-256 suite the test passes, but it communicates the wrong thing to the reader.
    // We cannot specify the correct algorithm at the moment, because it is currently commented out of alg_props implementation.
    // Once we are done implementing the algorithm with signature in alg_props, remove this comment and switch req.alg to
    // correct value.
    //req.alg = AES_256_GCM_IV12_AUTH16_KDSHA384_SIGEC384; // correct algorithm suite
    req.alg = AES_256_GCM_IV12_AUTH16_KDNONE_SIGNONE; // wrong algorithm suite
    req.enc_context = &enc_context;
    aws_array_list_init_dynamic(&req.encrypted_data_keys, alloc, 1, sizeof(struct aws_cryptosdk_edk));

    // first 32 bytes: encrypted data key, last 16 bytes GCM tag
    const uint8_t edk_bytes[] = {0xff, 0xaf, 0xb4, 0x82, 0xf0, 0x0f, 0x9b, 0x4e, 0x5e, 0x0e, 0x75, 0xea, 0x67, 0xbb, 0x80, 0xc6,
                                 0x5a, 0x37, 0x18, 0x35, 0x55, 0x62, 0xfb, 0x9c, 0x9e, 0x90, 0xd8, 0xae, 0xdd, 0x39, 0xd0, 0x67,
                                 0x85, 0x0e, 0x18, 0x5b, 0xcb, 0x92, 0xc7, 0xbb, 0xff, 0x88, 0xfd, 0xe8, 0xf9, 0x33, 0x6c, 0x74};

    const uint8_t edk_provider_info[] =
      "asdfhasiufhiasuhviawurhgiuawrhefiuawhf" // master key ID
      "\x00\x00\x00\x80" // GCM tag length in bits
      "\x00\x00\x00\x0c" // IV length in bytes
      "\x1b\x48\x76\xb4\x7a\x10\x16\x19\xeb\x3f\x93\x1d"; // IV

    struct aws_cryptosdk_edk edk;
    edk.enc_data_key = aws_byte_buf_from_array(edk_bytes, sizeof(edk_bytes));
    edk.provider_id = aws_byte_buf_from_array(test_vector_provider_id, sizeof(test_vector_provider_id) - 1);
    edk.provider_info = aws_byte_buf_from_array(edk_provider_info, sizeof(edk_provider_info) - 1);

    aws_array_list_push_back(&req.encrypted_data_keys, (void *) &edk);
    
    struct aws_cryptosdk_decryption_materials * dec_mat = aws_cryptosdk_decryption_materials_new(alloc, req.alg);
    TEST_ASSERT_ADDR_NOT_NULL(dec_mat);

    TEST_ASSERT_INT_EQ(AWS_OP_SUCCESS, aws_cryptosdk_mk_decrypt_data_key(mk, dec_mat, &req));
    TEST_ASSERT_ADDR_NOT_NULL(dec_mat->unencrypted_data_key.buffer);
    TEST_ASSERT_BUF_EQ(dec_mat->unencrypted_data_key,
                       0x9b, 0x01, 0xc1, 0xaa, 0x62, 0x25, 0x1d, 0x0f, 0x16, 0xa0, 0xa2, 0x15, 0xea, 0xe4, 0xc2, 0x37,
                       0x4a, 0x8c, 0xc7, 0x9f, 0xfa, 0x3a, 0xe7, 0xa2, 0xa4, 0xa8, 0x1e, 0x83, 0xba, 0x38, 0x23, 0x16);

    aws_cryptosdk_decryption_materials_destroy(dec_mat);
    aws_array_list_clean_up(&req.encrypted_data_keys);
    aws_hash_table_clean_up(&enc_context);
    aws_cryptosdk_mk_destroy(mk);
    return 0;
}

struct test_case raw_aes_mk_test_cases[] = {
    { "raw_aes_mk", "serialize_valid_provider_info", serialize_valid_provider_info },
    { "raw_aes_mk", "parse_valid_provider_info", parse_valid_provider_info },
    { "raw_aes_mk", "decrypt_data_key_empty_encryption_context", decrypt_data_key_empty_encryption_context },
    { "raw_aes_mk", "decrypt_data_key_with_signature", decrypt_data_key_with_signature },
    { NULL }
};
