/*
 * Copyright 2006-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <limits.h>
#include <string.h>

#include <openssl/asn1t.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/evp.h>

#include "internal/evp_int.h"
#include "rsa_locl.h"

/* RSA pkey context structure */

typedef struct {
    /* Key gen parameters */
    int nbits;
    BIGNUM *pub_exp;
    /* Keygen callback info */
    int gentmp[2];
    /* RSA padding mode */
    int pad_mode;
    /* message digest */
    const EVP_MD *md;
    /* message digest for MGF1 */
    const EVP_MD *mgf1md;
    /* PSS salt length */
    int saltlen;
    /* Temp buffer */
    uint8_t *tbuf;
    /* OAEP label */
    uint8_t *oaep_label;
    size_t oaep_labellen;
} RSA_PKEY_CTX;

static int pkey_rsa_init(EVP_PKEY_CTX *ctx)
{
    RSA_PKEY_CTX *rctx;
    rctx = calloc(1, sizeof(RSA_PKEY_CTX));
    if (rctx == NULL)
        return 0;
    rctx->nbits = 1024;
    rctx->pad_mode = RSA_PKCS1_PADDING;

    rctx->saltlen = -2;
    
    

    ctx->data = rctx;
    ctx->keygen_info = rctx->gentmp;
    ctx->keygen_info_count = 2;

    return 1;
}

static int pkey_rsa_copy(EVP_PKEY_CTX *dst, EVP_PKEY_CTX *src)
{
    RSA_PKEY_CTX *dctx, *sctx;
    if (!pkey_rsa_init(dst))
        return 0;
    sctx = src->data;
    dctx = dst->data;
    dctx->nbits = sctx->nbits;
    if (sctx->pub_exp) {
        dctx->pub_exp = BN_dup(sctx->pub_exp);
        if (!dctx->pub_exp)
            return 0;
    }
    dctx->pad_mode = sctx->pad_mode;
    dctx->md = sctx->md;
    dctx->mgf1md = sctx->mgf1md;
    if (sctx->oaep_label != NULL) {
        free(dctx->oaep_label);
        dctx->oaep_label = malloc(sctx->oaep_labellen);
        if (dctx->oaep_label == NULL)
            return 0;
        memcpy(dctx->oaep_label, sctx->oaep_label, sctx->oaep_labellen);
        dctx->oaep_labellen = sctx->oaep_labellen;
    }
    return 1;
}

static int setup_tbuf(RSA_PKEY_CTX *ctx, EVP_PKEY_CTX *pk)
{
    if (ctx->tbuf != NULL)
        return 1;
    ctx->tbuf = malloc(EVP_PKEY_size(pk->pkey));
    if (ctx->tbuf == NULL)
        return 0;
    return 1;
}

static void pkey_rsa_cleanup(EVP_PKEY_CTX *ctx)
{
    RSA_PKEY_CTX *rctx = ctx->data;
    if (rctx == NULL)
        return;
    BN_free(rctx->pub_exp);
    free(rctx->tbuf);
    free(rctx->oaep_label);
    free(rctx);
}

static int pkey_rsa_sign(EVP_PKEY_CTX *ctx, uint8_t *sig, size_t *siglen,
                         const uint8_t *tbs, size_t tbslen)
{
    int ret;
    RSA_PKEY_CTX *rctx = ctx->data;
    RSA *rsa = ctx->pkey->pkey.rsa;

    if (rctx->md) {
        if (tbslen != (size_t)EVP_MD_size(rctx->md)) {
            RSAerr(RSA_F_PKEY_RSA_SIGN,
                   RSA_R_INVALID_DIGEST_LENGTH);
            return -1;
        }

       if (rctx->pad_mode == RSA_X931_PADDING) {
           if ((size_t)EVP_PKEY_size(ctx->pkey) < tbslen + 1) {
               RSAerr(RSA_F_PKEY_RSA_SIGN, RSA_R_KEY_SIZE_TOO_SMALL);
               return -1;
           }
           if (!setup_tbuf(rctx, ctx)) {
               RSAerr(RSA_F_PKEY_RSA_SIGN, ERR_R_MALLOC_FAILURE);
               return -1;
           }
           memcpy(rctx->tbuf, tbs, tbslen);
           rctx->tbuf[tbslen] = RSA_X931_hash_id(EVP_MD_type(rctx->md));
           ret = RSA_private_encrypt(tbslen + 1, rctx->tbuf, sig, rsa,
                                     RSA_X931_PADDING);
        } else if (rctx->pad_mode == RSA_PKCS1_PADDING) {
            unsigned int sltmp;
            ret = RSA_sign(EVP_MD_type(rctx->md),
                           tbs, tbslen, sig, &sltmp, rsa);
            if (ret <= 0)
                return ret;
            ret = sltmp;
        } else if (rctx->pad_mode == RSA_PKCS1_PSS_PADDING) {
            if (!setup_tbuf(rctx, ctx))
                return -1;
            if (!RSA_padding_add_PKCS1_PSS_mgf1(rsa,
                                                rctx->tbuf, tbs,
                                                rctx->md, rctx->mgf1md,
                                                rctx->saltlen))
                return -1;
            ret = RSA_private_encrypt(RSA_size(rsa), rctx->tbuf,
                                      sig, rsa, RSA_NO_PADDING);
        } else
            return -1;
    } else
        ret = RSA_private_encrypt(tbslen, tbs, sig, ctx->pkey->pkey.rsa,
                                  rctx->pad_mode);
    if (ret < 0)
        return ret;
    *siglen = ret;
    return 1;
}

