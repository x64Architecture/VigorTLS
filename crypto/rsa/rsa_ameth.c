/*
 * Copyright 2006-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/asn1t.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include "internal/asn1_int.h"

static int rsa_pub_encode(X509_PUBKEY *pk, const EVP_PKEY *pkey)
{
    uint8_t *penc = NULL;
    int penclen;
    penclen = i2d_RSAPublicKey(pkey->pkey.rsa, &penc);
    if (penclen <= 0)
        return 0;
    if (X509_PUBKEY_set0_param(pk, OBJ_nid2obj(EVP_PKEY_RSA),
                               V_ASN1_NULL, NULL, penc, penclen))
        return 1;

    free(penc);
    return 0;
}

static int rsa_pub_decode(EVP_PKEY *pkey, X509_PUBKEY *pubkey)
{
    const uint8_t *p;
    int pklen;
    RSA *rsa = NULL;
    if (!X509_PUBKEY_get0_param(NULL, &p, &pklen, NULL, pubkey))
        return 0;
    if (!(rsa = d2i_RSAPublicKey(NULL, &p, pklen))) {
        RSAerr(RSA_F_RSA_PUB_DECODE, ERR_R_RSA_LIB);
        return 0;
    }
    EVP_PKEY_assign_RSA(pkey, rsa);
    return 1;
}

static int rsa_pub_cmp(const EVP_PKEY *a, const EVP_PKEY *b)
{
    if (BN_cmp(b->pkey.rsa->n, a->pkey.rsa->n) != 0
        || BN_cmp(b->pkey.rsa->e, a->pkey.rsa->e) != 0)
        return 0;
    return 1;
}

static int old_rsa_priv_decode(EVP_PKEY *pkey,
                               const uint8_t **pder, int derlen)
{
    RSA *rsa;
    if (!(rsa = d2i_RSAPrivateKey(NULL, pder, derlen))) {
        RSAerr(RSA_F_OLD_RSA_PRIV_DECODE, ERR_R_RSA_LIB);
        return 0;
    }
    EVP_PKEY_assign_RSA(pkey, rsa);
    return 1;
}

static int old_rsa_priv_encode(const EVP_PKEY *pkey, uint8_t **pder)
{
    return i2d_RSAPrivateKey(pkey->pkey.rsa, pder);
}

static int rsa_priv_encode(PKCS8_PRIV_KEY_INFO *p8, const EVP_PKEY *pkey)
{
    uint8_t *rk = NULL;
    int rklen;
    rklen = i2d_RSAPrivateKey(pkey->pkey.rsa, &rk);

    if (rklen <= 0) {
        RSAerr(RSA_F_RSA_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    if (!PKCS8_pkey_set0(p8, OBJ_nid2obj(NID_rsaEncryption), 0,
                         V_ASN1_NULL, NULL, rk, rklen)) {
        RSAerr(RSA_F_RSA_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    return 1;
}

static int rsa_priv_decode(EVP_PKEY *pkey, PKCS8_PRIV_KEY_INFO *p8)
{
    const uint8_t *p;
    int pklen;
    if (!PKCS8_pkey_get0(NULL, &p, &pklen, NULL, p8))
        return 0;
    return old_rsa_priv_decode(pkey, &p, pklen);
}

static int int_rsa_size(const EVP_PKEY *pkey)
{
    return RSA_size(pkey->pkey.rsa);
}

static int rsa_bits(const EVP_PKEY *pkey)
{
    return BN_num_bits(pkey->pkey.rsa->n);
}

static void int_rsa_free(EVP_PKEY *pkey)
{
    RSA_free(pkey->pkey.rsa);
}

static void update_buflen(const BIGNUM *b, size_t *pbuflen)
{
    size_t i;
    if (!b)
        return;
    if (*pbuflen < (i = (size_t)BN_num_bytes(b)))
        *pbuflen = i;
}

static int do_rsa_print(BIO *bp, const RSA *x, int off, int priv)
{
    const char *s, *str;
    uint8_t *m = NULL;
    int ret = 0, mod_len = 0;
    size_t buf_len = 0;

    update_buflen(x->n, &buf_len);
    update_buflen(x->e, &buf_len);

    if (priv) {
        update_buflen(x->d, &buf_len);
        update_buflen(x->p, &buf_len);
        update_buflen(x->q, &buf_len);
        update_buflen(x->dmp1, &buf_len);
        update_buflen(x->dmq1, &buf_len);
        update_buflen(x->iqmp, &buf_len);
    }

    m = malloc(buf_len + 10);
    if (m == NULL) {
        RSAerr(RSA_F_DO_RSA_PRINT, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if (x->n != NULL)
        mod_len = BN_num_bits(x->n);

    if (!BIO_indent(bp, off, 128))
        goto err;

    if (priv && x->d) {
        if (BIO_printf(bp, "Private-Key: (%d bit)\n", mod_len)
            <= 0)
            goto err;
        str = "modulus:";
        s = "publicExponent:";
    } else {
        if (BIO_printf(bp, "Public-Key: (%d bit)\n", mod_len)
            <= 0)
            goto err;
        str = "Modulus:";
        s = "Exponent:";
    }
    if (!ASN1_bn_print(bp, str, x->n, m, off))
        goto err;
    if (!ASN1_bn_print(bp, s, x->e, m, off))
        goto err;
    if (priv) {
        if (!ASN1_bn_print(bp, "privateExponent:", x->d, m, off))
            goto err;
        if (!ASN1_bn_print(bp, "prime1:", x->p, m, off))
            goto err;
        if (!ASN1_bn_print(bp, "prime2:", x->q, m, off))
            goto err;
        if (!ASN1_bn_print(bp, "exponent1:", x->dmp1, m, off))
            goto err;
        if (!ASN1_bn_print(bp, "exponent2:", x->dmq1, m, off))
            goto err;
        if (!ASN1_bn_print(bp, "coefficient:", x->iqmp, m, off))
            goto err;
    }
    ret = 1;
err:
    free(m);
    return (ret);
}

static int rsa_pub_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                         ASN1_PCTX *ctx)
{
    return do_rsa_print(bp, pkey->pkey.rsa, indent, 0);
}

static int rsa_priv_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                          ASN1_PCTX *ctx)
{
    return do_rsa_print(bp, pkey->pkey.rsa, indent, 1);
}

/* Given an MGF1 Algorithm ID decode to an Algorithm Identifier */
static X509_ALGOR *rsa_mgf1_decode(X509_ALGOR *alg)
{
    const uint8_t *p;
    int plen;
    if (alg == NULL || alg->parameter == NULL)
        return NULL;
    if (OBJ_obj2nid(alg->algorithm) != NID_mgf1)
        return NULL;
    if (alg->parameter->type != V_ASN1_SEQUENCE)
        return NULL;

    p = alg->parameter->value.sequence->data;
    plen = alg->parameter->value.sequence->length;
    return d2i_X509_ALGOR(NULL, &p, plen);
}

