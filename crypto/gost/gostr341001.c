/*
 * Copyright (c) 2014 Dmitry Eremin-Solenikov <dbaryshkov@gmail.com>
 * Copyright (c) 2005-2006 Cryptocom LTD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

#include <string.h>

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_GOST
#include <openssl/err.h>
#include <openssl/gost.h>
#include "gost_locl.h"

/* Convert little-endian byte array into bignum */
BIGNUM *GOST_le2bn(const unsigned char *buf, size_t len, BIGNUM *bn)
{
    unsigned char temp[64];
    int i;
    if (len > 64)
        return NULL;

    for (i = 0; i < len; i++) {
        temp[len - 1 - i] = buf[i];
    }

    return BN_bin2bn(temp, len, bn);
}

int GOST_bn2le(BIGNUM *bn, unsigned char *buf, int len)
{
    unsigned char temp[64];
    int i, bytes;

    bytes = BN_num_bytes(bn);
    if (len > 64 || bytes > len)
        return 0;

    BN_bn2bin(bn, temp);

    for (i = 0; i < bytes; i++) {
        buf[bytes - 1 - i] = temp[i];
    }

    memset(buf + bytes, 0, len - bytes);

    return 1;
}

int gost2001_compute_public(GOST_KEY *ec)
{
    const EC_GROUP *group = GOST_KEY_get0_group(ec);
    EC_POINT *pub_key = NULL;
    const BIGNUM *priv_key = NULL;
    BN_CTX *ctx = NULL;
    int ok = 0;

    if (!group) {
        GOSTerr(GOST_F_GOST2001_COMPUTE_PUBLIC, GOST_R_KEY_IS_NOT_INITIALIZED);
        return 0;
    }
    ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    if (!(priv_key = GOST_KEY_get0_private_key(ec))) {
        GOSTerr(GOST_F_GOST2001_COMPUTE_PUBLIC, ERR_R_EC_LIB);
        goto err;
    }

    pub_key = EC_POINT_new(group);
    if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx)) {
        GOSTerr(GOST_F_GOST2001_COMPUTE_PUBLIC, ERR_R_EC_LIB);
        goto err;
    }
    if (!GOST_KEY_set_public_key(ec, pub_key)) {
        GOSTerr(GOST_F_GOST2001_COMPUTE_PUBLIC, ERR_R_EC_LIB);
        goto err;
    }
    ok = 256;
err:
    BN_CTX_end(ctx);
    EC_POINT_free(pub_key);
    BN_CTX_free(ctx);
    return ok;
}

ECDSA_SIG *gost2001_do_sign(BIGNUM *md, GOST_KEY *eckey)
{
    ECDSA_SIG *newsig = NULL;
    BIGNUM *order = NULL;
    const EC_GROUP *group;
    const BIGNUM *priv_key;
    BIGNUM *r = NULL, *s = NULL, *X = NULL, *tmp = NULL, *tmp2 = NULL, *k = NULL,
           *e = NULL;
    EC_POINT *C = NULL;
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    newsig = ECDSA_SIG_new();
    if (!newsig) {
        GOSTerr(GOST_F_GOST2001_DO_SIGN, ERR_R_MALLOC_FAILURE);
        goto err;
    }
    s = newsig->s;
    r = newsig->r;
    group = GOST_KEY_get0_group(eckey);
    order = BN_CTX_get(ctx);
    EC_GROUP_get_order(group, order, ctx);
    priv_key = GOST_KEY_get0_private_key(eckey);
    e = BN_CTX_get(ctx);
    BN_mod(e, md, order, ctx);
    if (BN_is_zero(e)) {
        BN_one(e);
    }
    k = BN_CTX_get(ctx);
    X = BN_CTX_get(ctx);
    C = EC_POINT_new(group);
    do {
        do {
            if (!BN_rand_range(k, order)) {
                GOSTerr(GOST_F_GOST2001_DO_SIGN,
                        GOST_R_RANDOM_NUMBER_GENERATOR_FAILED);
                ECDSA_SIG_free(newsig);
                newsig = NULL;
                goto err;
            }
            /* We do not want timing information to leak the length of k,
             * so we compute G*k using an equivalent scalar of fixed
             * bit-length. */
            if (!BN_add(k, k, order))
                goto err;
            if (BN_num_bits(k) <= BN_num_bits(order))
                if (!BN_add(k, k, order))
                    goto err;

            if (!EC_POINT_mul(group, C, k, NULL, NULL, ctx)) {
                GOSTerr(GOST_F_GOST2001_DO_SIGN, ERR_R_EC_LIB);
                ECDSA_SIG_free(newsig);
                newsig = NULL;
                goto err;
            }
            if (!EC_POINT_get_affine_coordinates_GFp(group, C, X, NULL, ctx)) {
                GOSTerr(GOST_F_GOST2001_DO_SIGN, ERR_R_EC_LIB);
                ECDSA_SIG_free(newsig);
                newsig = NULL;
                goto err;
            }
            BN_nnmod(r, X, order, ctx);
        } while (BN_is_zero(r));
        /* s =  (r*priv_key+k*e) mod order */
        if (!tmp)
            tmp = BN_CTX_get(ctx);
        BN_mod_mul(tmp, priv_key, r, order, ctx);
        if (!tmp2)
            tmp2 = BN_CTX_get(ctx);
        BN_mod_mul(tmp2, k, e, order, ctx);
        BN_mod_add(s, tmp, tmp2, order, ctx);
    } while (BN_is_zero(s));

err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_POINT_free(C);
    BN_free(md);
    return newsig;
}

