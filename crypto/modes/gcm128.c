/*
 * Copyright 2010-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <machine/endian.h>
#include <assert.h>
#include <string.h>

#include <openssl/crypto.h>

#include "modes_lcl.h"

#if defined(BSWAP4) && defined(STRICT_ALIGNMENT)
/* redefine, because alignment is ensured */
#undef GETU32
#define GETU32(p) BSWAP4(*(const uint32_t *)(p))
#undef PUTU32
#define PUTU32(p, v) *(uint32_t *)(p) = BSWAP4(v)
#endif

#define PACK(s) ((size_t)(s) << (sizeof(size_t) * 8 - 16))
#define REDUCE1BIT(V)                                                \
    do {                                                             \
        if (sizeof(size_t) == 8) {                                   \
            uint64_t T = U64(0xe100000000000000) & (0 - (V.lo & 1)); \
            V.lo = (V.hi << 63) | (V.lo >> 1);                       \
            V.hi = (V.hi >> 1) ^ T;                                  \
        } else {                                                     \
            uint32_t T = 0xe1000000U & (0 - (uint32_t)(V.lo & 1));   \
            V.lo = (V.hi << 63) | (V.lo >> 1);                       \
            V.hi = (V.hi >> 1) ^ ((uint64_t)T << 32);                \
        }                                                            \
    } while (0)

static void gcm_init_4bit(u128 Htable[16], uint64_t H[2])
{
    u128 V;
#if defined(OPENSSL_SMALL_FOOTPRINT)
    int i;
#endif

    Htable[0].hi = 0;
    Htable[0].lo = 0;
    V.hi = H[0];
    V.lo = H[1];

#if defined(OPENSSL_SMALL_FOOTPRINT)
    for (Htable[8] = V, i = 4; i > 0; i >>= 1) {
        REDUCE1BIT(V);
        Htable[i] = V;
    }

    for (i = 2; i < 16; i <<= 1) {
        u128 *Hi = Htable + i;
        int j;
        for (V = *Hi, j = 1; j < i; ++j) {
            Hi[j].hi = V.hi ^ Htable[j].hi;
            Hi[j].lo = V.lo ^ Htable[j].lo;
        }
    }
#else
    Htable[8] = V;
    REDUCE1BIT(V);
    Htable[4] = V;
    REDUCE1BIT(V);
    Htable[2] = V;
    REDUCE1BIT(V);
    Htable[1] = V;
    Htable[3].hi = V.hi ^ Htable[2].hi, Htable[3].lo = V.lo ^ Htable[2].lo;
    V = Htable[4];
    Htable[5].hi = V.hi ^ Htable[1].hi, Htable[5].lo = V.lo ^ Htable[1].lo;
    Htable[6].hi = V.hi ^ Htable[2].hi, Htable[6].lo = V.lo ^ Htable[2].lo;
    Htable[7].hi = V.hi ^ Htable[3].hi, Htable[7].lo = V.lo ^ Htable[3].lo;
    V = Htable[8];
    Htable[9].hi = V.hi ^ Htable[1].hi, Htable[9].lo = V.lo ^ Htable[1].lo;
    Htable[10].hi = V.hi ^ Htable[2].hi, Htable[10].lo = V.lo ^ Htable[2].lo;
    Htable[11].hi = V.hi ^ Htable[3].hi, Htable[11].lo = V.lo ^ Htable[3].lo;
    Htable[12].hi = V.hi ^ Htable[4].hi, Htable[12].lo = V.lo ^ Htable[4].lo;
    Htable[13].hi = V.hi ^ Htable[5].hi, Htable[13].lo = V.lo ^ Htable[5].lo;
    Htable[14].hi = V.hi ^ Htable[6].hi, Htable[14].lo = V.lo ^ Htable[6].lo;
    Htable[15].hi = V.hi ^ Htable[7].hi, Htable[15].lo = V.lo ^ Htable[7].lo;
#endif
#if defined(GHASH_ASM) && defined(VIGORTLS_ARM)
    /*
     * ARM assembler expects specific dword order in Htable.
     */
    {
        int j;

        if (BYTE_ORDER == LITTLE_ENDIAN)
            for (j = 0; j < 16; ++j) {
                V = Htable[j];
                Htable[j].hi = V.lo;
                Htable[j].lo = V.hi;
            }
        else
            for (j = 0; j < 16; ++j) {
                V = Htable[j];
                Htable[j].hi = V.lo << 32 | V.lo >> 32;
                Htable[j].lo = V.hi << 32 | V.hi >> 32;
            }
    }
#endif
}

#ifndef GHASH_ASM
static const size_t rem_4bit[16]
    = { PACK(0x0000), PACK(0x1C20), PACK(0x3840), PACK(0x2460),
        PACK(0x7080), PACK(0x6CA0), PACK(0x48C0), PACK(0x54E0),
        PACK(0xE100), PACK(0xFD20), PACK(0xD940), PACK(0xC560),
        PACK(0x9180), PACK(0x8DA0), PACK(0xA9C0), PACK(0xB5E0) };

