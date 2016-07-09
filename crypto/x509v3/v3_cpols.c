/*
 * Copyright 1999-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#include "pcy_int.h"

/* Certificate policies extension support: this one is a bit complex... */

static int i2r_certpol(X509V3_EXT_METHOD *method, STACK_OF(POLICYINFO) *pol, BIO * out, int indent);
static STACK_OF(POLICYINFO) *r2i_certpol(X509V3_EXT_METHOD *method, X509V3_CTX *ctx, char *value);
static void print_qualifiers(BIO *out, STACK_OF(POLICYQUALINFO) *quals, int indent);
static void print_notice(BIO *out, USERNOTICE *notice, int indent);
static POLICYINFO *policy_section(X509V3_CTX *ctx,
                                  STACK_OF(CONF_VALUE) *polstrs, int ia5org);
static POLICYQUALINFO *notice_section(X509V3_CTX *ctx,
                                      STACK_OF(CONF_VALUE) *unot, int ia5org);
static int nref_nos(STACK_OF(ASN1_INTEGER) *nnums, STACK_OF(CONF_VALUE) *nos);

const X509V3_EXT_METHOD v3_cpols = {
    NID_certificate_policies, 0, ASN1_ITEM_ref(CERTIFICATEPOLICIES),
    0, 0, 0, 0,
    0, 0,
    0, 0,
    (X509V3_EXT_I2R)i2r_certpol,
    (X509V3_EXT_R2I)r2i_certpol,
    NULL
};

ASN1_ITEM_TEMPLATE(CERTIFICATEPOLICIES) = ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_SEQUENCE_OF, 0, CERTIFICATEPOLICIES, POLICYINFO)
    ASN1_ITEM_TEMPLATE_END(CERTIFICATEPOLICIES)

CERTIFICATEPOLICIES *d2i_CERTIFICATEPOLICIES(CERTIFICATEPOLICIES **a, const uint8_t **in, long len)
{
    return (CERTIFICATEPOLICIES *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(CERTIFICATEPOLICIES));
}

int i2d_CERTIFICATEPOLICIES(CERTIFICATEPOLICIES *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(CERTIFICATEPOLICIES));
}

CERTIFICATEPOLICIES *CERTIFICATEPOLICIES_new(void)
{
    return (CERTIFICATEPOLICIES *)ASN1_item_new(ASN1_ITEM_rptr(CERTIFICATEPOLICIES));
}

void CERTIFICATEPOLICIES_free(CERTIFICATEPOLICIES *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(CERTIFICATEPOLICIES));
}

ASN1_SEQUENCE(POLICYINFO) = {
    ASN1_SIMPLE(POLICYINFO, policyid, ASN1_OBJECT),
    ASN1_SEQUENCE_OF_OPT(POLICYINFO, qualifiers, POLICYQUALINFO)
} ASN1_SEQUENCE_END(POLICYINFO)

POLICYINFO *d2i_POLICYINFO(POLICYINFO **a, const uint8_t **in, long len)
{
    return (POLICYINFO *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(POLICYINFO));
}

int i2d_POLICYINFO(POLICYINFO *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(POLICYINFO));
}

POLICYINFO *POLICYINFO_new(void)
{
    return (POLICYINFO *)ASN1_item_new(ASN1_ITEM_rptr(POLICYINFO));
}

void POLICYINFO_free(POLICYINFO *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(POLICYINFO));
}

ASN1_ADB_TEMPLATE(policydefault) = ASN1_SIMPLE(POLICYQUALINFO, d.other, ASN1_ANY);

ASN1_ADB(POLICYQUALINFO) = {
    ADB_ENTRY(NID_id_qt_cps, ASN1_SIMPLE(POLICYQUALINFO, d.cpsuri, ASN1_IA5STRING)),
    ADB_ENTRY(NID_id_qt_unotice, ASN1_SIMPLE(POLICYQUALINFO, d.usernotice, USERNOTICE))
} ASN1_ADB_END(POLICYQUALINFO, 0, pqualid, 0, &policydefault_tt, NULL);

