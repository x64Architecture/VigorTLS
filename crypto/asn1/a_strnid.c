/*
 * Copyright 1999-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <openssl/asn1.h>
#include <openssl/err.h>
#include <openssl/objects.h>

static STACK_OF(ASN1_STRING_TABLE) *stable = NULL;
static void st_free(ASN1_STRING_TABLE *tbl);
static int sk_table_cmp(const ASN1_STRING_TABLE *const *a,
                        const ASN1_STRING_TABLE *const *b);

/* This is the global mask for the mbstring functions: this is use to
 * mask out certain types (such as BMPString and UTF8String) because
 * certain software (e.g. Netscape) has problems with them.
 */

static unsigned long global_mask = B_ASN1_UTF8STRING;

void ASN1_STRING_set_default_mask(unsigned long mask)
{
    global_mask = mask;
}

unsigned long ASN1_STRING_get_default_mask(void)
{
    return global_mask;
}

/* This function sets the default to various "flavours" of configuration.
 * based on an ASCII string. Currently this is:
 * MASK:XXXX : a numerical mask value.
 * nobmp : Don't use BMPStrings (just Printable, T61).
 * pkix : PKIX recommendation in RFC2459.
 * utf8only : only use UTF8Strings (RFC2459 recommendation for 2004).
 * default:   the default value, Printable, T61, BMP.
 */

int ASN1_STRING_set_default_mask_asc(const char *p)
{
    unsigned long mask;
    char *end;
    if (strncmp(p, "MASK:", 5) == 0) {
        if (!p[5])
            return 0;
        mask = strtoul(p + 5, &end, 0);
        if (*end)
            return 0;
    } else if (strcmp(p, "nombstr") == 0)
        mask = ~((unsigned long)(B_ASN1_BMPSTRING | B_ASN1_UTF8STRING));
    else if (strcmp(p, "pkix") == 0)
        mask = ~((unsigned long)B_ASN1_T61STRING);
    else if (strcmp(p, "utf8only") == 0)
        mask = B_ASN1_UTF8STRING;
    else if (strcmp(p, "default") == 0)
        mask = 0xFFFFFFFFL;
    else
        return 0;
    ASN1_STRING_set_default_mask(mask);
    return 1;
}

/* The following function generates an ASN1_STRING based on limits in a table.
 * Frequently the types and length of an ASN1_STRING are restricted by a
 * corresponding OID. For example certificates and certificate requests.
 */

ASN1_STRING *ASN1_STRING_set_by_NID(ASN1_STRING **out, const uint8_t *in,
                                    int inlen, int inform, int nid)
{
    ASN1_STRING_TABLE *tbl;
    ASN1_STRING *str = NULL;
    unsigned long mask;
    int ret;
    if (!out)
        out = &str;
    tbl = ASN1_STRING_TABLE_get(nid);
    if (tbl) {
        mask = tbl->mask;
        if (!(tbl->flags & STABLE_NO_MASK))
            mask &= global_mask;
        ret = ASN1_mbstring_ncopy(out, in, inlen, inform, mask,
                                  tbl->minsize, tbl->maxsize);
    } else
        ret = ASN1_mbstring_copy(out, in, inlen, inform, DIRSTRING_TYPE & global_mask);
    if (ret <= 0)
        return NULL;
    return *out;
}

/* Now the tables and helper functions for the string table:
 */

/* size limits: this stuff is taken straight from RFC3280 */

#define ub_name 32768
#define ub_common_name 64
#define ub_locality_name 128
#define ub_state_name 128
#define ub_organization_name 64
#define ub_organization_unit_name 64
#define ub_title 64
#define ub_email_address 128
#define ub_serial_number 64

/* This table must be kept in NID order */