static void gcm_gmult_4bit(uint64_t Xi[2], const u128 Htable[16])
{
    u128 Z;
    int cnt = 15;
    size_t rem, nlo, nhi;

    nlo = ((const uint8_t *)Xi)[15];
    nhi = nlo >> 4;
    nlo &= 0xf;

    Z.hi = Htable[nlo].hi;
    Z.lo = Htable[nlo].lo;

    while (1) {
        rem = (size_t)Z.lo & 0xf;
        Z.lo = (Z.hi << 60) | (Z.lo >> 4);
        Z.hi = (Z.hi >> 4);
        if (sizeof(size_t) == 8)
            Z.hi ^= rem_4bit[rem];
        else
            Z.hi ^= (uint64_t)rem_4bit[rem] << 32;

        Z.hi ^= Htable[nhi].hi;
        Z.lo ^= Htable[nhi].lo;

        if (--cnt < 0)
            break;

        nlo = ((const uint8_t *)Xi)[cnt];
        nhi = nlo >> 4;
        nlo &= 0xf;

        rem = (size_t)Z.lo & 0xf;
        Z.lo = (Z.hi << 60) | (Z.lo >> 4);
        Z.hi = (Z.hi >> 4);
        if (sizeof(size_t) == 8)
            Z.hi ^= rem_4bit[rem];
        else
            Z.hi ^= (uint64_t)rem_4bit[rem] << 32;

        Z.hi ^= Htable[nlo].hi;
        Z.lo ^= Htable[nlo].lo;
    }

    if (BYTE_ORDER == LITTLE_ENDIAN) {
#ifdef BSWAP8
        Xi[0] = BSWAP8(Z.hi);
        Xi[1] = BSWAP8(Z.lo);
#else
        uint8_t *p = (uint8_t *)Xi;
        uint32_t v;
        v = (uint32_t)(Z.hi >> 32);
        PUTU32(p, v);
        v = (uint32_t)(Z.hi);
        PUTU32(p + 4, v);
        v = (uint32_t)(Z.lo >> 32);
        PUTU32(p + 8, v);
        v = (uint32_t)(Z.lo);
        PUTU32(p + 12, v);
#endif
    } else {
        Xi[0] = Z.hi;
        Xi[1] = Z.lo;
    }
}

#if !defined(OPENSSL_SMALL_FOOTPRINT)
/*
 * Streamed gcm_mult_4bit, see CRYPTO_gcm128_[en|de]crypt for
 * details... Compiler-generated code doesn't seem to give any
 * performance improvement, at least not on x86[_64]. It's here
 * mostly as reference and a placeholder for possible future
 * non-trivial optimization[s]...
 */