ASN1_SEQUENCE(POLICYQUALINFO) = {
    ASN1_SIMPLE(POLICYQUALINFO, pqualid, ASN1_OBJECT),
    ASN1_ADB_OBJECT(POLICYQUALINFO)
} ASN1_SEQUENCE_END(POLICYQUALINFO)

POLICYQUALINFO *d2i_POLICYQUALINFO(POLICYQUALINFO **a, const uint8_t **in, long len)
{
    return (POLICYQUALINFO *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(POLICYQUALINFO));
}

int i2d_POLICYQUALINFO(POLICYQUALINFO *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(POLICYQUALINFO));
}

POLICYQUALINFO *POLICYQUALINFO_new(void)
{
    return (POLICYQUALINFO *)ASN1_item_new(ASN1_ITEM_rptr(POLICYQUALINFO));
}

void POLICYQUALINFO_free(POLICYQUALINFO *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(POLICYQUALINFO));
}

ASN1_SEQUENCE(USERNOTICE) = {
    ASN1_OPT(USERNOTICE, noticeref, NOTICEREF),
    ASN1_OPT(USERNOTICE, exptext, DISPLAYTEXT)
} ASN1_SEQUENCE_END(USERNOTICE)

USERNOTICE *d2i_USERNOTICE(USERNOTICE **a, const uint8_t **in, long len)
{
    return (USERNOTICE *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(USERNOTICE));
}

int i2d_USERNOTICE(USERNOTICE *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(USERNOTICE));
}

USERNOTICE *USERNOTICE_new(void)
{
    return (USERNOTICE *)ASN1_item_new(ASN1_ITEM_rptr(USERNOTICE));
}

void USERNOTICE_free(USERNOTICE *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(USERNOTICE));
}

ASN1_SEQUENCE(NOTICEREF) = {
    ASN1_SIMPLE(NOTICEREF, organization, DISPLAYTEXT),
    ASN1_SEQUENCE_OF(NOTICEREF, noticenos, ASN1_INTEGER)
} ASN1_SEQUENCE_END(NOTICEREF)

NOTICEREF *d2i_NOTICEREF(NOTICEREF **a, const uint8_t **in, long len)
{
    return (NOTICEREF *)ASN1_item_d2i((ASN1_VALUE **)a, in, len, ASN1_ITEM_rptr(NOTICEREF));
}

int i2d_NOTICEREF(NOTICEREF *a, uint8_t **out)
{
    return ASN1_item_i2d((ASN1_VALUE *)a, out, ASN1_ITEM_rptr(NOTICEREF));
}

NOTICEREF *NOTICEREF_new(void)
{
    return (NOTICEREF *)ASN1_item_new(ASN1_ITEM_rptr(NOTICEREF));
}

void NOTICEREF_free(NOTICEREF *a)
{
    ASN1_item_free((ASN1_VALUE *)a, ASN1_ITEM_rptr(NOTICEREF));
}

