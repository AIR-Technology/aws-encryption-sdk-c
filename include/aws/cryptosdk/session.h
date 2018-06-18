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

#ifndef AWS_CRYPTOSDK_SESSION_H
#define AWS_CRYPTOSDK_SESSION_H

#include <stdbool.h>

#include <aws/common/common.h>
#include <aws/cryptosdk/header.h>

#ifdef __cplusplus
extern "C" {
#endif

struct aws_cryptosdk_session;

struct aws_cryptosdk_session *aws_cryptosdk_session_new(
    struct aws_allocator *allocator
);

void aws_cryptosdk_session_destroy(struct aws_cryptosdk_session *session);

/**
 * Prepares the session to start a new decryption operation.
 * The session will retain any configured crypto material manager,
 * master key provider, or master key, as well as its associated
 * allocator. All other state will be reset to prepare for processing
 * a new message.
 *
 * This method can be used to reset a session currently in an error
 * state as well.
 */
int aws_cryptosdk_session_init_decrypt(
    struct aws_cryptosdk_session *session
);

/**
 * Prepares the session to start a new encryption operation.
 * The session will retain any configured crypto material manager,
 * master key provider, or master key, as well as its associated
 * allocator. All other state will be reset to prepare for processing
 * a new message.
 *
 * This method can be used to reset a session currently in an error
 * state as well.
 */
int aws_cryptosdk_session_init_encrypt(
    struct aws_cryptosdk_session *session
);

/**
 * Sets the frame size to use for encryption. If zero is specified, the message
 * will be processed in an unframed mode.
 */
int aws_cryptosdk_session_set_frame_size(
    struct aws_cryptosdk_session *session,
    size_t frame_size
);

/**
 * Sets the precise size of the message to encrypt. This function must be
 * called exactly once during an encrypt operation; it need not be called
 * before beginning to pass data, but providing the message size up front may
 * improve efficiency.
 *
 * If the session has already processed more than message_size bytes, or if this
 * method is called more than once, the session will enter an error state.
 *
 * This method is how the end of data is determined; if this is not called, then
 * process will be unable to write out the end-of-message frames (and instead will
 * continue to request additional data).
 *
 * Note that if the frame size is set to zero (i.e. this is a one-shot encrypt),
 * a message size must be set before any input data can be processed.
 */
int aws_cryptosdk_session_set_message_size(
    struct aws_cryptosdk_session *session,
    uint64_t message_size
);

/**
 * Provides an upper bound on the message size. This hint is useful when the exact
 * message size is not known up-front, but the configured CMM still needs to make use
 * of message size information. For example, the caching CMM enforces an upper bound
 * on the number of bytes encrypted under a cached key; if this method is not called,
 * it must assume that you are processing an unbounded amount of data, and it will
 * therefore bypass the cache.
 *
 * This method may be called more than once (the smallest call wins). The session will
 * enter an error state if more than the smallest bound of data is written, or if the
 * precise message size passed to aws_cryptosdk_session_set_message_size exceeds the
 * bound.
 *
 * It is recommended that the bound be set before invoking aws_cryptosdk_session_process;
 * failing to do so will result in the CMM being passed an unbounded message size.
 */
int aws_cryptosdk_session_set_message_bound(
    struct aws_cryptosdk_session *session,
    uint64_t max_message_size
);

#if 0
// TODO: CMMs will be implemented later. For now we assume an all-zero
// data key.

int aws_cryptosdk_session_set_cmm(
    struct aws_cryptosdk_session *session,
    struct aws_cryptosdk_cmm *cmm
);

// MKP variant...
// MK variant...
#endif

#if 0
/**
 * Retrieves a reference to the header associated with a session.
 * This function will fail with an AWS_CRYPTOSDK_ERR_BAD_STATE error
 * if the header is not yet available. This will happen if, for encrypt,
 * process has not yet been called, or if on decrypt not enough data has
 * been supplied to deserialize the header.
 *
 * The header returned is owned by the session and must not be modified.
 * It will be destroyed when the session is reinitialized or destroyed.
 */
int aws_cryptosdk_session_get_header(
    const struct aws_cryptosdk_session *session,
    const struct aws_cryptosdk_header **header
);
#endif

/**
 * Attempts to process some data through the cryptosdk session.
 * This method may do any combination of
 *   1. Consuming some data from the input buffer
 *   2. Producing some data in the output buffer
 *   3. Entering an error state, and raising the error in question.
 *
 * The data referenced by the input and output cursors must not overlap.
 * If this method raises an error, the contents of the output buffer will
 * be zeroed. The buffer referenced by the input buffer will never be modified.
 *
 * If there is insufficient output space and/or insufficient input
 * data, this method may not make any progress. The aws_cryptosdk_session_estimate_buf
 * method may be used to determine how much data may be needed at
 * this stage of processing. Note that these estimates may change once
 * the header has been parsed.
 *
 * Upon return, *out_bytes_written and *in_bytes_read will report the number
 * of bytes consumed from the output and input buffers, respectively.
 *
 * This method will return successfully unless the session has entered
 * an error state. Use aws_cryptosdk_session_is_done to determine if the
 * entire message has been processed.
 */
int aws_cryptosdk_session_process(
    struct aws_cryptosdk_session *session,
    uint8_t *outp, size_t outlen, size_t *out_bytes_written,
    const uint8_t *inp, size_t inlen, size_t *in_bytes_read
);

/**
 * Returns true if the session has finished processing the entire message.
 */
bool aws_cryptosdk_session_is_done(const struct aws_cryptosdk_session *session);

/**
 * Estimates the amount of buffer space needed to make forward progress.
 * Supplying the amount of data indicated here to _process guarantees that
 * some kind of progress will be made (if only to enter an error state).
 * Note that these values will change after initial header (de)serialization
 * is completed.
 *
 * This method never fails, but if the session is in an error state or is
 * only partially initialized, the returned sizes will be 1.
 */
void aws_cryptosdk_session_estimate_buf(
    const struct aws_cryptosdk_session *session,
    size_t * AWS_RESTRICT outbuf_needed,
    size_t * AWS_RESTRICT inbuf_needed
);

#ifdef __cplusplus
}
#endif

#endif