static RSA_PSS_PARAMS *rsa_pss_decode(const X509_ALGOR *alg, X509_ALGOR **pmaskHash)
{
    RSA_PSS_PARAMS *pss;

    *pmaskHash = NULL;

    pss = ASN1_TYPE_unpack_sequence(ASN1_ITEM_rptr(RSA_PSS_PARAMS), alg->parameter);

    if (pss == NULL)
        return NULL;

    *pmaskHash = rsa_mgf1_decode(pss->maskGenAlgorithm);

    return pss;
}

static int rsa_pss_param_print(BIO *bp, RSA_PSS_PARAMS *pss,
                               X509_ALGOR *maskHash, int indent)
{
    int rv = 0;
    if (!pss) {
        if (BIO_puts(bp, " (INVALID PSS PARAMETERS)\n") <= 0)
            return 0;
        return 1;
    }
    if (BIO_puts(bp, "\n") <= 0)
        goto err;
    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_puts(bp, "Hash Algorithm: ") <= 0)
        goto err;

    if (pss->hashAlgorithm) {
        if (i2a_ASN1_OBJECT(bp, pss->hashAlgorithm->algorithm) <= 0)
            goto err;
    } else if (BIO_puts(bp, "sha1 (default)") <= 0)
        goto err;

    if (BIO_puts(bp, "\n") <= 0)
        goto err;

    if (!BIO_indent(bp, indent, 128))
        goto err;

    if (BIO_puts(bp, "Mask Algorithm: ") <= 0)
        goto err;
    if (pss->maskGenAlgorithm) {
        if (i2a_ASN1_OBJECT(bp, pss->maskGenAlgorithm->algorithm) <= 0)
            goto err;
        if (BIO_puts(bp, " with ") <= 0)
            goto err;
        if (maskHash) {
            if (i2a_ASN1_OBJECT(bp, maskHash->algorithm) <= 0)
                goto err;
        } else if (BIO_puts(bp, "INVALID") <= 0)
            goto err;
    } else if (BIO_puts(bp, "mgf1 with sha1 (default)") <= 0)
        goto err;
    BIO_puts(bp, "\n");

    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_puts(bp, "Salt Length: 0x") <= 0)
        goto err;
    if (pss->saltLength) {
        if (i2a_ASN1_INTEGER(bp, pss->saltLength) <= 0)
            goto err;
    } else if (BIO_puts(bp, "14 (default)") <= 0)
        goto err;
    BIO_puts(bp, "\n");

    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_puts(bp, "Trailer Field: 0x") <= 0)
        goto err;
    if (pss->trailerField) {
        if (i2a_ASN1_INTEGER(bp, pss->trailerField) <= 0)
            goto err;
    } else if (BIO_puts(bp, "BC (default)") <= 0)
        goto err;
    BIO_puts(bp, "\n");

    rv = 1;

