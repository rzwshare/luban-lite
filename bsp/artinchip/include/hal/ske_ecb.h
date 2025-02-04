#ifndef SKE_ECB_H
#define SKE_ECB_H

#include <ske.h>

#ifdef __cplusplus
extern "C" {
#endif

//APIs
u32 ske_sec_ecb_init(SKE_CTX *ctx, SKE_ALG alg, SKE_CRYPTO crypto, u8 *key, u16 sp_key_idx, 
        SKE_PADDING padding);
u32 ske_sec_ecb_update_blocks(SKE_CTX *ctx, u8 *in, u8 *out, u32 bytes);
u32 ske_sec_ecb_update_including_last_block(SKE_CTX *ctx, u8 *in, u8 *out, u32 in_bytes, 
        u32 *out_bytes);
u32 ske_sec_ecb_final(SKE_CTX *ctx);
u32 ske_sec_ecb_crypto(SKE_ALG alg, SKE_CRYPTO crypto, u8 *key, u16 sp_key_idx,
        SKE_PADDING padding, u8 *in, u8 *out, u32 in_bytes, u32 *out_bytes);

#ifdef __cplusplus
}
#endif

#endif