static const ASN1_STRING_TABLE tbl_standard[] = {
    { NID_commonName, 1, ub_common_name, DIRSTRING_TYPE, 0 },
    { NID_countryName, 2, 2, B_ASN1_PRINTABLESTRING, STABLE_NO_MASK },
    { NID_localityName, 1, ub_locality_name, DIRSTRING_TYPE, 0 },
    { NID_stateOrProvinceName, 1, ub_state_name, DIRSTRING_TYPE, 0 },
    { NID_organizationName, 1, ub_organization_name, DIRSTRING_TYPE, 0 },
    { NID_organizationalUnitName, 1, ub_organization_unit_name, DIRSTRING_TYPE, 0 },
    { NID_pkcs9_emailAddress, 1, ub_email_address, B_ASN1_IA5STRING, STABLE_NO_MASK },
    { NID_pkcs9_unstructuredName, 1, -1, PKCS9STRING_TYPE, 0 },
    { NID_pkcs9_challengePassword, 1, -1, PKCS9STRING_TYPE, 0 },
    { NID_pkcs9_unstructuredAddress, 1, -1, DIRSTRING_TYPE, 0 },
    { NID_givenName, 1, ub_name, DIRSTRING_TYPE, 0 },
    { NID_surname, 1, ub_name, DIRSTRING_TYPE, 0 },
    { NID_initials, 1, ub_name, DIRSTRING_TYPE, 0 },
    { NID_serialNumber, 1, ub_serial_number, B_ASN1_PRINTABLESTRING, STABLE_NO_MASK },
    { NID_friendlyName, -1, -1, B_ASN1_BMPSTRING, STABLE_NO_MASK },
    { NID_name, 1, ub_name, DIRSTRING_TYPE, 0 },
    { NID_dnQualifier, -1, -1, B_ASN1_PRINTABLESTRING, STABLE_NO_MASK },
    { NID_domainComponent, 1, -1, B_ASN1_IA5STRING, STABLE_NO_MASK },
    { NID_ms_csp_name, -1, -1, B_ASN1_BMPSTRING, STABLE_NO_MASK }
};

static int sk_table_cmp(const ASN1_STRING_TABLE *const *a,
                        const ASN1_STRING_TABLE *const *b)
{
    return (*a)->nid - (*b)->nid;
}

DECLARE_OBJ_BSEARCH_CMP_FN(ASN1_STRING_TABLE, ASN1_STRING_TABLE, table);

static int table_cmp(const ASN1_STRING_TABLE *a, const ASN1_STRING_TABLE *b)
{
    return a->nid - b->nid;
}

IMPLEMENT_OBJ_BSEARCH_CMP_FN(ASN1_STRING_TABLE, ASN1_STRING_TABLE, table);

ASN1_STRING_TABLE *ASN1_STRING_TABLE_get(int nid)
{
    int idx;
    ASN1_STRING_TABLE *ttmp;
    ASN1_STRING_TABLE fnd;
    fnd.nid = nid;
    ttmp = OBJ_bsearch_table(&fnd, tbl_standard,
                             sizeof(tbl_standard) / sizeof(ASN1_STRING_TABLE));
    if (ttmp)
        return ttmp;
    if (!stable)
        return NULL;
    idx = sk_ASN1_STRING_TABLE_find(stable, &fnd);
    if (idx < 0)
        return NULL;
    return sk_ASN1_STRING_TABLE_value(stable, idx);
}

int ASN1_STRING_TABLE_add(int nid,
                          long minsize, long maxsize, unsigned long mask,
                          unsigned long flags)
{
    ASN1_STRING_TABLE *tmp;
    char new_nid = 0;
    flags &= ~STABLE_FLAGS_MALLOC;
    if (!stable)
        stable = sk_ASN1_STRING_TABLE_new(sk_table_cmp);
    if (!stable) {
        ASN1err(ASN1_F_ASN1_STRING_TABLE_ADD, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    if (!(tmp = ASN1_STRING_TABLE_get(nid))) {
        tmp = malloc(sizeof(ASN1_STRING_TABLE));
        if (!tmp) {
            ASN1err(ASN1_F_ASN1_STRING_TABLE_ADD,
                    ERR_R_MALLOC_FAILURE);
            return 0;
        }
        tmp->flags = flags | STABLE_FLAGS_MALLOC;
        tmp->nid = nid;
        tmp->minsize = tmp->maxsize = -1;
        new_nid = 1;
    } else
        tmp->flags = (tmp->flags & STABLE_FLAGS_MALLOC) | flags;
    if (minsize != -1)
        tmp->minsize = minsize;
    if (maxsize != -1)
        tmp->maxsize = maxsize;
    tmp->mask = mask;
    if (new_nid)
        sk_ASN1_STRING_TABLE_push(stable, tmp);
    return 1;
}

void ASN1_STRING_TABLE_cleanup(void)
{
    STACK_OF(ASN1_STRING_TABLE) *tmp;
    tmp = stable;
    if (!tmp)
        return;
    stable = NULL;
    sk_ASN1_STRING_TABLE_pop_free(tmp, st_free);
}

static void st_free(ASN1_STRING_TABLE *tbl)
{
    if (tbl->flags & STABLE_FLAGS_MALLOC)
        free(tbl);
}