err:
    return rv;
}

static int rsa_sig_print(BIO *bp, const X509_ALGOR *sigalg,
                         const ASN1_STRING *sig,
                         int indent, ASN1_PCTX *pctx)
{
    if (OBJ_obj2nid(sigalg->algorithm) == NID_rsassaPss) {
        int rv;
        RSA_PSS_PARAMS *pss;
        X509_ALGOR *maskHash;
        pss = rsa_pss_decode(sigalg, &maskHash);
        rv = rsa_pss_param_print(bp, pss, maskHash, indent);
        if (pss)
            RSA_PSS_PARAMS_free(pss);
        if (maskHash)
            X509_ALGOR_free(maskHash);
        if (!rv)
            return 0;
    } else if (!sig && BIO_puts(bp, "\n") <= 0)
        return 0;
    if (sig)
        return X509_signature_dump(bp, sig, indent);
    return 1;
}

static int rsa_pkey_ctrl(EVP_PKEY *pkey, int op, long arg1, void *arg2)
{
    X509_ALGOR *alg = NULL;
    switch (op) {

        case ASN1_PKEY_CTRL_PKCS7_SIGN:
            if (arg1 == 0)
                PKCS7_SIGNER_INFO_get0_algs(arg2, NULL, NULL, &alg);
            break;

        case ASN1_PKEY_CTRL_PKCS7_ENCRYPT:
            if (arg1 == 0)
                PKCS7_RECIP_INFO_get0_alg(arg2, &alg);
            break;

        case ASN1_PKEY_CTRL_DEFAULT_MD_NID:
            *(int *)arg2 = NID_sha256;
            return 1;

        default:
            return -2;
    }

    if (alg)
        X509_ALGOR_set0(alg, OBJ_nid2obj(NID_rsaEncryption),
                        V_ASN1_NULL, 0);

    return 1;
}

