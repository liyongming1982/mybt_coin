#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "kyk_block.h"
#include "kyk_buff.h"
#include "gens_block.h"
#include "mu_unit.h"

/* This block buf is sourced from: https://webbtc.com/block/00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee.hex */
/* The json format is: https://webbtc.com/block/00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee.json */
/* This block has 2 Tx, each txid is following: */
/* b1fea52486ce0c62bb442b530a3f0132b826c74e473d1f2c220bfa78111c5082 */
/* f4184fc596403b9d638783cf57adfe4c75c605f6356fbc91338530e9831e9e16 */
static uint8_t BLOCK_BUF[] = {
    0x01, 0x00, 0x00, 0x00, 0x55, 0xbd, 0x84, 0x0a,
    0x78, 0x79, 0x8a, 0xd0, 0xda, 0x85, 0x3f, 0x68,
    0x97, 0x4f, 0x3d, 0x18, 0x3e, 0x2b, 0xd1, 0xdb,
    0x6a, 0x84, 0x2c, 0x1f, 0xee, 0xcf, 0x22, 0x2a,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x10, 0x4c, 0xcb,
    0x05, 0x42, 0x1a, 0xb9, 0x3e, 0x63, 0xf8, 0xc3,
    0xce, 0x5c, 0x2c, 0x2e, 0x9d, 0xbb, 0x37, 0xde,
    0x27, 0x64, 0xb3, 0xa3, 0x17, 0x5c, 0x81, 0x66,
    0x56, 0x2c, 0xac, 0x7d, 0x51, 0xb9, 0x6a, 0x49,
    0xff, 0xff, 0x00, 0x1d, 0x28, 0x3e, 0x9e, 0x70,
    0x02, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0x07, 0x04, 0xff, 0xff, 0x00, 0x1d,
    0x01, 0x02, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00,
    0xf2, 0x05, 0x2a, 0x01, 0x00, 0x00, 0x00, 0x43,
    0x41, 0x04, 0xd4, 0x6c, 0x49, 0x68, 0xbd, 0xe0,
    0x28, 0x99, 0xd2, 0xaa, 0x09, 0x63, 0x36, 0x7c,
    0x7a, 0x6c, 0xe3, 0x4e, 0xec, 0x33, 0x2b, 0x32,
    0xe4, 0x2e, 0x5f, 0x34, 0x07, 0xe0, 0x52, 0xd6,
    0x4a, 0xc6, 0x25, 0xda, 0x6f, 0x07, 0x18, 0xe7,
    0xb3, 0x02, 0x14, 0x04, 0x34, 0xbd, 0x72, 0x57,
    0x06, 0x95, 0x7c, 0x09, 0x2d, 0xb5, 0x38, 0x05,
    0xb8, 0x21, 0xa8, 0x5b, 0x23, 0xa7, 0xac, 0x61,
    0x72, 0x5b, 0xac, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0xc9, 0x97, 0xa5, 0xe5,
    0x6e, 0x10, 0x41, 0x02, 0xfa, 0x20, 0x9c, 0x6a,
    0x85, 0x2d, 0xd9, 0x06, 0x60, 0xa2, 0x0b, 0x2d,
    0x9c, 0x35, 0x24, 0x23, 0xed, 0xce, 0x25, 0x85,
    0x7f, 0xcd, 0x37, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x48, 0x47, 0x30, 0x44, 0x02, 0x20, 0x4e, 0x45,
    0xe1, 0x69, 0x32, 0xb8, 0xaf, 0x51, 0x49, 0x61,
    0xa1, 0xd3, 0xa1, 0xa2, 0x5f, 0xdf, 0x3f, 0x4f,
    0x77, 0x32, 0xe9, 0xd6, 0x24, 0xc6, 0xc6, 0x15,
    0x48, 0xab, 0x5f, 0xb8, 0xcd, 0x41, 0x02, 0x20,
    0x18, 0x15, 0x22, 0xec, 0x8e, 0xca, 0x07, 0xde,
    0x48, 0x60, 0xa4, 0xac, 0xdd, 0x12, 0x90, 0x9d,
    0x83, 0x1c, 0xc5, 0x6c, 0xbb, 0xac, 0x46, 0x22,
    0x08, 0x22, 0x21, 0xa8, 0x76, 0x8d, 0x1d, 0x09,
    0x01, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0xca,
    0x9a, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x43, 0x41,
    0x04, 0xae, 0x1a, 0x62, 0xfe, 0x09, 0xc5, 0xf5,
    0x1b, 0x13, 0x90, 0x5f, 0x07, 0xf0, 0x6b, 0x99,
    0xa2, 0xf7, 0x15, 0x9b, 0x22, 0x25, 0xf3, 0x74,
    0xcd, 0x37, 0x8d, 0x71, 0x30, 0x2f, 0xa2, 0x84,
    0x14, 0xe7, 0xaa, 0xb3, 0x73, 0x97, 0xf5, 0x54,
    0xa7, 0xdf, 0x5f, 0x14, 0x2c, 0x21, 0xc1, 0xb7,
    0x30, 0x3b, 0x8a, 0x06, 0x26, 0xf1, 0xba, 0xde,
    0xd5, 0xc7, 0x2a, 0x70, 0x4f, 0x7e, 0x6c, 0xd8,
    0x4c, 0xac, 0x00, 0x28, 0x6b, 0xee, 0x00, 0x00,
    0x00, 0x00, 0x43, 0x41, 0x04, 0x11, 0xdb, 0x93,
    0xe1, 0xdc, 0xdb, 0x8a, 0x01, 0x6b, 0x49, 0x84,
    0x0f, 0x8c, 0x53, 0xbc, 0x1e, 0xb6, 0x8a, 0x38,
    0x2e, 0x97, 0xb1, 0x48, 0x2e, 0xca, 0xd7, 0xb1,
    0x48, 0xa6, 0x90, 0x9a, 0x5c, 0xb2, 0xe0, 0xea,
    0xdd, 0xfb, 0x84, 0xcc, 0xf9, 0x74, 0x44, 0x64,
    0xf8, 0x2e, 0x16, 0x0b, 0xfa, 0x9b, 0x8b, 0x64,
    0xf9, 0xd4, 0xc0, 0x3f, 0x99, 0x9b, 0x86, 0x43,
    0xf6, 0x56, 0xb4, 0x12, 0xa3, 0xac, 0x00, 0x00,
    0x00, 0x00
};