static STACK_OF(POLICYINFO) *r2i_certpol(X509V3_EXT_METHOD * method, X509V3_CTX * ctx, char *value)
{
    STACK_OF(POLICYINFO) *pols = NULL;
    char *pstr;
    POLICYINFO *pol;
    ASN1_OBJECT *pobj;
    STACK_OF(CONF_VALUE) *vals;
    CONF_VALUE *cnf;
    int i, ia5org;
    pols = sk_POLICYINFO_new_null();
    if (pols == NULL) {
        X509V3err(X509V3_F_R2I_CERTPOL, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    vals = X509V3_parse_list(value);
    if (vals == NULL) {
        X509V3err(X509V3_F_R2I_CERTPOL, ERR_R_X509V3_LIB);
        goto err;
    }
    ia5org = 0;
    for (i = 0; i < sk_CONF_VALUE_num(vals); i++) {
        cnf = sk_CONF_VALUE_value(vals, i);
        if (cnf->value || !cnf->name) {
            X509V3err(X509V3_F_R2I_CERTPOL, X509V3_R_INVALID_POLICY_IDENTIFIER);
            X509V3_conf_err(cnf);
            goto err;
        }
        pstr = cnf->name;
        if (strcmp(pstr, "ia5org") == 0) {
            ia5org = 1;
            continue;
        } else if (*pstr == '@') {
            STACK_OF(CONF_VALUE) *polsect;
            polsect = X509V3_get_section(ctx, pstr + 1);
            if (!polsect) {
                X509V3err(X509V3_F_R2I_CERTPOL, X509V3_R_INVALID_SECTION);

                X509V3_conf_err(cnf);
                goto err;
            }
            pol = policy_section(ctx, polsect, ia5org);
            X509V3_section_free(ctx, polsect);
            if (!pol)
                goto err;
        } else {
            if (!(pobj = OBJ_txt2obj(cnf->name, 0))) {
                X509V3err(X509V3_F_R2I_CERTPOL, X509V3_R_INVALID_OBJECT_IDENTIFIER);
                X509V3_conf_err(cnf);
                goto err;
            }
            pol = POLICYINFO_new();
            if (pol == NULL) {
                X509V3err(X509V3_F_R2I_CERTPOL, ERR_R_MALLOC_FAILURE);
                goto err;
            }
            pol->policyid = pobj;
        }
        if (!sk_POLICYINFO_push(pols, pol)) {
            POLICYINFO_free(pol);
            X509V3err(X509V3_F_R2I_CERTPOL, ERR_R_MALLOC_FAILURE);
            goto err;
        }
    }
    sk_CONF_VALUE_pop_free(vals, X509V3_conf_free);
    return pols;
err:
    sk_CONF_VALUE_pop_free(vals, X509V3_conf_free);
    sk_POLICYINFO_pop_free(pols, POLICYINFO_free);
    return NULL;
}

static POLICYINFO *policy_section(X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *polstrs, int ia5org)
{
    int i;
    CONF_VALUE *cnf;
    POLICYINFO *pol;
    POLICYQUALINFO *qual;
    if (!(pol = POLICYINFO_new()))
        goto merr;
    for (i = 0; i < sk_CONF_VALUE_num(polstrs); i++) {
        cnf = sk_CONF_VALUE_value(polstrs, i);
        if (strcmp(cnf->name, "policyIdentifier") == 0) {
            ASN1_OBJECT *pobj;
            if (!(pobj = OBJ_txt2obj(cnf->value, 0))) {
                X509V3err(X509V3_F_POLICY_SECTION, X509V3_R_INVALID_OBJECT_IDENTIFIER);
                X509V3_conf_err(cnf);
                goto err;
            }
            pol->policyid = pobj;

        } else if (!name_cmp(cnf->name, "CPS")) {
            if (!pol->qualifiers)
                pol->qualifiers = sk_POLICYQUALINFO_new_null();
            if (!(qual = POLICYQUALINFO_new()))
                goto merr;
            if (!sk_POLICYQUALINFO_push(pol->qualifiers, qual))
                goto merr;
            if (!(qual->pqualid = OBJ_nid2obj(NID_id_qt_cps))) {
                X509V3err(X509V3_F_POLICY_SECTION, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            qual->d.cpsuri = ASN1_IA5STRING_new();
            if (qual->d.cpsuri == NULL)
                goto merr;
            if (!ASN1_STRING_set(qual->d.cpsuri, cnf->value,
                                 strlen(cnf->value)))
                goto merr;
        } else if (!name_cmp(cnf->name, "userNotice")) {
            STACK_OF(CONF_VALUE) *unot;
            if (*cnf->value != '@') {
                X509V3err(X509V3_F_POLICY_SECTION, X509V3_R_EXPECTED_A_SECTION_NAME);
                X509V3_conf_err(cnf);
                goto err;
            }
            unot = X509V3_get_section(ctx, cnf->value + 1);
            if (!unot) {
                X509V3err(X509V3_F_POLICY_SECTION, X509V3_R_INVALID_SECTION);

                X509V3_conf_err(cnf);
                goto err;
            }
            qual = notice_section(ctx, unot, ia5org);
            X509V3_section_free(ctx, unot);
            if (!qual)
                goto err;
            if (!pol->qualifiers)
                pol->qualifiers = sk_POLICYQUALINFO_new_null();
            if (!sk_POLICYQUALINFO_push(pol->qualifiers, qual))
                goto merr;
        } else {
            X509V3err(X509V3_F_POLICY_SECTION, X509V3_R_INVALID_OPTION);

            X509V3_conf_err(cnf);
            goto err;
        }
    }
    if (!pol->policyid) {
        X509V3err(X509V3_F_POLICY_SECTION, X509V3_R_NO_POLICY_IDENTIFIER);
        goto err;
    }

    return pol;

merr:
    X509V3err(X509V3_F_POLICY_SECTION, ERR_R_MALLOC_FAILURE);

err:
    POLICYINFO_free(pol);
    return NULL;
}

static POLICYQUALINFO *notice_section(X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *unot, int ia5org)
{
    int i, ret;
    CONF_VALUE *cnf;
    USERNOTICE * not;
    POLICYQUALINFO *qual;
    if (!(qual = POLICYQUALINFO_new()))
        goto merr;
    if (!(qual->pqualid = OBJ_nid2obj(NID_id_qt_unotice))) {
        X509V3err(X509V3_F_NOTICE_SECTION, ERR_R_INTERNAL_ERROR);
        goto err;
    }    
    if (!(not = USERNOTICE_new()))
        goto merr;
    qual->d.usernotice = not;
    for (i = 0; i < sk_CONF_VALUE_num(unot); i++) {
        cnf = sk_CONF_VALUE_value(unot, i);
        if (strcmp(cnf->name, "explicitText") == 0) {
            not->exptext = ASN1_VISIBLESTRING_new();
            if (not->exptext == NULL)
                goto merr;
            if (!ASN1_STRING_set(not->exptext, cnf->value,
                                 strlen(cnf->value)))
                goto merr;
        } else if (strcmp(cnf->name, "organization") == 0) {
            NOTICEREF *nref;
            if (!not->noticeref) {
                if (!(nref = NOTICEREF_new()))
                    goto merr;
                not->noticeref = nref;
            } else
                nref = not->noticeref;
            if (ia5org)
                nref->organization->type = V_ASN1_IA5STRING;
            else
                nref->organization->type = V_ASN1_VISIBLESTRING;
            if (!ASN1_STRING_set(nref->organization, cnf->value,
                                 strlen(cnf->value)))
                goto merr;
        } else if (strcmp(cnf->name, "noticeNumbers") == 0) {
            NOTICEREF *nref;
            STACK_OF(CONF_VALUE) *nos;
            if (!not->noticeref) {
                if (!(nref = NOTICEREF_new()))
                    goto merr;
                not->noticeref = nref;
            } else
                nref = not->noticeref;
            nos = X509V3_parse_list(cnf->value);
            if (!nos || !sk_CONF_VALUE_num(nos)) {
                X509V3err(X509V3_F_NOTICE_SECTION, X509V3_R_INVALID_NUMBERS);
                X509V3_conf_err(cnf);
                sk_CONF_VALUE_pop_free(nos, X509V3_conf_free);
                goto err;
            }
            ret = nref_nos(nref->noticenos, nos);
            sk_CONF_VALUE_pop_free(nos, X509V3_conf_free);
            if (!ret)
                goto err;
        } else {
            X509V3err(X509V3_F_NOTICE_SECTION, X509V3_R_INVALID_OPTION);
            X509V3_conf_err(cnf);
            goto err;
        }
    }

    if (not->noticeref && (!not->noticeref->noticenos || !not->noticeref->organization)) {
        X509V3err(X509V3_F_NOTICE_SECTION, X509V3_R_NEED_ORGANIZATION_AND_NUMBERS);
        goto err;
    }

    return qual;

merr:
    X509V3err(X509V3_F_NOTICE_SECTION, ERR_R_MALLOC_FAILURE);

err:
    POLICYQUALINFO_free(qual);
    return NULL;
}

static int nref_nos(STACK_OF(ASN1_INTEGER) *nnums, STACK_OF(CONF_VALUE) *nos)
{
    CONF_VALUE *cnf;
    ASN1_INTEGER *aint;

    int i;

    for (i = 0; i < sk_CONF_VALUE_num(nos); i++) {
        cnf = sk_CONF_VALUE_value(nos, i);
        if (!(aint = s2i_ASN1_INTEGER(NULL, cnf->name))) {
            X509V3err(X509V3_F_NREF_NOS, X509V3_R_INVALID_NUMBER);
            goto err;
        }
        if (!sk_ASN1_INTEGER_push(nnums, aint))
            goto merr;
    }
    return 1;

merr:
    X509V3err(X509V3_F_NREF_NOS, ERR_R_MALLOC_FAILURE);

err:
    sk_ASN1_INTEGER_pop_free(nnums, ASN1_STRING_free);
    return 0;
}

static int i2r_certpol(X509V3_EXT_METHOD *method, STACK_OF(POLICYINFO) *pol,
                       BIO * out, int indent)
{
    int i;
    POLICYINFO *pinfo;
    /* First print out the policy OIDs */
    for (i = 0; i < sk_POLICYINFO_num(pol); i++) {
        pinfo = sk_POLICYINFO_value(pol, i);
        BIO_printf(out, "%*sPolicy: ", indent, "");
        i2a_ASN1_OBJECT(out, pinfo->policyid);
        BIO_puts(out, "\n");
        if (pinfo->qualifiers)
            print_qualifiers(out, pinfo->qualifiers, indent + 2);
    }
    return 1;
}

static void print_qualifiers(BIO *out, STACK_OF(POLICYQUALINFO) *quals,
                             int indent)
{
    POLICYQUALINFO *qualinfo;
    int i;
    for (i = 0; i < sk_POLICYQUALINFO_num(quals); i++) {
        qualinfo = sk_POLICYQUALINFO_value(quals, i);
        switch (OBJ_obj2nid(qualinfo->pqualid)) {
            case NID_id_qt_cps:
                BIO_printf(out, "%*sCPS: %s\n", indent, "",
                           qualinfo->d.cpsuri->data);
                break;

            case NID_id_qt_unotice:
                BIO_printf(out, "%*sUser Notice:\n", indent, "");
                print_notice(out, qualinfo->d.usernotice, indent + 2);
                break;

            default:
                BIO_printf(out, "%*sUnknown Qualifier: ",
                           indent + 2, "");

                i2a_ASN1_OBJECT(out, qualinfo->pqualid);
                BIO_puts(out, "\n");
                break;
        }
    }
}

static void print_notice(BIO *out, USERNOTICE *notice, int indent)
{
    int i;
    if (notice->noticeref) {
        NOTICEREF *ref;
        ref = notice->noticeref;
        BIO_printf(out, "%*sOrganization: %s\n", indent, "",
                   ref->organization->data);
        BIO_printf(out, "%*sNumber%s: ", indent, "",
                   sk_ASN1_INTEGER_num(ref->noticenos) > 1 ? "s" : "");
        for (i = 0; i < sk_ASN1_INTEGER_num(ref->noticenos); i++) {
            ASN1_INTEGER *num;
            char *tmp;
            num = sk_ASN1_INTEGER_value(ref->noticenos, i);
            if (i)
                BIO_puts(out, ", ");
            tmp = i2s_ASN1_INTEGER(NULL, num);
            BIO_puts(out, tmp);
            free(tmp);
        }
        BIO_puts(out, "\n");
    }
    if (notice->exptext)
        BIO_printf(out, "%*sExplicit Text: %s\n", indent, "",
                   notice->exptext->data);
}

void X509_POLICY_NODE_print(BIO *out, X509_POLICY_NODE *node, int indent)
{
    const X509_POLICY_DATA *dat = node->data;

    BIO_printf(out, "%*sPolicy: ", indent, "");

    i2a_ASN1_OBJECT(out, dat->valid_policy);
    BIO_puts(out, "\n");
    BIO_printf(out, "%*s%s\n", indent + 2, "",
               node_data_critical(dat) ? "Critical" : "Non Critical");
    if (dat->qualifier_set)
        print_qualifiers(out, dat->qualifier_set, indent + 2);
    else
        BIO_printf(out, "%*sNo Qualifiers\n", indent + 2, "");
}