static int pkey_rsa_verifyrecover(EVP_PKEY_CTX *ctx,
                                  uint8_t *rout, size_t *routlen,
                                  const uint8_t *sig, size_t siglen)
{
    int ret;
    RSA_PKEY_CTX *rctx = ctx->data;

    if (rctx->md) {
        if (rctx->pad_mode == RSA_X931_PADDING) {
            if (!setup_tbuf(rctx, ctx))
                return -1;
            ret = RSA_public_decrypt(siglen, sig,
                                     rctx->tbuf, ctx->pkey->pkey.rsa,
                                     RSA_X931_PADDING);
            if (ret < 1)
                return 0;
            ret--;
            if (rctx->tbuf[ret] != RSA_X931_hash_id(EVP_MD_type(rctx->md))) {
                RSAerr(RSA_F_PKEY_RSA_VERIFYRECOVER,
                       RSA_R_ALGORITHM_MISMATCH);
                return 0;
            }
            if (ret != EVP_MD_size(rctx->md)) {
                RSAerr(RSA_F_PKEY_RSA_VERIFYRECOVER,
                       RSA_R_INVALID_DIGEST_LENGTH);
                return 0;
            }
            if (rout)
                memcpy(rout, rctx->tbuf, ret);
        } else if (rctx->pad_mode == RSA_PKCS1_PADDING) {
            size_t sltmp;
            ret = int_rsa_verify(EVP_MD_type(rctx->md),
                                 NULL, 0, rout, &sltmp,
                                 sig, siglen, ctx->pkey->pkey.rsa);
            if (ret <= 0)
                return 0;
            ret = sltmp;
        } else
            return -1;
    } else
        ret = RSA_public_decrypt(siglen, sig, rout, ctx->pkey->pkey.rsa,
                                 rctx->pad_mode);
    if (ret < 0)
        return ret;
    *routlen = ret;
    return 1;
}

static int pkey_rsa_verify(EVP_PKEY_CTX *ctx,
                           const uint8_t *sig, size_t siglen,
                           const uint8_t *tbs, size_t tbslen)
{
    RSA_PKEY_CTX *rctx = ctx->data;
    RSA *rsa = ctx->pkey->pkey.rsa;
    size_t rslen;
    if (rctx->md) {
        if (rctx->pad_mode == RSA_PKCS1_PADDING)
            return RSA_verify(EVP_MD_type(rctx->md), tbs, tbslen,
                              sig, siglen, rsa);
        if (rctx->pad_mode == RSA_X931_PADDING) {
            if (pkey_rsa_verifyrecover(ctx, NULL, &rslen,
                                       sig, siglen) <= 0)
                return 0;
        } else if (rctx->pad_mode == RSA_PKCS1_PSS_PADDING) {
            int ret;
            if (!setup_tbuf(rctx, ctx))
                return -1;
            ret = RSA_public_decrypt(siglen, sig, rctx->tbuf,
                                     rsa, RSA_NO_PADDING);
            if (ret <= 0)
                return 0;
            ret = RSA_verify_PKCS1_PSS_mgf1(rsa, tbs,
                                            rctx->md, rctx->mgf1md,
                                            rctx->tbuf, rctx->saltlen);
            if (ret <= 0)
                return 0;
            return 1;
        } else
            return -1;
    } else {
        if (!setup_tbuf(rctx, ctx))
            return -1;
        rslen = RSA_public_decrypt(siglen, sig, rctx->tbuf,
                                   rsa, rctx->pad_mode);
        if (rslen == 0)
            return 0;
    }

    if ((rslen != tbslen) || memcmp(tbs, rctx->tbuf, rslen))
        return 0;

    return 1;
}

