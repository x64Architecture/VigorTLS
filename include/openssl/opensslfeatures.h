#define OPENSSL_NO_CAMELLIA
#define OPENSSL_NO_EC_NISTP_64_GCC_128
#define OPENSSL_NO_CMS
#define OPENSSL_NO_COMP
#define OPENSSL_NO_GMP
#define OPENSSL_NO_GOST
#define OPENSSL_NO_JPAKE
#define OPENSSL_NO_KRB5
#define OPENSSL_NO_MD2
#define OPENSSL_NO_PSK
#define OPENSSL_NO_RC5
#define OPENSSL_NO_RFC3779
#define OPENSSL_NO_SCTP
#define OPENSSL_NO_SEED
#define OPENSSL_NO_SRP
#define OPENSSL_NO_SSL2
#define OPENSSL_NO_STORE
#define OPENSSL_NO_BUF_FREELISTS
#define OPENSSL_NO_HEARTBEATS
#define OPENSSL_NO_DYNAMIC_ENGINE
#define OPENSSL_THREADS

#ifndef OPENSSL_NO_ASM
 #define AES_ASM
 #define VPAES_ASM
 #define OPENSSL_BN_ASM_MONT
 #define OPENSSL_IA32_SSE2
#if defined(__x86_64) || defined(__x86_64__)
 #define OPENSSL_BN_ASM_MONT5
 #define OPENSSL_BN_ASM_GF2m
#endif
#endif
