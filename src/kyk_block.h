#ifndef KYK_BLOCK_H__
#define KYK_BLOCK_H__

#include "varint.h"
#include "kyk_tx.h"
#include "kyk_ldb.h"
#include "kyk_buff.h"

#define KYK_BLK_HD_LEN 80
#define KYK_BLK_HD_NO_NONCE_LEN 76
#define KYK_BLK_MAGIC_NO 0xD9B4BEF9

struct kyk_blk_hd_chain {
    struct kyk_blk_header* hd;
    struct kyk_blk_hd_chain* prev;
    struct kyk_blk_hd_chain* next;
};

struct kyk_blk_header {
    uint32_t version;
    uint8_t pre_blk_hash[32];
    uint8_t mrk_root_hash[32];
    uint32_t tts;
    uint32_t bts;
    uint32_t nonce;
    uint8_t blk_hash[32];
};

struct kyk_block {
    uint32_t magic_no;
    uint32_t blk_size;
    struct kyk_blk_header *hd;
    varint_t tx_count;
    struct kyk_tx *tx;
};

size_t kyk_seri_blk_hd(uint8_t *buf, const struct kyk_blk_header *hd);
size_t kyk_seri_blk_hd_without_nonce(uint8_t *buf, const struct kyk_blk_header *hd);
size_t kyk_ser_blk(struct kyk_buff* buf, const struct kyk_block* blk);
size_t kyk_ser_blk_for_file(struct kyk_buff* buf, const struct kyk_block* blk);

int kyk_deseri_block(struct kyk_block* blk,
		     const uint8_t* buf,
		     size_t* byte_num);

int kyk_deseri_blk_header(struct kyk_blk_header *hd,
			  const uint8_t *buf,
			  size_t* len);

struct kyk_blk_header* kyk_make_blk_header(struct kyk_tx* tx_list,
					   size_t tx_count,
					   uint32_t version,
					   uint8_t* pre_blk_hash,
					   uint32_t tts,
					   uint32_t bts);

int kyk_make_block(struct kyk_block* blk,
		   struct kyk_blk_header* blk_hd,
		   struct kyk_tx* tx_list,
		   varint_t tx_count);

int kyk_blk_hash256(uint8_t* digest, const struct kyk_blk_header* hd);


void kyk_free_block(struct kyk_block *blk);

int kyk_init_block(struct kyk_block *blk);

int kyk_validate_blk_header(struct kyk_blk_hd_chain* hd_chain,
			    const struct kyk_blk_header* hd);

int kyk_append_blk_hd_chain(struct kyk_blk_hd_chain* hd_chain,
			    struct kyk_blk_header* hd);

int kyk_init_blk_hd_chain(struct kyk_blk_hd_chain** hd_chain);

void kyk_free_blk_hd_chain(struct kyk_blk_hd_chain* hd_chain);

int kyk_tail_blk_hd_chain(struct kyk_blk_hd_chain** hd_chain);

#endif