static int pkey_rsa_encrypt(EVP_PKEY_CTX *ctx,
                            uint8_t *out, size_t *outlen,
                            const uint8_t *in, size_t inlen)
{
    int ret;
    RSA_PKEY_CTX *rctx = ctx->data;
    if (rctx->pad_mode == RSA_PKCS1_OAEP_PADDING) {
        int klen = RSA_size(ctx->pkey->pkey.rsa);
        if (!setup_tbuf(rctx, ctx))
            return -1;
        if (!RSA_padding_add_PKCS1_OAEP_mgf1(rctx->tbuf, klen, in, inlen,
                                             rctx->oaep_label,
                                             rctx->oaep_labellen,
                                             rctx->md, rctx->mgf1md))
            return -1;
        ret = RSA_public_encrypt(klen, rctx->tbuf, out, ctx->pkey->pkey.rsa,
                                 RSA_NO_PADDING);
    } else {
        ret = RSA_public_encrypt(inlen, in, out, ctx->pkey->pkey.rsa,
                                 rctx->pad_mode);
    }
    if (ret < 0)
        return ret;
    *outlen = ret;
    return 1;
}

static int pkey_rsa_decrypt(EVP_PKEY_CTX *ctx,
                            uint8_t *out, size_t *outlen,
                            const uint8_t *in, size_t inlen)
{
    int ret;
    RSA_PKEY_CTX *rctx = ctx->data;
    
    if (rctx->pad_mode == RSA_PKCS1_OAEP_PADDING) {
        int i;
        if (!setup_tbuf(rctx, ctx))
            return -1;
        ret = RSA_private_decrypt(inlen, in, rctx->tbuf, ctx->pkey->pkey.rsa,
                                  RSA_NO_PADDING);
        if (ret <= 0)
            return ret;
        for (i = 0; i < ret; i++) {
            if (rctx->tbuf[i])
                break;
        }
        ret = RSA_padding_check_PKCS1_OAEP_mgf1(out, ret, rctx->tbuf + i,
                                                ret - i, ret, rctx->oaep_label,
                                                rctx->oaep_labellen,
                                                rctx->md, rctx->mgf1md);
    } else {
        ret = RSA_private_decrypt(inlen, in, out, ctx->pkey->pkey.rsa,
                                  rctx->pad_mode);
    }
    if (ret < 0)
        return ret;
    *outlen = ret;
    return 1;
}

static int check_padding_md(const EVP_MD *md, int padding)
{
    if (!md)
        return 1;

    if (padding == RSA_NO_PADDING) {
        RSAerr(RSA_F_CHECK_PADDING_MD, RSA_R_INVALID_PADDING_MODE);
        return 0;
    }

    if (padding == RSA_X931_PADDING) {
        if (RSA_X931_hash_id(EVP_MD_type(md)) == -1) {
            RSAerr(RSA_F_CHECK_PADDING_MD, RSA_R_INVALID_X931_DIGEST);
            return 0;
        }
        return 1;
    }

    return 1;
}