/* allocate and set algorithm ID from EVP_MD, default SHA1 */
static int rsa_md_to_algor(X509_ALGOR **palg, const EVP_MD *md)
{
    if (EVP_MD_type(md) == NID_sha1)
        return 1;
    *palg = X509_ALGOR_new();
    if (*palg == NULL)
        return 0;
    X509_ALGOR_set_md(*palg, md);
    return 1;
}

/* Allocate and set MGF1 algorithm ID from EVP_MD */
static int rsa_md_to_mgf1(X509_ALGOR **palg, const EVP_MD *mgf1md)
{
    X509_ALGOR *algtmp = NULL;
    ASN1_STRING *stmp = NULL;
    *palg = NULL;
    if (EVP_MD_type(mgf1md) == NID_sha1)
        return 1;
    /* need to embed algorithm ID inside another */
    if (!rsa_md_to_algor(&algtmp, mgf1md))
        goto err;
    if (!ASN1_item_pack(algtmp, ASN1_ITEM_rptr(X509_ALGOR), &stmp))
        goto err;
    *palg = X509_ALGOR_new();
    if (*palg == NULL)
        goto err;
    X509_ALGOR_set0(*palg, OBJ_nid2obj(NID_mgf1), V_ASN1_SEQUENCE, stmp);
    stmp = NULL;
err:
    ASN1_STRING_free(stmp);
    X509_ALGOR_free(algtmp);
    if (*palg != NULL)
        return 1;
    return 0;
}

/* convert algorithm ID to EVP_MD, default SHA1 */
static const EVP_MD *rsa_algor_to_md(X509_ALGOR *alg)
{
    const EVP_MD *md;
    if (!alg)
        return EVP_sha1();
    md = EVP_get_digestbyobj(alg->algorithm);
    if (md == NULL)
        RSAerr(RSA_F_RSA_ALGOR_TO_MD, RSA_R_UNKNOWN_DIGEST);
    return md;
}
/* convert MGF1 algorithm ID to EVP_MD, default SHA1 */
static const EVP_MD *rsa_mgf1_to_md(X509_ALGOR *alg, X509_ALGOR *maskHash)
{
    const EVP_MD *md;
    if (alg == NULL)
        return EVP_sha1();
    /* Check mask and lookup mask hash algorithm */
    if (OBJ_obj2nid(alg->algorithm) != NID_mgf1) {
        RSAerr(RSA_F_RSA_MGF1_TO_MD, RSA_R_UNSUPPORTED_MASK_ALGORITHM);
        return NULL;
    }
    if (maskHash == NULL) {
        RSAerr(RSA_F_RSA_MGF1_TO_MD, RSA_R_UNSUPPORTED_MASK_PARAMETER);
        return NULL;
    }
    md = EVP_get_digestbyobj(maskHash->algorithm);
    if (md == NULL) {
        RSAerr(RSA_F_RSA_MGF1_TO_MD, RSA_R_UNKNOWN_MASK_DIGEST);
        return NULL;
    }
    return md;
}

/*
 * Convert EVP_PKEY_CTX is PSS mode into corresponding algorithm parameter,
 * suitable for setting an AlgorithmIdentifier.
 */