static void gcm_ghash_4bit(uint64_t Xi[2], const u128 Htable[16],
                           const uint8_t *inp, size_t len)
{
    u128 Z;
    int cnt;
    size_t rem, nlo, nhi;

    do {
        cnt = 15;
        nlo = ((const uint8_t *)Xi)[15];
        nlo ^= inp[15];
        nhi = nlo >> 4;
        nlo &= 0xf;

        Z.hi = Htable[nlo].hi;
        Z.lo = Htable[nlo].lo;

        while (1) {
            rem = (size_t)Z.lo & 0xf;
            Z.lo = (Z.hi << 60) | (Z.lo >> 4);
            Z.hi = (Z.hi >> 4);
            if (sizeof(size_t) == 8)
                Z.hi ^= rem_4bit[rem];
            else
                Z.hi ^= (uint64_t)rem_4bit[rem] << 32;

            Z.hi ^= Htable[nhi].hi;
            Z.lo ^= Htable[nhi].lo;

            if (--cnt < 0)
                break;

            nlo = ((const uint8_t *)Xi)[cnt];
            nlo ^= inp[cnt];
            nhi = nlo >> 4;
            nlo &= 0xf;

            rem = (size_t)Z.lo & 0xf;
            Z.lo = (Z.hi << 60) | (Z.lo >> 4);
            Z.hi = (Z.hi >> 4);
            if (sizeof(size_t) == 8)
                Z.hi ^= rem_4bit[rem];
            else
                Z.hi ^= (uint64_t)rem_4bit[rem] << 32;

            Z.hi ^= Htable[nlo].hi;
            Z.lo ^= Htable[nlo].lo;
        }

        if (BYTE_ORDER == LITTLE_ENDIAN) {
#ifdef BSWAP8
            Xi[0] = BSWAP8(Z.hi);
            Xi[1] = BSWAP8(Z.lo);
#else
            uint8_t *p = (uint8_t *)Xi;
            uint32_t v;
            v = (uint32_t)(Z.hi >> 32);
            PUTU32(p, v);
            v = (uint32_t)(Z.hi);
            PUTU32(p + 4, v);
            v = (uint32_t)(Z.lo >> 32);
            PUTU32(p + 8, v);
            v = (uint32_t)(Z.lo);
            PUTU32(p + 12, v);
#endif
        } else {
            Xi[0] = Z.hi;
            Xi[1] = Z.lo;
        }
    } while (inp += 16, len -= 16);
}
#endif
#else
void gcm_gmult_4bit(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_4bit(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                    size_t len);
#endif

#define GCM_MUL(ctx, Xi) gcm_gmult_4bit(ctx->Xi.u, ctx->Htable)
#if defined(GHASH_ASM) || !defined(OPENSSL_SMALL_FOOTPRINT)
#define GHASH(ctx, in, len) gcm_ghash_4bit((ctx)->Xi.u, (ctx)->Htable, in, len)
/* GHASH_CHUNK is "stride parameter" missioned to mitigate cache
 * trashing effect. In other words idea is to hash data while it's
 * still in L1 cache after encryption pass... */
#define GHASH_CHUNK (3 * 1024)
#endif

#if defined(GHASH_ASM)
#if defined(VIGORTLS_X86) || defined(VIGORTLS_X86_64)
#define GHASH_ASM_X86_OR_64
#define GCM_FUNCREF_4BIT
extern unsigned int OPENSSL_ia32cap_P[2];

void gcm_init_clmul(u128 Htable[16], const uint64_t Xi[2]);
void gcm_gmult_clmul(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_clmul(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                     size_t len);

#if defined(VIGORTLS_X86)
# define gcm_init_avx  gcm_init_clmul
# define gcm_gmult_avx gcm_gmult_clmul
# define gcm_ghash_avx gcm_ghash_clmul
#else
void gcm_init_avx(u128 Htable[16], const uint64_t Xi[2]);
void gcm_gmult_avx(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_avx(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                   size_t len);
#endif

#if defined(VIGORTLS_X86)
#define GHASH_ASM_X86
void gcm_gmult_4bit_mmx(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_4bit_mmx(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len);

void gcm_gmult_4bit_x86(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_4bit_x86(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len);
#endif
#elif defined(VIGORTLS_ARM)
#include "arm_arch.h"
#if __ARM_MAX_ARCH__ >= 7
#define GHASH_ASM_ARM
#define GCM_FUNCREF_4BIT
void gcm_gmult_neon(uint64_t Xi[2], const u128 Htable[16]);
void gcm_ghash_neon(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                    size_t len);
#endif
#endif
#endif

#ifdef GCM_FUNCREF_4BIT
#undef GCM_MUL
#define GCM_MUL(ctx, Xi) (*gcm_gmult_p)(ctx->Xi.u, ctx->Htable)
#ifdef GHASH
#undef GHASH
#define GHASH(ctx, in, len) (*gcm_ghash_p)(ctx->Xi.u, ctx->Htable, in, len)
#endif
#endif

void CRYPTO_gcm128_init(GCM128_CONTEXT *ctx, void *key, block128_f block)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->block = block;
    ctx->key = key;

    (*block)(ctx->H.c, ctx->H.c, key);

    if (BYTE_ORDER == LITTLE_ENDIAN) {
/* H is stored in host byte order */
#ifdef BSWAP8
        ctx->H.u[0] = BSWAP8(ctx->H.u[0]);
        ctx->H.u[1] = BSWAP8(ctx->H.u[1]);
#else
        uint8_t *p = ctx->H.c;
        uint64_t hi, lo;
        hi = (uint64_t)GETU32(p) << 32 | GETU32(p + 4);
        lo = (uint64_t)GETU32(p + 8) << 32 | GETU32(p + 12);
        ctx->H.u[0] = hi;
        ctx->H.u[1] = lo;
#endif
    }

#if defined(GHASH_ASM_X86_OR_64)
#if !defined(GHASH_ASM_X86) || defined(OPENSSL_IA32_SSE2)
    if (OPENSSL_ia32cap_P[0] & (1 << 24) && /* check FXSR bit */
        OPENSSL_ia32cap_P[1] & (1 << 1)) {  /* check PCLMULQDQ bit */
        if (((OPENSSL_ia32cap_P[1] >> 22) & 0x41) == 0x41) {  /* AVX+MOVBE */
            gcm_init_avx(ctx->Htable, ctx->H.u);
            ctx->gmult = gcm_gmult_avx;
            ctx->ghash = gcm_ghash_avx;
        } else {
            gcm_init_clmul(ctx->Htable, ctx->H.u);
            ctx->gmult = gcm_gmult_clmul;
            ctx->ghash = gcm_ghash_clmul;
        }
        return;
    }
#endif
    gcm_init_4bit(ctx->Htable, ctx->H.u);
#if defined(GHASH_ASM_X86) /* x86 only */
#if defined(OPENSSL_IA32_SSE2)
    if (OPENSSL_ia32cap_P[0] & (1 << 25)) { /* check SSE bit */
#else
    if (OPENSSL_ia32cap_P[0] & (1 << 23)) { /* check MMX bit */
#endif
        ctx->gmult = gcm_gmult_4bit_mmx;
        ctx->ghash = gcm_ghash_4bit_mmx;
    } else {
        ctx->gmult = gcm_gmult_4bit_x86;
        ctx->ghash = gcm_ghash_4bit_x86;
    }
#else
    ctx->gmult = gcm_gmult_4bit;
    ctx->ghash = gcm_ghash_4bit;
#endif
#elif defined(GHASH_ASM_ARM)
    if (OPENSSL_armcap_P & ARMV7_NEON) {
        ctx->gmult = gcm_gmult_neon;
        ctx->ghash = gcm_ghash_neon;
    } else {
        gcm_init_4bit(ctx->Htable, ctx->H.u);
        ctx->gmult = gcm_gmult_4bit;
        ctx->ghash = gcm_ghash_4bit;
    }
#else
    gcm_init_4bit(ctx->Htable, ctx->H.u);
#endif
}

void CRYPTO_gcm128_setiv(GCM128_CONTEXT *ctx, const uint8_t *iv, size_t len)
{
    unsigned int ctr;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#endif

    ctx->Yi.u[0] = 0;
    ctx->Yi.u[1] = 0;
    ctx->Xi.u[0] = 0;
    ctx->Xi.u[1] = 0;
    ctx->len.u[0] = 0; /* AAD length */
    ctx->len.u[1] = 0; /* message length */
    ctx->ares = 0;
    ctx->mres = 0;

    if (len == 12) {
        memcpy(ctx->Yi.c, iv, 12);
        ctx->Yi.c[15] = 1;
        ctr = 1;
    } else {
        size_t i;
        uint64_t len0 = len;

        while (len >= 16) {
            for (i = 0; i < 16; ++i)
                ctx->Yi.c[i] ^= iv[i];
            GCM_MUL(ctx, Yi);
            iv += 16;
            len -= 16;
        }
        if (len) {
            for (i = 0; i < len; ++i)
                ctx->Yi.c[i] ^= iv[i];
            GCM_MUL(ctx, Yi);
        }
        len0 <<= 3;
        if (BYTE_ORDER == LITTLE_ENDIAN) {
#ifdef BSWAP8
            ctx->Yi.u[1] ^= BSWAP8(len0);
#else
            ctx->Yi.c[8] ^= (uint8_t)(len0 >> 56);
            ctx->Yi.c[9] ^= (uint8_t)(len0 >> 48);
            ctx->Yi.c[10] ^= (uint8_t)(len0 >> 40);
            ctx->Yi.c[11] ^= (uint8_t)(len0 >> 32);
            ctx->Yi.c[12] ^= (uint8_t)(len0 >> 24);
            ctx->Yi.c[13] ^= (uint8_t)(len0 >> 16);
            ctx->Yi.c[14] ^= (uint8_t)(len0 >> 8);
            ctx->Yi.c[15] ^= (uint8_t)(len0);
#endif
        } else
            ctx->Yi.u[1] ^= len0;

        GCM_MUL(ctx, Yi);

        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctr = BSWAP4(ctx->Yi.d[3]);
#else
            ctr = GETU32(ctx->Yi.c + 12);
#endif
        else
            ctr = ctx->Yi.d[3];
    }

    (*ctx->block)(ctx->Yi.c, ctx->EK0.c, ctx->key);
    ++ctr;
    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
        ctx->Yi.d[3] = BSWAP4(ctr);
#else
        PUTU32(ctx->Yi.c + 12, ctr);
#endif
    else
        ctx->Yi.d[3] = ctr;
}

int CRYPTO_gcm128_aad(GCM128_CONTEXT *ctx, const uint8_t *aad, size_t len)
{
    size_t i;
    unsigned int n;
    uint64_t alen = ctx->len.u[0];
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#ifdef GHASH
    void (*gcm_ghash_p)(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len) = ctx->ghash;
#endif
#endif

    if (ctx->len.u[1])
        return -2;

    alen += len;
    if (alen > (U64(1) << 61) || (sizeof(len) == 8 && alen < len))
        return -1;
    ctx->len.u[0] = alen;

    n = ctx->ares;
    if (n) {
        while (n && len) {
            ctx->Xi.c[n] ^= *(aad++);
            --len;
            n = (n + 1) % 16;
        }
        if (n == 0)
            GCM_MUL(ctx, Xi);
        else {
            ctx->ares = n;
            return 0;
        }
    }

#ifdef GHASH
    if ((i = (len & (size_t)-16))) {
        GHASH(ctx, aad, i);
        aad += i;
        len -= i;
    }
#else
    while (len >= 16) {
        for (i = 0; i < 16; ++i)
            ctx->Xi.c[i] ^= aad[i];
        GCM_MUL(ctx, Xi);
        aad += 16;
        len -= 16;
    }
#endif
    if (len) {
        n = (unsigned int)len;
        for (i = 0; i < len; ++i)
            ctx->Xi.c[i] ^= aad[i];
    }

    ctx->ares = n;
    return 0;
}

int CRYPTO_gcm128_encrypt(GCM128_CONTEXT *ctx, const uint8_t *in, uint8_t *out,
                          size_t len)
{
    unsigned int n, ctr;
    size_t i;
    uint64_t mlen = ctx->len.u[1];
    block128_f block = ctx->block;
    void *key = ctx->key;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#ifdef GHASH
    void (*gcm_ghash_p)(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len) = ctx->ghash;
#endif
#endif

    mlen += len;
    if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
        return -1;
    ctx->len.u[1] = mlen;

    if (ctx->ares) {
        /* First call to encrypt finalizes GHASH(AAD) */
        GCM_MUL(ctx, Xi);
        ctx->ares = 0;
    }

    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
        ctr = BSWAP4(ctx->Yi.d[3]);
#else
        ctr = GETU32(ctx->Yi.c + 12);
#endif
    else
        ctr = ctx->Yi.d[3];

    n = ctx->mres;
#if !defined(OPENSSL_SMALL_FOOTPRINT)
    if (16 % sizeof(size_t) == 0)
        do { /* always true actually */
            if (n) {
                while (n && len) {
                    ctx->Xi.c[n] ^= *(out++) = *(in++) ^ ctx->EKi.c[n];
                    --len;
                    n = (n + 1) % 16;
                }
                if (n == 0)
                    GCM_MUL(ctx, Xi);
                else {
                    ctx->mres = n;
                    return 0;
                }
            }
#if defined(STRICT_ALIGNMENT)
            if (((size_t)in | (size_t)out) % sizeof(size_t) != 0)
                break;
#endif
#if defined(GHASH) && defined(GHASH_CHUNK)
            while (len >= GHASH_CHUNK) {
                size_t j = GHASH_CHUNK;

                while (j) {
                    size_t *out_t = (size_t *)out;
                    const size_t *in_t = (const size_t *)in;

                    (*block)(ctx->Yi.c, ctx->EKi.c, key);
                    ++ctr;
                    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                        ctx->Yi.d[3] = BSWAP4(ctr);
#else
                        PUTU32(ctx->Yi.c + 12, ctr);
#endif
                    else
                        ctx->Yi.d[3] = ctr;
                    for (i = 0; i < 16 / sizeof(size_t); ++i)
                        out_t[i] = in_t[i] ^ ctx->EKi.t[i];
                    out += 16;
                    in += 16;
                    j -= 16;
                }
                GHASH(ctx, out - GHASH_CHUNK, GHASH_CHUNK);
                len -= GHASH_CHUNK;
            }
            if ((i = (len & (size_t)-16))) {
                size_t j = i;

                while (len >= 16) {
                    size_t *out_t = (size_t *)out;
                    const size_t *in_t = (const size_t *)in;

                    (*block)(ctx->Yi.c, ctx->EKi.c, key);
                    ++ctr;
                    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                        ctx->Yi.d[3] = BSWAP4(ctr);
#else
                        PUTU32(ctx->Yi.c + 12, ctr);
#endif
                    else
                        ctx->Yi.d[3] = ctr;
                    for (i = 0; i < 16 / sizeof(size_t); ++i)
                        out_t[i] = in_t[i] ^ ctx->EKi.t[i];
                    out += 16;
                    in += 16;
                    len -= 16;
                }
                GHASH(ctx, out - j, j);
            }
#else
            while (len >= 16) {
                size_t *out_t = (size_t *)out;
                const size_t *in_t = (const size_t *)in;

                (*block)(ctx->Yi.c, ctx->EKi.c, key);
                ++ctr;
                if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                    ctx->Yi.d[3] = BSWAP4(ctr);
#else
                    PUTU32(ctx->Yi.c + 12, ctr);
#endif
                else
                    ctx->Yi.d[3] = ctr;
                for (i = 0; i < 16 / sizeof(size_t); ++i)
                    ctx->Xi.t[i] ^= out_t[i] = in_t[i] ^ ctx->EKi.t[i];
                GCM_MUL(ctx, Xi);
                out += 16;
                in += 16;
                len -= 16;
            }
#endif
            if (len) {
                (*block)(ctx->Yi.c, ctx->EKi.c, key);
                ++ctr;
                if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                    ctx->Yi.d[3] = BSWAP4(ctr);
#else
                    PUTU32(ctx->Yi.c + 12, ctr);
#endif
                else
                    ctx->Yi.d[3] = ctr;
                while (len--) {
                    ctx->Xi.c[n] ^= out[n] = in[n] ^ ctx->EKi.c[n];
                    ++n;
                }
            }

            ctx->mres = n;
            return 0;
        } while (0);
#endif
    for (i = 0; i < len; ++i) {
        if (n == 0) {
            (*block)(ctx->Yi.c, ctx->EKi.c, key);
            ++ctr;
            if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                ctx->Yi.d[3] = BSWAP4(ctr);
#else
                PUTU32(ctx->Yi.c + 12, ctr);
#endif
            else
                ctx->Yi.d[3] = ctr;
        }
        ctx->Xi.c[n] ^= out[i] = in[i] ^ ctx->EKi.c[n];
        n = (n + 1) % 16;
        if (n == 0)
            GCM_MUL(ctx, Xi);
    }

    ctx->mres = n;
    return 0;
}

int CRYPTO_gcm128_decrypt(GCM128_CONTEXT *ctx, const uint8_t *in, uint8_t *out,
                          size_t len)
{
    unsigned int n, ctr;
    size_t i;
    uint64_t mlen = ctx->len.u[1];
    block128_f block = ctx->block;
    void *key = ctx->key;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#ifdef GHASH
    void (*gcm_ghash_p)(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len) = ctx->ghash;
#endif
#endif

    mlen += len;
    if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
        return -1;
    ctx->len.u[1] = mlen;

    if (ctx->ares) {
        /* First call to decrypt finalizes GHASH(AAD) */
        GCM_MUL(ctx, Xi);
        ctx->ares = 0;
    }

    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
        ctr = BSWAP4(ctx->Yi.d[3]);
#else
        ctr = GETU32(ctx->Yi.c + 12);
#endif
    else
        ctr = ctx->Yi.d[3];

    n = ctx->mres;
#if !defined(OPENSSL_SMALL_FOOTPRINT)
    if (16 % sizeof(size_t) == 0)
        do { /* always true actually */
            if (n) {
                while (n && len) {
                    uint8_t c = *(in++);
                    *(out++) = c ^ ctx->EKi.c[n];
                    ctx->Xi.c[n] ^= c;
                    --len;
                    n = (n + 1) % 16;
                }
                if (n == 0)
                    GCM_MUL(ctx, Xi);
                else {
                    ctx->mres = n;
                    return 0;
                }
            }
#if defined(STRICT_ALIGNMENT)
            if (((size_t)in | (size_t)out) % sizeof(size_t) != 0)
                break;
#endif
#if defined(GHASH) && defined(GHASH_CHUNK)
            while (len >= GHASH_CHUNK) {
                size_t j = GHASH_CHUNK;

                GHASH(ctx, in, GHASH_CHUNK);
                while (j) {
                    size_t *out_t = (size_t *)out;
                    const size_t *in_t = (const size_t *)in;

                    (*block)(ctx->Yi.c, ctx->EKi.c, key);
                    ++ctr;
                    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                        ctx->Yi.d[3] = BSWAP4(ctr);
#else
                        PUTU32(ctx->Yi.c + 12, ctr);
#endif
                    else
                        ctx->Yi.d[3] = ctr;
                    for (i = 0; i < 16 / sizeof(size_t); ++i)
                        out_t[i] = in_t[i] ^ ctx->EKi.t[i];
                    out += 16;
                    in += 16;
                    j -= 16;
                }
                len -= GHASH_CHUNK;
            }
            if ((i = (len & (size_t)-16))) {
                GHASH(ctx, in, i);
                while (len >= 16) {
                    size_t *out_t = (size_t *)out;
                    const size_t *in_t = (const size_t *)in;

                    (*block)(ctx->Yi.c, ctx->EKi.c, key);
                    ++ctr;
                    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                        ctx->Yi.d[3] = BSWAP4(ctr);
#else
                        PUTU32(ctx->Yi.c + 12, ctr);
#endif
                    else
                        ctx->Yi.d[3] = ctr;
                    for (i = 0; i < 16 / sizeof(size_t); ++i)
                        out_t[i] = in_t[i] ^ ctx->EKi.t[i];
                    out += 16;
                    in += 16;
                    len -= 16;
                }
            }
#else
            while (len >= 16) {
                size_t *out_t = (size_t *)out;
                const size_t *in_t = (const size_t *)in;

                (*block)(ctx->Yi.c, ctx->EKi.c, key);
                ++ctr;
                if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                    ctx->Yi.d[3] = BSWAP4(ctr);
#else
                    PUTU32(ctx->Yi.c + 12, ctr);
#endif
                else
                    ctx->Yi.d[3] = ctr;
                for (i = 0; i < 16 / sizeof(size_t); ++i) {
                    size_t c = in[i];
                    out[i] = c ^ ctx->EKi.t[i];
                    ctx->Xi.t[i] ^= c;
                }
                GCM_MUL(ctx, Xi);
                out += 16;
                in += 16;
                len -= 16;
            }
#endif
            if (len) {
                (*block)(ctx->Yi.c, ctx->EKi.c, key);
                ++ctr;
                if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                    ctx->Yi.d[3] = BSWAP4(ctr);
#else
                    PUTU32(ctx->Yi.c + 12, ctr);
#endif
                else
                    ctx->Yi.d[3] = ctr;
                while (len--) {
                    uint8_t c = in[n];
                    ctx->Xi.c[n] ^= c;
                    out[n] = c ^ ctx->EKi.c[n];
                    ++n;
                }
            }

            ctx->mres = n;
            return 0;
        } while (0);
#endif
    for (i = 0; i < len; ++i) {
        uint8_t c;
        if (n == 0) {
            (*block)(ctx->Yi.c, ctx->EKi.c, key);
            ++ctr;
            if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
                ctx->Yi.d[3] = BSWAP4(ctr);
#else
                PUTU32(ctx->Yi.c + 12, ctr);
#endif
            else
                ctx->Yi.d[3] = ctr;
        }
        c = in[i];
        out[i] = c ^ ctx->EKi.c[n];
        ctx->Xi.c[n] ^= c;
        n = (n + 1) % 16;
        if (n == 0)
            GCM_MUL(ctx, Xi);
    }

    ctx->mres = n;
    return 0;
}

int CRYPTO_gcm128_encrypt_ctr32(GCM128_CONTEXT *ctx, const uint8_t *in,
                                uint8_t *out, size_t len, ctr128_f stream)
{
    unsigned int n, ctr;
    size_t i;
    uint64_t mlen = ctx->len.u[1];
    void *key = ctx->key;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#ifdef GHASH
    void (*gcm_ghash_p)(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len) = ctx->ghash;
#endif
#endif

    mlen += len;
    if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
        return -1;
    ctx->len.u[1] = mlen;

    if (ctx->ares) {
        /* First call to encrypt finalizes GHASH(AAD) */
        GCM_MUL(ctx, Xi);
        ctx->ares = 0;
    }

    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
        ctr = BSWAP4(ctx->Yi.d[3]);
#else
        ctr = GETU32(ctx->Yi.c + 12);
#endif
    else
        ctr = ctx->Yi.d[3];

    n = ctx->mres;
    if (n) {
        while (n && len) {
            ctx->Xi.c[n] ^= *(out++) = *(in++) ^ ctx->EKi.c[n];
            --len;
            n = (n + 1) % 16;
        }
        if (n == 0)
            GCM_MUL(ctx, Xi);
        else {
            ctx->mres = n;
            return 0;
        }
    }
#if defined(GHASH) && !defined(OPENSSL_SMALL_FOOTPRINT)
    while (len >= GHASH_CHUNK) {
        (*stream)(in, out, GHASH_CHUNK / 16, key, ctx->Yi.c);
        ctr += GHASH_CHUNK / 16;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        GHASH(ctx, out, GHASH_CHUNK);
        out += GHASH_CHUNK;
        in += GHASH_CHUNK;
        len -= GHASH_CHUNK;
    }
#endif
    if ((i = (len & (size_t)-16))) {
        size_t j = i / 16;

        (*stream)(in, out, j, key, ctx->Yi.c);
        ctr += (unsigned int)j;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        in += i;
        len -= i;
#if defined(GHASH)
        GHASH(ctx, out, i);
        out += i;
#else
        while (j--) {
            for (i = 0; i < 16; ++i)
                ctx->Xi.c[i] ^= out[i];
            GCM_MUL(ctx, Xi);
            out += 16;
        }
#endif
    }
    if (len) {
        (*ctx->block)(ctx->Yi.c, ctx->EKi.c, key);
        ++ctr;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        while (len--) {
            ctx->Xi.c[n] ^= out[n] = in[n] ^ ctx->EKi.c[n];
            ++n;
        }
    }

    ctx->mres = n;
    return 0;
}

int CRYPTO_gcm128_decrypt_ctr32(GCM128_CONTEXT *ctx, const uint8_t *in,
                                uint8_t *out, size_t len, ctr128_f stream)
{
    unsigned int n, ctr;
    size_t i;
    uint64_t mlen = ctx->len.u[1];
    void *key = ctx->key;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#ifdef GHASH
    void (*gcm_ghash_p)(uint64_t Xi[2], const u128 Htable[16], const uint8_t *inp,
                        size_t len) = ctx->ghash;
#endif
#endif

    mlen += len;
    if (mlen > ((U64(1) << 36) - 32) || (sizeof(len) == 8 && mlen < len))
        return -1;
    ctx->len.u[1] = mlen;

    if (ctx->ares) {
        /* First call to decrypt finalizes GHASH(AAD) */
        GCM_MUL(ctx, Xi);
        ctx->ares = 0;
    }

    if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
        ctr = BSWAP4(ctx->Yi.d[3]);
#else
        ctr = GETU32(ctx->Yi.c + 12);
#endif
    else
        ctr = ctx->Yi.d[3];

    n = ctx->mres;
    if (n) {
        while (n && len) {
            uint8_t c = *(in++);
            *(out++) = c ^ ctx->EKi.c[n];
            ctx->Xi.c[n] ^= c;
            --len;
            n = (n + 1) % 16;
        }
        if (n == 0)
            GCM_MUL(ctx, Xi);
        else {
            ctx->mres = n;
            return 0;
        }
    }
#if defined(GHASH) && !defined(OPENSSL_SMALL_FOOTPRINT)
    while (len >= GHASH_CHUNK) {
        GHASH(ctx, in, GHASH_CHUNK);
        (*stream)(in, out, GHASH_CHUNK / 16, key, ctx->Yi.c);
        ctr += GHASH_CHUNK / 16;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        out += GHASH_CHUNK;
        in += GHASH_CHUNK;
        len -= GHASH_CHUNK;
    }
#endif
    if ((i = (len & (size_t)-16))) {
        size_t j = i / 16;

#if defined(GHASH)
        GHASH(ctx, in, i);
#else
        while (j--) {
            size_t k;
            for (k = 0; k < 16; ++k)
                ctx->Xi.c[k] ^= in[k];
            GCM_MUL(ctx, Xi);
            in += 16;
        }
        j = i / 16;
        in -= i;
#endif
        (*stream)(in, out, j, key, ctx->Yi.c);
        ctr += (unsigned int)j;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        out += i;
        in += i;
        len -= i;
    }
    if (len) {
        (*ctx->block)(ctx->Yi.c, ctx->EKi.c, key);
        ++ctr;
        if (BYTE_ORDER == LITTLE_ENDIAN)
#ifdef BSWAP4
            ctx->Yi.d[3] = BSWAP4(ctr);
#else
            PUTU32(ctx->Yi.c + 12, ctr);
#endif
        else
            ctx->Yi.d[3] = ctr;
        while (len--) {
            uint8_t c = in[n];
            ctx->Xi.c[n] ^= c;
            out[n] = c ^ ctx->EKi.c[n];
            ++n;
        }
    }

    ctx->mres = n;
    return 0;
}

int CRYPTO_gcm128_finish(GCM128_CONTEXT *ctx, const uint8_t *tag, size_t len)
{
    uint64_t alen = ctx->len.u[0] << 3;
    uint64_t clen = ctx->len.u[1] << 3;
#ifdef GCM_FUNCREF_4BIT
    void (*gcm_gmult_p)(uint64_t Xi[2], const u128 Htable[16]) = ctx->gmult;
#endif

    if (ctx->mres || ctx->ares)
        GCM_MUL(ctx, Xi);

    if (BYTE_ORDER == LITTLE_ENDIAN) {
#ifdef BSWAP8
        alen = BSWAP8(alen);
        clen = BSWAP8(clen);
#else
        uint8_t *p = ctx->len.c;

        ctx->len.u[0] = alen;
        ctx->len.u[1] = clen;

        alen = (uint64_t)GETU32(p) << 32 | GETU32(p + 4);
        clen = (uint64_t)GETU32(p + 8) << 32 | GETU32(p + 12);
#endif
    }

    ctx->Xi.u[0] ^= alen;
    ctx->Xi.u[1] ^= clen;
    GCM_MUL(ctx, Xi);

    ctx->Xi.u[0] ^= ctx->EK0.u[0];
    ctx->Xi.u[1] ^= ctx->EK0.u[1];

    if (tag && len <= sizeof(ctx->Xi))
        return CRYPTO_memcmp(ctx->Xi.c, tag, len);
    else
        return -1;
}

void CRYPTO_gcm128_tag(GCM128_CONTEXT *ctx, uint8_t *tag, size_t len)
{
    CRYPTO_gcm128_finish(ctx, NULL, 0);
    memcpy(tag, ctx->Xi.c, len <= sizeof(ctx->Xi.c) ? len : sizeof(ctx->Xi.c));
}

GCM128_CONTEXT *CRYPTO_gcm128_new(void *key, block128_f block)
{
    GCM128_CONTEXT *ret;

    ret = malloc(sizeof(GCM128_CONTEXT));
    if (ret == NULL)
        return NULL;

    CRYPTO_gcm128_init(ret, key, block);

    return ret;
}

void CRYPTO_gcm128_release(GCM128_CONTEXT *ctx)
{
    if (ctx == NULL)
        return;
    vigortls_zeroize(ctx, sizeof(*ctx));
    free(ctx);
}