static int pkey_rsa_ctrl(EVP_PKEY_CTX *ctx, int type, int p1, void *p2)
{
    RSA_PKEY_CTX *rctx = ctx->data;
    switch (type) {
        case EVP_PKEY_CTRL_RSA_PADDING:
            if ((p1 >= RSA_PKCS1_PADDING) && (p1 <= RSA_PKCS1_PSS_PADDING)) {
                if (!check_padding_md(rctx->md, p1))
                    return 0;
                if (p1 == RSA_PKCS1_PSS_PADDING) {
                    if (!(ctx->operation & (EVP_PKEY_OP_SIGN | EVP_PKEY_OP_VERIFY)))
                        goto bad_pad;
                    if (!rctx->md)
                        rctx->md = EVP_sha1();
                }
                if (p1 == RSA_PKCS1_OAEP_PADDING) {
                    if (!(ctx->operation & EVP_PKEY_OP_TYPE_CRYPT))
                        goto bad_pad;
                    if (!rctx->md)
                        rctx->md = EVP_sha1();
                }
                rctx->pad_mode = p1;
                return 1;
            }
        bad_pad:
            RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_ILLEGAL_OR_UNSUPPORTED_PADDING_MODE);
            return -2;

        case EVP_PKEY_CTRL_GET_RSA_PADDING:
            *(int *)p2 = rctx->pad_mode;
            return 1;

        case EVP_PKEY_CTRL_RSA_PSS_SALTLEN:
        case EVP_PKEY_CTRL_GET_RSA_PSS_SALTLEN:
            if (rctx->pad_mode != RSA_PKCS1_PSS_PADDING) {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_PSS_SALTLEN);
                return -2;
            }
            if (type == EVP_PKEY_CTRL_GET_RSA_PSS_SALTLEN)
                *(int *)p2 = rctx->saltlen;
            else {
                if (p1 < -2)
                    return -2;
                rctx->saltlen = p1;
            }
            return 1;

        case EVP_PKEY_CTRL_RSA_KEYGEN_BITS:
            if (p1 < 256) {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_KEYBITS);
                return -2;
            }
            rctx->nbits = p1;
            return 1;

        case EVP_PKEY_CTRL_RSA_KEYGEN_PUBEXP:
            if (p2 == NULL || !BN_is_odd((BIGNUM *)p2) ||
                BN_is_one((BIGNUM *)p2))
            {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_BAD_E_VALUE);
                return -2;
            }
            BN_free(rctx->pub_exp);
            rctx->pub_exp = p2;
            return 1;
            
        case EVP_PKEY_CTRL_RSA_OAEP_MD:
        case EVP_PKEY_CTRL_GET_RSA_OAEP_MD:
            if (rctx->pad_mode != RSA_PKCS1_OAEP_PADDING) {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_PADDING_MODE);
                return -2;
            }
            if (type == EVP_PKEY_CTRL_GET_RSA_OAEP_MD)
                *(const EVP_MD **)p2 = rctx->md;
            else
                rctx->md = p2;
            return 1;

        case EVP_PKEY_CTRL_MD:
            if (!check_padding_md(p2, rctx->pad_mode))
                return 0;
            rctx->md = p2;
            return 1;

        case EVP_PKEY_CTRL_GET_MD:
            *(const EVP_MD **)p2 = rctx->md;
            return 1;

        case EVP_PKEY_CTRL_RSA_MGF1_MD:
        case EVP_PKEY_CTRL_GET_RSA_MGF1_MD:
            if (rctx->pad_mode != RSA_PKCS1_PSS_PADDING &&
                rctx->pad_mode != RSA_PKCS1_OAEP_PADDING)
            {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_MGF1_MD);
                return -2;
            }
            if (type == EVP_PKEY_CTRL_GET_RSA_MGF1_MD) {
                if (rctx->mgf1md)
                    *(const EVP_MD **)p2 = rctx->mgf1md;
                else
                    *(const EVP_MD **)p2 = rctx->md;
            } else
                rctx->mgf1md = p2;
            return 1;
            
        case EVP_PKEY_CTRL_RSA_OAEP_LABEL:
            if (rctx->pad_mode != RSA_PKCS1_OAEP_PADDING) {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_PADDING_MODE);
                return -2;
            }
            free(rctx->oaep_label);
            if (p2 && p1 > 0) {
                rctx->oaep_label = p2;
                rctx->oaep_labellen = p1;
            } else {
                rctx->oaep_label = NULL;
                rctx->oaep_labellen = 0;
            }
            return 1;
            
        case EVP_PKEY_CTRL_GET_RSA_OAEP_LABEL:
            if (rctx->pad_mode != RSA_PKCS1_OAEP_PADDING) {
                RSAerr(RSA_F_PKEY_RSA_CTRL, RSA_R_INVALID_PADDING_MODE);
                return -2;
            }
            *(uint8_t **)p2 = rctx->oaep_label;
            return rctx->oaep_labellen;

        case EVP_PKEY_CTRL_DIGESTINIT:
        case EVP_PKEY_CTRL_PKCS7_ENCRYPT:
        case EVP_PKEY_CTRL_PKCS7_DECRYPT:
        case EVP_PKEY_CTRL_PKCS7_SIGN:
            return 1;
        case EVP_PKEY_CTRL_PEER_KEY:
            RSAerr(RSA_F_PKEY_RSA_CTRL,
                   RSA_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            return -2;

        default:
            return -2;
    }
}