/*
 * This block header is sourced from https://webbtc.com/block/00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee.hex
*/
static uint8_t BLK_HD[KYK_BLK_HD_LEN] = {
    0x01, 0x00, 0x00, 0x00, 0x55, 0xbd, 0x84, 0x0a,
    0x78, 0x79, 0x8a, 0xd0, 0xda, 0x85, 0x3f, 0x68,
    0x97, 0x4f, 0x3d, 0x18, 0x3e, 0x2b, 0xd1, 0xdb,
    0x6a, 0x84, 0x2c, 0x1f, 0xee, 0xcf, 0x22, 0x2a,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x10, 0x4c, 0xcb,
    0x05, 0x42, 0x1a, 0xb9, 0x3e, 0x63, 0xf8, 0xc3,
    0xce, 0x5c, 0x2c, 0x2e, 0x9d, 0xbb, 0x37, 0xde,
    0x27, 0x64, 0xb3, 0xa3, 0x17, 0x5c, 0x81, 0x66,
    0x56, 0x2c, 0xac, 0x7d, 0x51, 0xb9, 0x6a, 0x49,
    0xff, 0xff, 0x00, 0x1d, 0x28, 0x3e, 0x9e, 0x70
};

/*
 * this block is sourced from https://webbtc.com/block/00000000c9ec538cab7f38ef9c67a95742f56ab07b0a37c5be6b02808dbfb4e0.json
 * block hash is 00000000c9ec538cab7f38ef9c67a95742f56ab07b0a37c5be6b02808dbfb4e0
 * this block's parent is BLOCK_BUF : https://webbtc.com/block/00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee.json
 */