static ASN1_STRING *rsa_ctx_to_pss(EVP_PKEY_CTX *pkctx)
{
    const EVP_MD *sigmd, *mgf1md;
    RSA_PSS_PARAMS *pss = NULL;
    ASN1_STRING *os = NULL;
    EVP_PKEY *pk = EVP_PKEY_CTX_get0_pkey(pkctx);
    int saltlen, rv = 0;
    if (EVP_PKEY_CTX_get_signature_md(pkctx, &sigmd) <= 0)
        goto err;
    if (EVP_PKEY_CTX_get_rsa_mgf1_md(pkctx, &mgf1md) <= 0)
        goto err;
    if (!EVP_PKEY_CTX_get_rsa_pss_saltlen(pkctx, &saltlen))
        goto err;
    if (saltlen == -1)
        saltlen = EVP_MD_size(sigmd);
    else if (saltlen == -2) {
        saltlen = EVP_PKEY_size(pk) - EVP_MD_size(sigmd) - 2;
        if (((EVP_PKEY_bits(pk) - 1) & 0x7) == 0)
            saltlen--;
    }
    pss = RSA_PSS_PARAMS_new();
    if (pss == NULL)
        goto err;
    if (saltlen != 20) {
        pss->saltLength = ASN1_INTEGER_new();
        if (pss->saltLength == NULL)
            goto err;
        if (!ASN1_INTEGER_set(pss->saltLength, saltlen))
            goto err;
    }
    if (!rsa_md_to_algor(&pss->hashAlgorithm, sigmd))
        goto err;
    if (!rsa_md_to_mgf1(&pss->maskGenAlgorithm, mgf1md))
        goto err;
    /* Finally create string with pss parameter encoding. */
    if (!ASN1_item_pack(pss, ASN1_ITEM_rptr(RSA_PSS_PARAMS), &os))
        goto err;
    rv = 1;
err:
    RSA_PSS_PARAMS_free(pss);
    if (rv)
        return os;
    ASN1_STRING_free(os);
    return NULL;
}

/* 
 * From PSS AlgorithmIdentifier set public key parameters. If pkey
 * isn't NULL then the EVP_MD_CTX is setup and initalised. If it
 * is NULL parameters are passed to pkctx instead.
 */

static int rsa_pss_to_ctx(EVP_MD_CTX *ctx, EVP_PKEY_CTX *pkctx,
                          X509_ALGOR *sigalg, EVP_PKEY *pkey)
{
    int rv = -1;
    int saltlen;
    const EVP_MD *mgf1md = NULL, *md = NULL;
    RSA_PSS_PARAMS *pss;
    X509_ALGOR *maskHash;
    /* Sanity check: make sure it is PSS */
    if (OBJ_obj2nid(sigalg->algorithm) != NID_rsassaPss) {
        RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_UNSUPPORTED_SIGNATURE_TYPE);
        return -1;
    }
    /* Decode PSS parameters */
    pss = rsa_pss_decode(sigalg, &maskHash);

    if (pss == NULL) {
        RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_INVALID_PSS_PARAMETERS);
        goto err;
    }
    mgf1md = rsa_mgf1_to_md(pss->maskGenAlgorithm, maskHash);
    if (mgf1md == NULL)
        goto err;
    md = rsa_algor_to_md(pss->hashAlgorithm);
    if (md == NULL)
        goto err;

    if (pss->hashAlgorithm) {
        md = EVP_get_digestbyobj(pss->hashAlgorithm->algorithm);
        if (md == NULL) {
            RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_UNKNOWN_PSS_DIGEST);
            goto err;
        }
    } else
        md = EVP_sha1();

    if (pss->saltLength) {
        saltlen = ASN1_INTEGER_get(pss->saltLength);

        /* Could perform more salt length sanity checks but the main
         * RSA routines will trap other invalid values anyway.
         */
        if (saltlen < 0) {
            RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_INVALID_SALT_LENGTH);
            goto err;
        }
    } else
        saltlen = 20;

    /* low-level routines support only trailer field 0xbc (value 1)
     * and PKCS#1 says we should reject any other value anyway.
     */
    if (pss->trailerField && ASN1_INTEGER_get(pss->trailerField) != 1) {
        RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_INVALID_TRAILER);
        goto err;
    }

    /* We have all parameters now set up context */

    if (pkey) {
        if (!EVP_DigestVerifyInit(ctx, &pkctx, md, NULL, pkey))
            goto err;
    } else {
        const EVP_MD *checkmd;
        if (EVP_PKEY_CTX_get_signature_md(pkctx, &checkmd) <= 0)
            goto err;
        if (EVP_MD_type(md) != EVP_MD_type(checkmd)) {
            RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_DIGEST_DOES_NOT_MATCH);
            goto err;
        }
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pkctx, RSA_PKCS1_PSS_PADDING) <= 0)
        goto err;

    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pkctx, saltlen) <= 0)
        goto err;

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(pkctx, mgf1md) <= 0)
        goto err;
    /* Carry on */
    rv = 1;

