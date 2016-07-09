/*
 * Copyright 1999-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/asn1t.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static void *v2i_EXTENDED_KEY_USAGE(const X509V3_EXT_METHOD *method,
                                    X509V3_CTX *ctx,
                                    STACK_OF(CONF_VALUE) *nval);
static STACK_OF(CONF_VALUE) *i2v_EXTENDED_KEY_USAGE(const X509V3_EXT_METHOD *method,
                                                     void *eku, STACK_OF(CONF_VALUE) *extlist);

const X509V3_EXT_METHOD v3_ext_ku = {
    NID_ext_key_usage, 0,
    ASN1_ITEM_ref(EXTENDED_KEY_USAGE),
    0, 0, 0, 0,
    0, 0,
    i2v_EXTENDED_KEY_USAGE,
    v2i_EXTENDED_KEY_USAGE,
    0, 0,
    NULL
};

/* NB OCSP acceptable responses also is a SEQUENCE OF OBJECT */
const X509V3_EXT_METHOD v3_ocsp_accresp = {
    NID_id_pkix_OCSP_acceptableResponses, 0,
    ASN1_ITEM_ref(EXTENDED_KEY_USAGE),
    0, 0, 0, 0,
    0, 0,
    i2v_EXTENDED_KEY_USAGE,
    v2i_EXTENDED_KEY_USAGE,
    0, 0,
    NULL
};

ASN1_ITEM_TEMPLATE(EXTENDED_KEY_USAGE) = ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_SEQUENCE_OF, 0, EXTENDED_KEY_USAGE, ASN1_OBJECT)
ASN1_ITEM_TEMPLATE_END(EXTENDED_KEY_USAGE)

EXTENDED_KEY_USAGE *d2i_EXTENDED_KEY_USAGE(EXTENDED_KEY_USAGE **a, const uint8_t **in, long len)
{
    return (EXTENDED_KEY_USAGE *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(EXTENDED_KEY_USAGE));
}

int i2d_EXTENDED_KEY_USAGE(EXTENDED_KEY_USAGE *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(EXTENDED_KEY_USAGE));
}

EXTENDED_KEY_USAGE *EXTENDED_KEY_USAGE_new(void)
{
    return (EXTENDED_KEY_USAGE *)ASN1_item_new(ASN1_ITEM_rptr(EXTENDED_KEY_USAGE));
}

void EXTENDED_KEY_USAGE_free(EXTENDED_KEY_USAGE *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(EXTENDED_KEY_USAGE));
}

static STACK_OF(CONF_VALUE) *i2v_EXTENDED_KEY_USAGE(const X509V3_EXT_METHOD *method, void *a,
                                                    STACK_OF(CONF_VALUE) *ext_list)
{
    EXTENDED_KEY_USAGE *eku = a;
    int i;
    ASN1_OBJECT *obj;
    char obj_tmp[80];
    for (i = 0; i < sk_ASN1_OBJECT_num(eku); i++) {
        obj = sk_ASN1_OBJECT_value(eku, i);
        i2t_ASN1_OBJECT(obj_tmp, 80, obj);
        X509V3_add_value(NULL, obj_tmp, &ext_list);
    }
    return ext_list;
}

static void *v2i_EXTENDED_KEY_USAGE(const X509V3_EXT_METHOD *method,
                                    X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *nval)
{
    EXTENDED_KEY_USAGE *extku;
    char *extval;
    ASN1_OBJECT *objtmp;
    CONF_VALUE *val;
    int i;

    if (!(extku = sk_ASN1_OBJECT_new_null())) {
        X509V3err(X509V3_F_V2I_EXTENDED_KEY_USAGE, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    for (i = 0; i < sk_CONF_VALUE_num(nval); i++) {
        val = sk_CONF_VALUE_value(nval, i);
        if (val->value)
            extval = val->value;
        else
            extval = val->name;
        if (!(objtmp = OBJ_txt2obj(extval, 0))) {
            sk_ASN1_OBJECT_pop_free(extku, ASN1_OBJECT_free);
            X509V3err(X509V3_F_V2I_EXTENDED_KEY_USAGE, X509V3_R_INVALID_OBJECT_IDENTIFIER);
            X509V3_conf_err(val);
            return NULL;
        }
        sk_ASN1_OBJECT_push(extku, objtmp);
    }
    return extku;
}