static uint8_t BLOCK2_BUF[] = {
    0x01, 0x00, 0x00, 0x00, 0xee, 0xa2, 0xd4, 0x8d,
    0x2f, 0xce, 0xd4, 0x34, 0x68, 0x42, 0x83, 0x5c,
    0x65, 0x9e, 0x49, 0x3d, 0x32, 0x3f, 0x06, 0xd4,
    0x03, 0x44, 0x69, 0xa8, 0x90, 0x57, 0x14, 0xd1,
    0x00, 0x00, 0x00, 0x00, 0xf2, 0x93, 0xc8, 0x69,
    0x73, 0xe7, 0x58, 0xcc, 0xd1, 0x19, 0x75, 0xfa,
    0x46, 0x4d, 0x4c, 0x3e, 0x85, 0x00, 0x97, 0x9c,
    0x95, 0x42, 0x5c, 0x7b, 0xe6, 0xf0, 0xa6, 0x53,
    0x14, 0xd2, 0xf2, 0xd5, 0xc9, 0xba, 0x6a, 0x49,
    0xff, 0xff, 0x00, 0x1d, 0x07, 0xa8, 0xf2, 0x26,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
    0xff, 0xff, 0x07, 0x04, 0xff, 0xff, 0x00, 0x1d,
    0x01, 0x0e, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00,
    0xf2, 0x05, 0x2a, 0x01, 0x00, 0x00, 0x00, 0x43,
    0x41, 0x04, 0x56, 0x68, 0x24, 0xc3, 0x12, 0x07,
    0x33, 0x15, 0xdf, 0x60, 0xe5, 0xaa, 0x64, 0x90,
    0xb6, 0xcd, 0xd8, 0x0c, 0xd9, 0x0f, 0x6a, 0x8f,
    0x02, 0xe0, 0x22, 0xca, 0x3c, 0x2d, 0x52, 0x96,
    0x8c, 0x25, 0x30, 0x06, 0xc9, 0xc6, 0x02, 0xe0,
    0x3a, 0xed, 0x7b, 0xe5, 0x2d, 0x6a, 0xc5, 0x5f,
    0x5b, 0x55, 0x7c, 0x72, 0x52, 0x9b, 0xcc, 0x38,
    0x99, 0xac, 0xe7, 0xeb, 0x42, 0x27, 0x15, 0x3e,
    0xb4, 0x4b, 0xac, 0x00, 0x00, 0x00, 0x00
};

static uint8_t BLK2_HD[KYK_BLK_HD_LEN] = {
    0x01, 0x00, 0x00, 0x00, 0xee, 0xa2, 0xd4, 0x8d,
    0x2f, 0xce, 0xd4, 0x34, 0x68, 0x42, 0x83, 0x5c,
    0x65, 0x9e, 0x49, 0x3d, 0x32, 0x3f, 0x06, 0xd4,
    0x03, 0x44, 0x69, 0xa8, 0x90, 0x57, 0x14, 0xd1,
    0x00, 0x00, 0x00, 0x00, 0xf2, 0x93, 0xc8, 0x69,
    0x73, 0xe7, 0x58, 0xcc, 0xd1, 0x19, 0x75, 0xfa,
    0x46, 0x4d, 0x4c, 0x3e, 0x85, 0x00, 0x97, 0x9c,
    0x95, 0x42, 0x5c, 0x7b, 0xe6, 0xf0, 0xa6, 0x53
};


char *test_kyk_ser_blk()
{
    struct kyk_block* blk = NULL;
    struct kyk_buff* buf = NULL;
    char *errmsg = "failed to test kyk ser block";

    blk = make_gens_block();
    check(blk != NULL, "failed to make gens block");

    buf = create_kyk_buff(1000);
    check(buf != NULL, "failed to create kyk buff");

    kyk_ser_blk(buf, blk);

    mu_assert(buf -> idx == blk -> blk_size, "failed to get the correct block len");

    return NULL;

error:
    if(buf) free_kyk_buff(buf);
    if(blk) kyk_free_block(blk);
    return errmsg;
    
}