err:
    RSA_PSS_PARAMS_free(pss);
    X509_ALGOR_free(maskHash);
    return rv;
}

/*
 * Customised RSA item verification routine. This is called
 * when a signature is encountered requiring special handling. We
 * currently only handle PSS.
 */


static int rsa_item_verify(EVP_MD_CTX *ctx, const ASN1_ITEM *it, void *asn,
                           X509_ALGOR *sigalg, ASN1_BIT_STRING *sig,
                           EVP_PKEY *pkey)
{
    /* Sanity check: make sure it is PSS */
    if (OBJ_obj2nid(sigalg->algorithm) != NID_rsassaPss) {
        RSAerr(RSA_F_RSA_ITEM_VERIFY, RSA_R_UNSUPPORTED_SIGNATURE_TYPE);
        return -1;
    }
    if (rsa_pss_to_ctx(ctx, NULL, sigalg, pkey) > 0)
        return 2; /* Carry on */
    return -1;
}

static int rsa_item_sign(EVP_MD_CTX *ctx, const ASN1_ITEM *it, void *asn,
                         X509_ALGOR *alg1, X509_ALGOR *alg2,
                         ASN1_BIT_STRING *sig)
{
    int pad_mode;
    EVP_PKEY_CTX *pkctx = ctx->pctx;
    if (EVP_PKEY_CTX_get_rsa_padding(pkctx, &pad_mode) <= 0)
        return 0;
    if (pad_mode == RSA_PKCS1_PADDING)
        return 2;
    if (pad_mode == RSA_PKCS1_PSS_PADDING) {
        ASN1_STRING *os1 = NULL;
        os1 = rsa_ctx_to_pss(pkctx);
        if (os1 == NULL)
            return 0;
        /* Duplicate parameters if we have to */
        if (alg2) {
            ASN1_STRING *os2 = ASN1_STRING_dup(os1);
            if (os2 == NULL) {
                ASN1_STRING_free(os1);
                return 0;
            }
            X509_ALGOR_set0(alg2, OBJ_nid2obj(NID_rsassaPss),
                            V_ASN1_SEQUENCE, os2);
        }
        X509_ALGOR_set0(alg1, OBJ_nid2obj(NID_rsassaPss),
                        V_ASN1_SEQUENCE, os1);
        return 3;
    }
    return 2;
}

const EVP_PKEY_ASN1_METHOD rsa_asn1_meths[] = {
    { .pkey_id = EVP_PKEY_RSA,
      .pkey_base_id = EVP_PKEY_RSA,
      .pkey_flags = ASN1_PKEY_SIGPARAM_NULL,

      .pem_str = (char *)"RSA",
      .info = (char *)"OpenSSL RSA method",

      .pub_decode = rsa_pub_decode,
      .pub_encode = rsa_pub_encode,
      .pub_cmp = rsa_pub_cmp,
      .pub_print = rsa_pub_print,

      .priv_decode = rsa_priv_decode,
      .priv_encode = rsa_priv_encode,
      .priv_print = rsa_priv_print,

      .pkey_size = int_rsa_size,
      .pkey_bits = rsa_bits,

      .sig_print = rsa_sig_print,

      .pkey_free = int_rsa_free,
      .pkey_ctrl = rsa_pkey_ctrl,
      .old_priv_decode = old_rsa_priv_decode,
      .old_priv_encode = old_rsa_priv_encode,
      .item_verify = rsa_item_verify,
      .item_sign = rsa_item_sign },

    { .pkey_id = EVP_PKEY_RSA2,
      .pkey_base_id = EVP_PKEY_RSA,
      .pkey_flags = ASN1_PKEY_ALIAS }
};

