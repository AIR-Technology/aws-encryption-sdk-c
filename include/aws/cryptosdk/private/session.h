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

#ifndef AWS_CRYPTOSDK_PRIVATE_SESSION_H
#define AWS_CRYPTOSDK_PRIVATE_SESSION_H

#include <aws/cryptosdk/session.h>
#include <aws/cryptosdk/private/header.h>
#include <aws/cryptosdk/private/cipher.h>

#define DEFAULT_FRAME_SIZE (256 * 1024)
#define MAX_FRAME_SIZE 0xFFFFFFFF

enum session_state {
/*** Common states ***/

/* State ST_CONFIG: Initial configuration. No data has been supplied */
    ST_CONFIG = 0,
/* State ST_ERROR: De/encryption failure. No data will be processed until reset */
    ST_ERROR,
    ST_DONE,

/*** Decrypt path ***/

    ST_READ_HEADER,
    ST_UNWRAP_KEY,
    ST_DECRYPT_BODY,
    ST_CHECK_TRAILER,

/*** Encrypt path ***/

    ST_GEN_KEY,
    ST_WRITE_HEADER,
    ST_ENCRYPT_BODY,
    ST_WRITE_TRAILER,
};

struct aws_cryptosdk_session {
    struct aws_allocator *alloc;
    int error;
    enum aws_cryptosdk_mode mode;
    enum session_state state;

    /* Encrypt mode configuration */
    uint64_t precise_size; /* Exact size of message */
    uint64_t size_bound;   /* Maximum message size */
    uint64_t data_so_far;  /* Bytes processed thus far */
    bool precise_size_known;

    /* The actual header, if parsed */
    uint8_t *header_copy;
    size_t header_size;
    struct aws_cryptosdk_hdr header;
    uint64_t frame_size;   /* Frame size, zero for unframed */

    /* Estimate for the amount of input data needed to make progress. */
    size_t input_size_estimate;

    /* Estimate for the amount of output buffer needed to make progress. */
    size_t output_size_estimate;

    uint64_t frame_seqno;

    const struct aws_cryptosdk_alg_properties *alg_props;

    /* Decrypted, derived (if applicable) content key */
    struct content_key content_key;
};


/* Common session routines */

void session_change_state(struct aws_cryptosdk_session *session, enum session_state new_state);
int fail_session(struct aws_cryptosdk_session *session, int error_code);

/* Decrypt path */
int unwrap_keys(struct aws_cryptosdk_session * restrict session);
int try_parse_header(
    struct aws_cryptosdk_session * restrict session,
    struct aws_byte_cursor * restrict input
);
int try_decrypt_body(
    struct aws_cryptosdk_session * restrict session,
    struct aws_byte_cursor * restrict poutput,
    struct aws_byte_cursor * restrict pinput
);

/* Encrypt path */
void encrypt_compute_body_estimate(struct aws_cryptosdk_session *session);

int try_gen_key(struct aws_cryptosdk_session *session);
int try_write_header(struct aws_cryptosdk_session *session, struct aws_byte_cursor *output);
int try_encrypt_body(
    struct aws_cryptosdk_session * restrict session,
    struct aws_byte_cursor * restrict poutput,
    struct aws_byte_cursor * restrict pinput
);



#endif