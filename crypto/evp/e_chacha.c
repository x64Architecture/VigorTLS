/*
 * Copyright (c) 2014 - 2016, Kurt Cancemi (kurt@x64architecture.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_CHACHA

#include <string.h>

#include <openssl/chacha.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

#include "evp_locl.h"

typedef struct {
    uint8_t key[32];
    uint8_t nonce[12];
} EVP_CHACHA_CTX;

static int chacha_init(EVP_CIPHER_CTX *ctx, const uint8_t *key,
                       const uint8_t *iv, int enc)
{
    EVP_CHACHA_CTX *chacha_ctx = (EVP_CHACHA_CTX *)ctx->cipher_data;

    if (key)
        memcpy(chacha_ctx->key, key, 32);

    if (iv)
        memcpy(chacha_ctx->nonce, iv, 12);

    return 1;
}

static int chacha_cipher(EVP_CIPHER_CTX *ctx, uint8_t *out, const uint8_t *in,
                         size_t len)
{
    EVP_CHACHA_CTX *chacha_ctx = (EVP_CHACHA_CTX *)ctx->cipher_data;

    CRYPTO_chacha_20(out, in, len, chacha_ctx->key, chacha_ctx->nonce, 0);

    return 1;
}

static const EVP_CIPHER chacha20_cipher = {
    .nid = NID_chacha20,
    .block_size = 1,
    .key_len = 32,
    .iv_len = 12,
    .flags = EVP_CIPH_STREAM_CIPHER,
    .init = chacha_init,
    .do_cipher = chacha_cipher,
    .ctx_size = sizeof(EVP_CHACHA_CTX)
};

const EVP_CIPHER *EVP_chacha20(void)
{
    return (&chacha20_cipher);
}

#endif