char* test_deseri_blk_header()
{
    struct kyk_blk_header* hd = NULL;
    size_t len = 0;
    int res = -1;
    uint8_t target_mkl_root[] = {
	0x7d, 0xac, 0x2c, 0x56, 0x66, 0x81, 0x5c, 0x17,
	0xa3, 0xb3, 0x64, 0x27, 0xde, 0x37, 0xbb, 0x9d,
	0x2e, 0x2c, 0x5c, 0xce, 0xc3, 0xf8, 0x63, 0x3e,
	0xb9, 0x1a, 0x42, 0x05, 0xcb, 0x4c, 0x10, 0xff
    };
    uint8_t target_blk_hash[] = {
	0x00, 0x00, 0x00, 0x00, 0xd1, 0x14, 0x57, 0x90,
	0xa8, 0x69, 0x44, 0x03, 0xd4, 0x06, 0x3f, 0x32,
	0x3d, 0x49, 0x9e, 0x65, 0x5c, 0x83, 0x42, 0x68,
	0x34, 0xd4, 0xce, 0x2f, 0x8d, 0xd4, 0xa2, 0xee
    };
    uint8_t blk_hash[32];

    uint8_t buf[1000];
    
    memcpy(buf, BLK_HD, sizeof(BLK_HD));

    hd = calloc(1, sizeof(*hd));
    check(hd, "Failed to test_deseri_blk_header: calloc blk header failed");

    res = kyk_deseri_blk_header(hd, buf, &len);
    mu_assert(res == 0, "Failed to test_deseri_blk_header");
    mu_assert(len == sizeof(BLK_HD), "Failed to test_deseri_blk_header");
    mu_assert(kyk_digest_eq(hd -> mrk_root_hash, target_mkl_root, sizeof(target_mkl_root)), "Failed to test_deseri_blk_header");

    res = kyk_blk_hash256(blk_hash, hd);
    mu_assert(res == 0, "Failed to test_deseri_blk_header");
    mu_assert(kyk_digest_eq(blk_hash, target_blk_hash, sizeof(target_blk_hash)), "Failed to test_deseri_blk_header");

    return NULL;
    
error:

    return "test_deseri_blk_header failed";
}


char* test_deseri_block()
{
    struct kyk_block* blk = NULL;
    uint8_t target_blk_hash[] = {
	0x00, 0x00, 0x00, 0x00, 0xd1, 0x14, 0x57, 0x90,
	0xa8, 0x69, 0x44, 0x03, 0xd4, 0x06, 0x3f, 0x32,
	0x3d, 0x49, 0x9e, 0x65, 0x5c, 0x83, 0x42, 0x68,
	0x34, 0xd4, 0xce, 0x2f, 0x8d, 0xd4, 0xa2, 0xee
    };
    uint8_t blk_hash[32];

    uint8_t target_tx1_id[] = {
	0xb1, 0xfe, 0xa5, 0x24, 0x86, 0xce, 0x0c, 0x62,
	0xbb, 0x44, 0x2b, 0x53, 0x0a, 0x3f, 0x01, 0x32,
	0xb8, 0x26, 0xc7, 0x4e, 0x47, 0x3d, 0x1f, 0x2c,
	0x22, 0x0b, 0xfa, 0x78, 0x11, 0x1c, 0x50, 0x82
    };

    uint8_t target_tx2_id[] = {
	0xf4, 0x18, 0x4f, 0xc5, 0x96, 0x40, 0x3b, 0x9d,
	0x63, 0x87, 0x83, 0xcf, 0x57, 0xad, 0xfe, 0x4c,
	0x75, 0xc6, 0x05, 0xf6, 0x35, 0x6f, 0xbc, 0x91,
	0x33, 0x85, 0x30, 0xe9, 0x83, 0x1e, 0x9e, 0x16
    };

    uint8_t digest[32];
    struct kyk_tx* tx = NULL;
    int res = -1;
    size_t blk_size = 0;

    
    blk = malloc(sizeof(*blk));
    check(blk, "Failed to test_parse_block: blk malloc failed");

    res = kyk_deseri_block(blk, BLOCK_BUF, &blk_size);
    mu_assert(res == 0, "Failed to test_deseri_block");
    res = kyk_blk_hash256(blk_hash, blk -> hd);
    mu_assert(res == 0, "Failed to test_parse_block: kyk_blk_hash256 failed");
    mu_assert(kyk_digest_eq(blk_hash, target_blk_hash, sizeof(blk_hash)), "Failed to test_parse_block");

    tx = blk -> tx;
    res = kyk_tx_hash256(digest, tx);
    check(res == 0, "Failed to test_deseri_block: kyk_tx_hash256 failed");
    mu_assert(kyk_digest_eq(digest, target_tx1_id, sizeof(digest)), "Failed to test_deseri_block");

    tx = blk -> tx + 1;
    
    res = kyk_tx_hash256(digest, tx);
    check(res == 0, "Failed to test_deseri_block: kyk_tx_hash256 failed");
    mu_assert(kyk_digest_eq(digest, target_tx2_id, sizeof(digest)), "Failed to test_deseri_block");

    kyk_free_block(blk);
    
    return NULL;

error:

    return "Failed to test_deseri_block";
}