static int pkey_rsa_ctrl_str(EVP_PKEY_CTX *ctx, const char *type, const char *value)
{
    long lval;
    char *ep;
    if (!value) {
        RSAerr(RSA_F_PKEY_RSA_CTRL_STR, RSA_R_VALUE_MISSING);
        return 0;
    }
    if (strcmp(type, "rsa_padding_mode") == 0) {
        int pm;
        if (strcmp(value, "pkcs1") == 0)
            pm = RSA_PKCS1_PADDING;
        else if (strcmp(value, "sslv23") == 0)
            pm = RSA_SSLV23_PADDING;
        else if (strcmp(value, "none") == 0)
            pm = RSA_NO_PADDING;
        else if (strcmp(value, "oeap") == 0) /* FIXME: Typo was fixed in 2013 */
            pm = RSA_PKCS1_OAEP_PADDING;
        else if (strcmp(value, "oaep") == 0)
            pm = RSA_PKCS1_OAEP_PADDING;
        else if (strcmp(value, "x931") == 0)
            pm = RSA_X931_PADDING;
        else if (strcmp(value, "pss") == 0)
            pm = RSA_PKCS1_PSS_PADDING;
        else {
            RSAerr(RSA_F_PKEY_RSA_CTRL_STR, RSA_R_UNKNOWN_PADDING_TYPE);
            return -2;
        }
        return EVP_PKEY_CTX_set_rsa_padding(ctx, pm);
    }

    if (strcmp(type, "rsa_pss_saltlen") == 0) {
        int saltlen;

        lval = strtol(value, &ep, 10);
        if (value[0] == '\0' || *ep != '\0')
            goto invalid_number;
        if ((errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN)) ||
            (lval > INT_MAX || lval < INT_MIN))
            goto out_of_range;
        saltlen = lval;
        return EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, saltlen);
    }

    if (strcmp(type, "rsa_keygen_bits") == 0) {
        int nbits;

        errno = 0;
        lval = strtol(value, &ep, 10);
        if (value[0] == '\0' || *ep != '\0')
            goto invalid_number;
        if ((errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN)) ||
            (lval > INT_MAX || lval < INT_MIN))
            goto out_of_range;
        nbits = lval;
        return EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, nbits);
    }

    if (strcmp(type, "rsa_keygen_pubexp") == 0) {
        int ret;
        BIGNUM *pubexp = NULL;
        if (!BN_asc2bn(&pubexp, value)) {
            BN_free(pubexp);
            return 0;
        }
        ret = EVP_PKEY_CTX_set_rsa_keygen_pubexp(ctx, pubexp);
        if (ret <= 0)
            BN_free(pubexp);
        return ret;
    }
    
    if (strcmp(type, "rsa_mgf1_md") == 0) {
        const EVP_MD *md;
        if (!(md = EVP_get_digestbyname(value))) {
            RSAerr(RSA_F_PKEY_RSA_CTRL_STR, RSA_R_INVALID_DIGEST);
            return 0;
        }
        return EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, md);
    }
    
    if (strcmp(type, "rsa_oaep_md") == 0) {
        const EVP_MD *md;
        if (!(md = EVP_get_digestbyname(value))) {
            RSAerr(RSA_F_PKEY_RSA_CTRL_STR, RSA_R_INVALID_DIGEST);
            return 0;
        }
        return EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md);
    }
    if (strcmp(type, "rsa_oaep_label") == 0) {
        uint8_t *lab;
        long lablen;
        int ret;
        lab = string_to_hex(value, &lablen);
        if (lab == NULL)
            return 0;
        ret = EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, lab, lablen);
        if (ret <= 0)
            free(lab);
        return ret;
    }

invalid_number:
out_of_range:
    return -2;
}

static int pkey_rsa_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY *pkey)
{
    RSA *rsa = NULL;
    RSA_PKEY_CTX *rctx = ctx->data;
    BN_GENCB *pcb, cb;
    int ret;
    if (!rctx->pub_exp) {
        rctx->pub_exp = BN_new();
        if (!rctx->pub_exp || !BN_set_word(rctx->pub_exp, RSA_F4))
            return 0;
    }
    rsa = RSA_new();
    if (!rsa)
        return 0;
    if (ctx->pkey_gencb) {
        pcb = &cb;
        evp_pkey_set_cb_translate(pcb, ctx);
    } else
        pcb = NULL;
    ret = RSA_generate_key_ex(rsa, rctx->nbits, rctx->pub_exp, pcb);
    if (ret > 0)
        EVP_PKEY_assign_RSA(pkey, rsa);
    else
        RSA_free(rsa);
    return ret;
}

const EVP_PKEY_METHOD rsa_pkey_meth = {
    .pkey_id = EVP_PKEY_RSA,
    .flags = EVP_PKEY_FLAG_AUTOARGLEN,

    .init = pkey_rsa_init,
    .copy = pkey_rsa_copy,
    .cleanup = pkey_rsa_cleanup,

    .keygen = pkey_rsa_keygen,

    .sign = pkey_rsa_sign,

    .verify = pkey_rsa_verify,

    .verify_recover = pkey_rsa_verifyrecover,

    .encrypt = pkey_rsa_encrypt,

    .decrypt = pkey_rsa_decrypt,

    .ctrl = pkey_rsa_ctrl,
    .ctrl_str = pkey_rsa_ctrl_str
};