int gost2001_do_verify(BIGNUM *md, ECDSA_SIG *sig, GOST_KEY *ec)
{
    BN_CTX *ctx = BN_CTX_new();
    const EC_GROUP *group = GOST_KEY_get0_group(ec);
    BIGNUM *order;
    BIGNUM *e = NULL, *R = NULL, *v = NULL, *z1 = NULL, *z2 = NULL;
    BIGNUM *X = NULL, *tmp = NULL;
    EC_POINT *C = NULL;
    const EC_POINT *pub_key = NULL;
    int ok = 0;

    BN_CTX_start(ctx);
    order = BN_CTX_get(ctx);
    e = BN_CTX_get(ctx);
    z1 = BN_CTX_get(ctx);
    z2 = BN_CTX_get(ctx);
    tmp = BN_CTX_get(ctx);
    X = BN_CTX_get(ctx);
    R = BN_CTX_get(ctx);
    v = BN_CTX_get(ctx);

    EC_GROUP_get_order(group, order, ctx);
    pub_key = GOST_KEY_get0_public_key(ec);
    if (BN_is_zero(sig->s) || BN_is_zero(sig->r) || (BN_cmp(sig->s, order) >= 1)
        || (BN_cmp(sig->r, order) >= 1)) {
        GOSTerr(GOST_F_GOST2001_DO_VERIFY, GOST_R_SIGNATURE_PARTS_GREATER_THAN_Q);
        goto err;
    }

    BN_mod(e, md, order, ctx);
    if (BN_is_zero(e))
        BN_one(e);
    v = BN_mod_inverse(v, e, order, ctx);
    BN_mod_mul(z1, sig->s, v, order, ctx);
    BN_sub(tmp, order, sig->r);
    BN_mod_mul(z2, tmp, v, order, ctx);
    C = EC_POINT_new(group);
    if (!EC_POINT_mul(group, C, z1, pub_key, z2, ctx)) {
        GOSTerr(GOST_F_GOST2001_DO_VERIFY, ERR_R_EC_LIB);
        goto err;
    }
    if (!EC_POINT_get_affine_coordinates_GFp(group, C, X, NULL, ctx)) {
        GOSTerr(GOST_F_GOST2001_DO_VERIFY, ERR_R_EC_LIB);
        goto err;
    }
    BN_mod(R, X, order, ctx);
    if (BN_cmp(R, sig->r) != 0) {
        GOSTerr(GOST_F_GOST2001_DO_VERIFY, GOST_R_SIGNATURE_MISMATCH);
    } else {
        ok = 1;
    }
err:
    EC_POINT_free(C);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return ok;
}

/* Implementation of CryptoPro VKO 34.10-2001 algorithm */
void VKO_compute_key(BIGNUM *X, BIGNUM *Y, const GOST_KEY *pkey, GOST_KEY *priv_key,
                     const BIGNUM *ukm)
{
    BIGNUM *p = NULL, *order = NULL;
    const BIGNUM *key = GOST_KEY_get0_private_key(priv_key);
    const EC_POINT *pub_key = GOST_KEY_get0_public_key(pkey);
    EC_POINT *pnt = EC_POINT_new(GOST_KEY_get0_group(priv_key));
    BN_CTX *ctx = BN_CTX_new();

    BN_CTX_start(ctx);
    p = BN_CTX_get(ctx);
    order = BN_CTX_get(ctx);
    EC_GROUP_get_order(GOST_KEY_get0_group(priv_key), order, ctx);
    BN_mod_mul(p, key, ukm, order, ctx);
    EC_POINT_mul(GOST_KEY_get0_group(priv_key), pnt, NULL, pub_key, p, ctx);
    EC_POINT_get_affine_coordinates_GFp(GOST_KEY_get0_group(priv_key), pnt, X, Y,
                                        ctx);
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    EC_POINT_free(pnt);
}

int gost2001_keygen(GOST_KEY *ec)
{
    BIGNUM *order = BN_new(), *d = BN_new();
    const EC_GROUP *group = GOST_KEY_get0_group(ec);
    EC_GROUP_get_order(group, order, NULL);

    do {
        if (!BN_rand_range(d, order)) {
            GOSTerr(GOST_F_GOST2001_KEYGEN, GOST_R_RANDOM_NUMBER_GENERATOR_FAILED);
            BN_free(d);
            BN_free(order);
            return 0;
        }
    } while (BN_is_zero(d));
    GOST_KEY_set_private_key(ec, d);
    BN_free(d);
    BN_free(order);
    return gost2001_compute_public(ec);
}
#endif