/*
 * The target block header is sourced from
 * https://webbtc.com/block/00000000d1145790a8694403d4063f323d499e655c83426834d4ce2f8dd4a2ee.json
 */
char* test_make_blk_header()
{
    struct kyk_block* blk = NULL;
    size_t blk_len = 0;
    struct kyk_blk_header* hd = NULL;
    struct kyk_tx* tx_list = NULL;
    uint32_t version = 1;
    uint8_t pre_blk_hash[32] = {
	0x00, 0x00, 0x00, 0x00, 0x2a, 0x22, 0xcf, 0xee,
	0x1f, 0x2c, 0x84, 0x6a, 0xdb, 0xd1, 0x2b, 0x3e,
	0x18, 0x3d, 0x4f, 0x97, 0x68, 0x3f, 0x85, 0xda,
	0xd0, 0x8a, 0x79, 0x78, 0x0a, 0x84, 0xbd, 0x55
    };
    uint32_t tts = 1231731025;
    uint32_t bts = 486604799;

    uint8_t blk_hash[32];
    uint8_t target_blk_hash[32] = {
	0x00, 0x00, 0x00, 0x00, 0xd1, 0x14, 0x57, 0x90,
	0xa8, 0x69, 0x44, 0x03, 0xd4, 0x06, 0x3f, 0x32,
	0x3d, 0x49, 0x9e, 0x65, 0x5c, 0x83, 0x42, 0x68,
	0x34, 0xd4, 0xce, 0x2f, 0x8d, 0xd4, 0xa2, 0xee
    };
    
    int res = -1;

    blk = calloc(1, sizeof(*blk));
    res = kyk_deseri_block(blk, BLOCK_BUF, &blk_len);
    check(res == 0, "Failed to test_make_blk_header: kyk_deseri_block failed");

    tx_list = blk -> tx;

    hd = kyk_make_blk_header(tx_list,
			     blk -> tx_count,
			     version,
			     pre_blk_hash,
			     tts,
			     bts);

    /* suppose we have got the nonce by mining */
    hd -> nonce = 1889418792;

    check(hd, "Failed to test_make_blk_header: kyk_make_blk_header failed");

    res = kyk_blk_hash256(blk_hash, hd);
    check(res == 0, "Failed to test_make_blk_header: kyk_blk_hash256 failed");

    mu_assert(kyk_digest_eq(blk_hash, target_blk_hash, sizeof(blk_hash)), "Failed to test_make_blk_header");
    
    return NULL;

error:

    return "Failed to test_make_blk_header";
}

char* test_kyk_make_block()
{
    struct kyk_block* blk = NULL;
    struct kyk_block* blk2 = NULL;
    size_t blk_len = 0;
    size_t target_blk_size = 490;
    int res = -1;

    blk = calloc(1, sizeof(*blk));
    res = kyk_deseri_block(blk, BLOCK_BUF, &blk_len);
    check(res == 0, "Failed to test_make_blk_header: kyk_deseri_block failed");

    blk2 = calloc(1, sizeof(*blk2));
    res = kyk_make_block(blk2, blk -> hd, blk -> tx, blk -> tx_count);
    mu_assert(res == 0, "Failed to test_kyk_make_block");
    mu_assert(blk2 -> blk_size == target_blk_size, "Failed to test_kyk_make_block: invalid blk_size");

    return NULL;

error:

    return "Failed to test_kyk_make_block";
}

char* test_kyk_init_blk_hd_chain()
{
    struct kyk_blk_hd_chain* hdc = NULL;
    int res = -1;

    res = kyk_init_blk_hd_chain(&hdc);
    mu_assert(res == 0, "Failed to test_init_blk_hd_chain: kyk_init_blk_hd_chain failed");
    mu_assert(hdc, "Failed to test_kyk_init_blk_hd_chain: kyk_init_blk_hd_chain failed");
    mu_assert(hdc -> next == NULL, "Failed to test_kyk_init_blk_hd_chain: kyk_init_blk_hd_chain failed");
    mu_assert(hdc -> prev == NULL, "Failed to test_kyk_init_blk_hd_chain: kyk_init_blk_hd_chain failed");

    kyk_free_blk_hd_chain(hdc);

    return NULL;
}

char* test_kyk_append_blk_hd_chain()
{
    struct kyk_blk_hd_chain* hdc = NULL;
    struct kyk_blk_header* hd = NULL;
    struct kyk_blk_header* hd2 = NULL;
    size_t len = 0;
    int res = -1;

    res = kyk_init_blk_hd_chain(&hdc);

    hd = calloc(1, sizeof(*hd));
    check(hd, "Failed to test_kyk_append_blk_hd_chain: calloc blk header failed");
    res = kyk_deseri_blk_header(hd, BLK_HD, &len);
    check(res == 0, "Failed to test_kyk_append_blk_hd_chain: kyk_deseri_blk_header failed");

    hd2 = calloc(1, sizeof(*hd2));
    res = kyk_deseri_blk_header(hd2, BLK2_HD, &len);

    hdc -> hd = hd;
    res = kyk_append_blk_hd_chain(hdc, hd2);
    mu_assert(res == 0, "Failed to test_kyk_append_blk_hd_chain");

    kyk_free_blk_hd_chain(hdc);
    
    return NULL;

error:

    return "Failed to test_kyk_append_blk_hd_chain";
}


char* test2_kyk_append_blk_hd_chain()
{
    struct kyk_blk_hd_chain* hdc = NULL;
    struct kyk_blk_header* hd = NULL;
    struct kyk_blk_header* hd2 = NULL;
    size_t len = 0;
    int res = -1;

    res = kyk_init_blk_hd_chain(&hdc);

    hd = calloc(1, sizeof(*hd));
    check(hd, "Failed to test_kyk_append_blk_hd_chain: calloc blk header failed");
    res = kyk_deseri_blk_header(hd, BLK2_HD, &len);
    check(res == 0, "Failed to test_kyk_append_blk_hd_chain: kyk_deseri_blk_header failed");

    hd2 = calloc(1, sizeof(*hd2));
    res = kyk_deseri_blk_header(hd2, BLK_HD, &len);

    hdc -> hd = hd;
    res = kyk_append_blk_hd_chain(hdc, hd2);
    mu_assert(res == -1, "Failed to test_kyk_append_blk_hd_chain");

    kyk_free_blk_hd_chain(hdc);
    
    return NULL;

error:

    return "Failed to test_kyk_append_blk_hd_chain";
}

char* test_kyk_tail_blk_hd_chain()
{
    struct kyk_blk_hd_chain* hdc = NULL;
    struct kyk_blk_hd_chain* hdc_cpy = NULL;
    struct kyk_blk_header* hd = NULL;
    struct kyk_blk_header* hd2 = NULL;
    size_t len = 0;
    int res = -1;

    kyk_init_blk_hd_chain(&hdc);
    hd = calloc(1, sizeof(*hd));
    res = kyk_deseri_blk_header(hd, BLK_HD, &len);
    check(res == 0, "Failed to test_kyk_tail_blk_hd_chain: kyk_deseri_blk_header failed");

    hd2 = calloc(1, sizeof(*hd2));
    res = kyk_deseri_blk_header(hd2, BLK2_HD, &len);
    hdc -> hd = hd;
    res = kyk_append_blk_hd_chain(hdc, hd2);
    check(res == 0, "Failed to test_kyk_tail_blk_hd_chain: kyk_append_blk_hd_chain failed");

    hdc_cpy = hdc;

    res = kyk_tail_blk_hd_chain(&hdc_cpy);
    mu_assert(res == 0, "Failed to test_kyk_tail_blk_hd_chain");
    mu_assert(hdc -> next, "Failed to test_kyk_tail_blk_hd_chain");
    mu_assert(hdc_cpy -> next == NULL, "Failed to test_kyk_tail_blk_hd_chain");

    return NULL;

error:

    return "Failed to test_kyk_tail_blk_hd_chain";
}


char *all_tests()
{
    mu_suite_start();
    
    mu_run_test(test_kyk_ser_blk);    
    mu_run_test(test_deseri_blk_header);
    mu_run_test(test_deseri_block);
    mu_run_test(test_make_blk_header);
    mu_run_test(test_kyk_make_block);
    mu_run_test(test_kyk_init_blk_hd_chain);
    mu_run_test(test_kyk_append_blk_hd_chain);
    mu_run_test(test2_kyk_append_blk_hd_chain);
    
    return NULL;
}

MU_RUN_TESTS(all_tests);
