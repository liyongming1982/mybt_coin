#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kyk_tx.h"
#include "varint.h"
#include "beej_pack.h"
#include "kyk_utils.h"
#include "kyk_script.h"
#include "kyk_buff.h"
#include "kyk_sha.h"
#include "dbg.h"


static size_t kyk_seri_txin(unsigned char *buf, struct kyk_txin *txin);
static size_t kyk_seri_txin_list(unsigned char *buf, struct kyk_txin *txin, size_t count);
static size_t kyk_seri_txout(unsigned char *buf, struct kyk_txout *txout);
static size_t kyk_seri_txout_list(unsigned char *buf, struct kyk_txout *txout, size_t count);
static int kyk_make_coinbase_sc(struct kyk_txin *txin, const char *cb_note);
static int get_txin_size(struct kyk_txin* txin, size_t* txin_size);
static int get_txout_size(struct kyk_txout* txout, size_t* txout_size);

int kyk_deseri_txin_list(struct kyk_txin* txin_list,
			 size_t txin_count,
			 const uint8_t* buf,
			 size_t* byte_num);

int kyk_deseri_txin(struct kyk_txin* txin,
		    const uint8_t* buf,
		    size_t* byte_num);


int kyk_deseri_txout_list(struct kyk_txout* txout_list,
			  size_t txout_count,
			  const uint8_t* buf,
			  size_t* byte_num);

int kyk_deseri_txout(struct kyk_txout* txout,
		     const uint8_t* buf,
		     size_t* byte_num);


int kyk_tx_hash256(uint8_t* digest, const struct kyk_tx* tx)
{
    uint8_t *buf = NULL;
    size_t len = 0;
    size_t tx_size = 0;
    int res = -1;

    check(digest, "Failed to kyk_tx_hash256: digest is NULL");
    check(tx, "Failed to kyk_tx_hash256: tx is NULL");

    res = kyk_get_tx_size(tx, &tx_size);
    check(res == 0, "Failed to kyk_tx_hash256: kyk_get_tx_size failed");
    check(tx_size > 0, "Failed to kyk_tx_hash256: kyk_get_tx_size failed");

    buf = calloc(tx_size, sizeof(*buf));
    len = kyk_seri_tx((unsigned char*)buf, tx);
    check(len == tx_size, "Failed to kyk_tx_hash256: kyk_seri_tx failed");
    
    kyk_dgst_hash256(digest, buf, tx_size);
    kyk_reverse(digest, SHA256_DIGEST_LENGTH);

    return 0;
    
error:

    return -1;
}

int kyk_copy_tx(struct kyk_tx* dest_tx, const struct kyk_tx* src_tx)
{
    size_t i = 0;
    int res = -1;
    struct kyk_txin* txin = NULL;
    struct kyk_txout* txout = NULL;
    check(dest_tx, "Failed to kyk_copy_tx: dest_tx is NULL");
    check(src_tx, "Failed to kyk_copy_tx: src_tx is NULL");

    dest_tx -> version = src_tx -> version;
    dest_tx -> vin_sz = src_tx -> vin_sz;
    txin = src_tx -> txin;
    dest_tx -> txin = calloc(dest_tx -> vin_sz, sizeof(struct kyk_txin));
    check(dest_tx -> txin, "Failed to kyk_copy_tx: calloc dest_tx -> txin failed");
    for(i = 0; i < src_tx -> vin_sz; i++){
	res = kyk_add_txin(dest_tx, i, txin);
	check(res == 0, "failed to kyk_copy_tx: kyk_add_txin failed");
    }

    dest_tx -> vout_sz = src_tx -> vout_sz;
    txout = src_tx -> txout;
    dest_tx -> txout = calloc(dest_tx -> vout_sz, sizeof(struct kyk_txout));
    check(dest_tx -> txout, "Failed to kyk_copy_tx: calloc dest_tx -> txout failed");
    for(i = 0; i < src_tx -> vout_sz; i++){
	res = kyk_add_txout(dest_tx, i, txout);
	check(res == 0, "failed to kyk_copy_tx: kyk_add_txout failed");
    }

    dest_tx -> lock_time = src_tx -> lock_time;

    return 0;
error:

    return -1;
}

int kyk_get_tx_size(const struct kyk_tx* tx, size_t* tx_size)
{
    size_t len = 0;
    size_t i = 0;
    int res = -1;
    struct kyk_txin* txin = NULL;
    struct kyk_txout* txout = NULL;

    check(tx, "Failed to kyk_get_tx_size: tx is NULL");
    check(tx_size, "Failed to kyk_get_tx_size: tx_size is NULL");
    
    len += sizeof(tx -> version);
    len += get_varint_size(tx -> vin_sz);
    check(tx -> txin, "Failed to kyk_get_tx_size: tx -> txin is NULL");
    
    for(i = 0; i < tx -> vin_sz; i++){
	size_t txin_size = 0;
	txin = tx -> txin + i;
	res = get_txin_size(txin, &txin_size);
	check(res == 0, "Failed to kyk_get_tx_size: get_txin_size failed");

	len += txin_size;
    }

    len += get_varint_size(tx -> vout_sz);

    check(tx -> txout, "Failed to kyk_get_tx_size: tx -> txout is NULL");
    for(i = 0; i < tx -> vout_sz; i++){
	size_t txout_size = 0;
	txout = tx -> txout + i;
	res = get_txout_size(txout, &txout_size);
	check(res == 0, "Failed to kyk_get_tx_size: get_txout_size failed");

	len += txout_size;
    }

    len += sizeof(tx -> lock_time);

    *tx_size = len;

    return 0;
    
error:

    return -1;
    
}

int get_txin_size(struct kyk_txin* txin, size_t* txin_size)
{
    size_t len = 0;
    len += sizeof(txin -> pre_txid);
    len += sizeof(txin -> pre_tx_inx);
    check(txin -> sc_size >= 1, "Failed to get_txin_size: txin -> sc_size is invalid");
    len += get_varint_size(txin -> sc_size);
    len += txin -> sc_size;
    len += sizeof(txin -> seq_no);

    *txin_size = len;

    return 0;

error:

    return -1;
}

int get_txout_size(struct kyk_txout* txout, size_t* txout_size)
{
    size_t len = 0;
    len += sizeof(txout -> value);
    check(txout -> sc_size >= 1, "Failed to get_txout_size: txout -> sc_size is invalid");
    len += get_varint_size(txout -> sc_size);
    len += txout -> sc_size;

    *txout_size = len;

    return 0;

error:
    return -1;
}

int kyk_seri_tx_list(struct kyk_bon_buff* buf_list,
		     const struct kyk_tx* tx_list,
		     size_t tx_count)
{
    struct kyk_bon_buff* buf = NULL;
    const struct kyk_tx* tx = NULL;
    size_t i = 0;
    size_t len = 0;
    size_t tx_size = 0;
    int res = -1;

    for(i = 0; i < tx_count; i++){
	buf = buf_list + i;
	check(buf, "Failed to kyk_seri_tx_list: buf is NULL");	
	tx = &tx_list[i];
	check(tx, "Failed to kyk_seri_tx_list: tx is NULL");
	if(buf -> base) free(buf -> base);
	res = kyk_get_tx_size(tx, &tx_size);
	check(res == 0, "Failed to kyk_seri_tx_list: kyk_get_tx_size failed");
	buf -> base = calloc(tx_size, sizeof(*buf -> base));
	check(buf -> base, "Failed to kyk_seri_tx_list: calloc buf -> base failed");
	len = kyk_seri_tx(buf -> base, tx);
	check(len > 0, "Failed to kyk_seri_tx_list: kyk_seri_tx failed");
	buf -> len = len;
    }
    
    return 0;

error:

    return -1;
}


size_t kyk_seri_tx(unsigned char *buf, const struct kyk_tx *tx)
{
    size_t size;
    size_t total = 0;

    size = beej_pack(buf, "<L", tx -> version);
    buf += size;
    total += size;

    size = kyk_pack_varint(buf, tx -> vin_sz);
    buf += size;
    total += size;

    size = kyk_seri_txin_list(buf, tx -> txin, tx -> vin_sz);
    buf += size;
    total += size;

    size = kyk_pack_varint(buf, tx -> vout_sz);
    buf += size;
    total += size;

    size = kyk_seri_txout_list(buf, tx -> txout, tx -> vout_sz);
    buf += size;
    total += size;

    size = beej_pack(buf, "<L", tx -> lock_time);
    buf += size;
    total += size;

    return total;
}

size_t kyk_seri_txin_list(unsigned char *buf, struct kyk_txin *txin, size_t count)
{
    size_t size = 0;
    size_t i = 0;
    size_t total = 0;

    for(i=0; i < count; i++){
	size = kyk_seri_txin(buf, txin);
	txin++;
	buf += size;
	total += size;
    }

    return total;
}

size_t kyk_seri_txout_list(unsigned char *buf, struct kyk_txout *txout, size_t count)
{
    size_t size = 0;
    size_t total = 0;
    size_t i = 0;

    for(i=0; i < count; i++){
	size = kyk_seri_txout(buf, txout);
	txout++;
	buf += size;
	total += size;
    }

    return total;
}


size_t kyk_seri_txin(unsigned char *buf, struct kyk_txin *txin)
{
    size_t size;
    size_t total = 0;

    size = kyk_reverse_pack_chars(buf, txin -> pre_txid, sizeof(txin->pre_txid));
    buf += size;
    total += size;

    size = beej_pack(buf, "<L", txin -> pre_tx_inx);
    buf += size;
    total += size;

    size = kyk_pack_varint(buf, txin -> sc_size);
    buf += size;
    total += size;

    size = kyk_pack_chars(buf, txin -> sc, txin -> sc_size);
    buf += size;
    total += size;

    size = beej_pack(buf, "<L", txin -> seq_no);
    buf += size;
    total += size;

    return total;
}

size_t kyk_seri_txout(unsigned char *buf, struct kyk_txout *txout)
{
    size_t size;
    size_t total = 0;

    size = beej_pack(buf, "<Q", txout -> value);
    buf += size;
    total += size;

    size = kyk_pack_varint(buf, txout -> sc_size);
    buf += size;
    total += size;

    size = kyk_pack_chars(buf, txout -> sc, txout -> sc_size);
    buf += size;
    total += size;

    return total;
}

int kyk_add_txin(struct kyk_tx* tx,
		 size_t inx,
		 const struct kyk_txin* out_txin)
{
    check(tx, "Failed to kyk_add_txin: tx is NULL");
    check(inx >= 0, "Failed to kyk_add_txin: inx is invalid");
    check(out_txin, "Failed to kyk_add_txin: out_txin is NULL");

    struct kyk_txin* txin = NULL;

    txin = tx -> txin + inx;
    check(txin, "Failed to kyk_add_txin: txin out of memory");

    memcpy(txin -> pre_txid, out_txin -> pre_txid, sizeof(txin -> pre_txid));
    
    txin -> pre_tx_inx = out_txin -> pre_tx_inx;
    txin -> sc_size = out_txin -> sc_size;
    
    if(txin -> sc){
	free(txin -> sc);
	txin -> sc = NULL;
    }
    txin -> sc = calloc(txin -> sc_size, sizeof(unsigned char));
    check(txin -> sc, "Failed to kyk_add_txin: txin -> sc is NULL");
    
    memcpy(txin -> sc, out_txin -> sc, txin -> sc_size);
    txin -> seq_no = out_txin -> seq_no;

    return 0;


error:

    return -1;
}

int kyk_add_txout(struct kyk_tx* tx,
		  size_t inx,
		  const struct kyk_txout* out_txout)
{
    check(tx, "Failed to kyk_add_txout: tx is NULL");
    check(inx >= 0, "Failed to kyk_add_txout: inx is invalid");
    check(out_txout, "Failed to kyk_add_txout: out_txout is NULL");
    check(out_txout -> sc_size > 0, "Failed to kyk_add_txout: out_txout -> sc_size is inivalid");

    struct kyk_txout* txout = NULL;

    txout = tx -> txout + inx;
    check(txout, "Failed to kyk_add_txin: txout out of memory");

    txout -> value = out_txout -> value;
    txout -> sc_size = out_txout -> sc_size;

    if(txout -> sc) free(txout -> sc);
    txout -> sc = calloc(txout -> sc_size, sizeof(unsigned char));
    check(txout -> sc, "Failed to kyk_add_txout: txout -> sc calloc failed");
    memcpy(txout -> sc, out_txout -> sc, txout -> sc_size);

    return 0;
 
error:

    return -1;

}


struct kyk_txin *create_txin(const char *pre_txid,
			     uint32_t pre_tx_inx,
			     varint_t sc_size,
			     const char *sc,
			     uint32_t seq_no)
{
    struct kyk_txin *txin = malloc(sizeof(struct kyk_txin));
    if(txin == NULL){
	fprintf(stderr, "failed in malloc kyk_txin \n");
	exit(1);
    }

    if(hexstr_to_bytes(pre_txid, txin->pre_txid, 32) == -1){
	fprintf(stderr, "failed in setting pre_txid \n");
	exit(1);
    }

    txin->pre_tx_inx = pre_tx_inx;
    txin->sc_size = sc_size;
    txin->sc = malloc(sc_size * sizeof(unsigned char));
    if(hexstr_to_bytes(sc, txin->sc, sc_size) == -1){
	fprintf(stderr, "failed in setting txin sc \n");
	exit(1);
    }

    txin->seq_no = seq_no;

    return txin;
}

struct kyk_txout *create_txout(uint64_t value,
			       varint_t sc_size,
			       const char *sc)
{
    struct kyk_txout *txout = malloc(sizeof(struct kyk_txout));
    if(txout == NULL){
	fprintf(stderr, "failed in malloc kyk_txout \n");
	exit(1);
    }

    txout -> value = value;
    txout -> sc_size = sc_size;
    txout -> sc = malloc(sc_size * sizeof(unsigned char));
    
    if(hexstr_to_bytes(sc, txout->sc, sc_size) == -1){
	fprintf(stderr, "failed in setting txout sc \n");
	exit(1);
    }

    return txout;
}


void kyk_free_tx(struct kyk_tx *tx)
{
    if(tx){
	if(tx -> txin) {
	    kyk_free_txin(tx -> txin);
	    tx -> txin = NULL;
	}
	if(tx -> txout) {
	    kyk_free_txout(tx -> txout);
	    tx -> txout = NULL;
	}
	
	free(tx);
	tx = NULL;
    }
}

void kyk_free_txin(struct kyk_txin* txin)
{
    if(txin){
	if(txin -> sc) {
	    free(txin -> sc);
	    txin -> sc = NULL;
	}
	free(txin);
	txin = NULL;
    }

}

void kyk_free_txout(struct kyk_txout *txout)
{
    if(txout) {
	if(txout -> sc){
	    free(txout -> sc);
	    txout -> sc = NULL;
	}
    
	free(txout);
	txout = NULL;
    }
}

int kyk_make_coinbase_sc(struct kyk_txin *txin, const char *cb_note)
{
    unsigned char cb_tmp[1000] = {0x04, 0xff, 0xff, 0x00, 0x1d, 0x01, 0x04};
    size_t cb_len = 7;
    size_t cb_note_len = strlen(cb_note);

    cb_tmp[7] = (uint8_t) cb_note_len;
    cb_len += 1;

    memcpy(cb_tmp + 8, cb_note, cb_note_len);
    cb_len += cb_note_len;

    txin -> sc_size = cb_len;
    check(txin -> sc_size < MAX_COINBASE_SC_LEN, "Failed to kyk_make_coinbase: over MAX_COINBASE_SC_LEN");

    txin -> sc = malloc(txin -> sc_size * sizeof(unsigned char));
    check(txin -> sc, "Failed to kyk_make_coinbase: malloc error");
    
    memcpy(txin -> sc, cb_tmp, txin -> sc_size);

    return 0;

error:
    return -1;
}

int kyk_make_coinbase_tx(struct kyk_tx** cb_tx,
			 const char* note,
			 uint64_t outValue,
			 const uint8_t* pub,
			 size_t pub_len
    )
{
    struct kyk_tx* tx = NULL;
    struct kyk_txin* txin = NULL;
    struct kyk_txout* txout = NULL;
    struct kyk_buff* pbk_sc = NULL;
    int res = -1;

    check(cb_tx, "Failed to kyk_make_coinbase_tx: cb_tx is NULL");

    tx = calloc(1, sizeof(*tx));
    check(tx, "Failed to kyk_make_coinbase_tx: tx calloc failed");

    tx -> version = 1;
    tx -> vin_sz = 1;
    tx -> lock_time = 0;
    tx -> txin = calloc(tx -> vin_sz, sizeof(struct kyk_txin));
    check(tx -> txin, "Failed to kyk_make_coinbase_tx: tx -> txin calloc failed");
    
    tx -> vout_sz = 1;
    tx -> txout = calloc(tx -> vout_sz, sizeof(struct kyk_txout));
    check(tx -> txout, "Failed to kyk_make_coinbase_tx: tx -> txout calloc failed");
    
    txin = tx -> txin;
    txout = tx -> txout;

    memset(txin -> pre_txid, KYK_COINBASE_PRE_TX_BYTE, sizeof(txin -> pre_txid));
    txin -> pre_tx_inx = KYK_COINBASE_PRE_TX_INX;
    kyk_make_coinbase_sc(txin, note);
    
    txin -> seq_no = KYK_COINBASE_SEQ_NO;
    txout -> value = outValue;

    res = build_p2pkh_sc_from_pubkey(pub, pub_len, &pbk_sc);
    check(res == 0, "Failed to kyk_make_coinbase_tx: build_p2pkh_sc_from_pubkey error");

    txout -> sc_size = pbk_sc -> len;
    txout -> sc = calloc(txout -> sc_size, sizeof(*txout -> sc));
    check(txout -> sc, "Failed to kyk_make_coinbase_tx: calloc failed");
    memcpy(txout -> sc, pbk_sc -> base, txout -> sc_size);

    *cb_tx = tx;

    free_kyk_buff(pbk_sc);
    pbk_sc = NULL;

    return 0;

error:
    if(tx) kyk_free_tx(tx);
    if(pbk_sc) {
	free_kyk_buff(pbk_sc);
	pbk_sc = NULL;
    }
    return -1;

}

struct kyk_tx* kyk_create_tx(uint32_t version,
			     varint_t vin_sz,
			     varint_t vout_sz,
			     uint32_t lock_time
    )
{
    struct kyk_tx* tx = NULL;
    tx = calloc(1, sizeof(struct kyk_tx));
    check(tx, "Failed to kyk_create_tx: calloc failed");

    tx -> version = version;
    
    check(vin_sz >= 1, "Failed to kyk_create_tx: vin_sz should be greater than 1");
    tx -> vin_sz = vin_sz;
    tx -> txin = calloc(tx -> vin_sz, sizeof(struct kyk_txin));
    check(tx -> txin, "Failed to kyk_create_tx: calloc txin failed");

    check(vout_sz >= 1, "Failed to kyk_create_tx: vout_sz should be greater than 1");
    tx -> vout_sz = vout_sz;
    tx -> txout = calloc(tx -> vout_sz, sizeof(struct kyk_txout));
    check(tx -> txout, "Failed to kyk_create_tx: calloc txout failed");

    tx -> lock_time = lock_time;

    return tx;
    
error:

    return NULL;
}


int kyk_deseri_tx_list(struct kyk_tx* tx_list,
		       size_t tx_count,
		       const uint8_t* buf,
		       size_t* byte_num)
{
    struct kyk_tx* tx = NULL;
    size_t len = 0;
    const unsigned char* bufp = (const unsigned char*)buf;
    int res = -1;
    size_t i = 0;

    check(tx_list != NULL, "Failed to kyk_deseri_tx_list: tx_list is NULL");
    check(tx_count >= 1, "Failed to kyk_deseri_tx_list: tx_count is invalid");
    check(buf != NULL, "Failed to kyk_deseri_tx_list: buf is NULL");
    

    for(i = 0; i < tx_count; i++){
	tx = tx_list + i;
	res = kyk_deseri_tx(tx, bufp, &len);
	check(res == 0, "Failed to kyk_deseri_tx_list: kyk_deseri_tx failed");
	bufp += len;
    }

    *byte_num = bufp - buf;

    return 0;
    
error:

    return -1;
}

int kyk_deseri_tx(struct kyk_tx* tx,
		  const uint8_t* buf,
		  size_t* byte_num)
{
    size_t len = 0;
    unsigned char* bufp = (unsigned char*)buf;
    int res = -1;
    int arg_checked = 0;

    check(tx != NULL, "Failed to kyk_deseri_tx: tx is NULL");
    check(tx -> txin == NULL, "Failed to kyk_deseri_tx: tx -> txin is not NULL");
    check(tx -> txout == NULL, "Failed to kyk_deseri_tx: tx -> txout is not NULL");
    check(buf != NULL,  "Failed to kyk_deseri_tx: buf is NULL");
    arg_checked = 1;
    
    beej_unpack(bufp, "<L", &tx -> version);
    bufp += sizeof(tx -> version);

    len = kyk_unpack_varint(bufp, &tx -> vin_sz);
    check(len > 0, "Failed to kyk_deseri_tx: kyk_unpack_varint failed");
    bufp += len;
    
    tx -> txin = calloc(tx -> vin_sz, sizeof(struct kyk_txin));
    check(tx -> txin, "Failed to kyk_deseri_tx: calloc tx -> txin failed");
    
    res = kyk_deseri_txin_list(tx -> txin, tx -> vin_sz, bufp, &len);
    check(res == 0, "Failed to kyk_deseri_tx: kyk_deseri_txin_list failed");
    bufp += len;

    len = kyk_unpack_varint(bufp, &tx -> vout_sz);
    check(len > 0, "Failed to kyk_deseri_tx: kyk_unpack_varint failed");
    bufp += len;

    tx -> txout = calloc(tx -> vout_sz, sizeof(struct kyk_txout));
    check(tx -> txout, "Failed to kyk_deseri_tx: calloc tx -> txout failed");
    res = kyk_deseri_txout_list(tx -> txout, tx -> vout_sz, bufp, &len);
    bufp += len;

    beej_unpack(bufp, "<L", &tx -> lock_time);
    bufp += sizeof(tx -> lock_time);

    *byte_num = bufp - buf;

    return 0;

error:
    if(arg_checked){
	if(tx -> txin) {
	    kyk_free_txin(tx -> txin);
	    tx -> txin = NULL;
	}
	if(tx -> txout) {
	    kyk_free_txout(tx -> txout);
	    tx -> txout = NULL;
	}
    }
    return -1;
}


int kyk_deseri_txin_list(struct kyk_txin* txin_list,
			 size_t txin_count,
			 const uint8_t* buf,
			 size_t* byte_num)
{
    unsigned char* bufp = NULL;
    struct kyk_txin* txin = NULL;
    int res = -1;
    size_t i = 0;
    size_t len = 0;

    check(txin_list, "Failed to kyk_deseri_txin_list: txin_list is NULL");
    check(buf, "Failed to kyk_deseri_txin_list: buf is NULL");

    bufp = (unsigned char*) buf;

    for(i = 0; i < txin_count; i++){
	txin = txin_list + i;
	res = kyk_deseri_txin(txin, bufp, &len);
	check(res == 0, "Failed to kyk_deseri_txin_list: kyk_deseri_txin failed");
	bufp += len;
    }

    *byte_num = bufp - buf;

    return 0;

error:

    return -1;
    
}


int kyk_deseri_txin(struct kyk_txin* txin,
		    const uint8_t* buf,
		    size_t* byte_num)
{
    unsigned char* bufp = NULL;
    size_t len = 0;
    int arg_checked = 0;
    
    check(txin, "Failed to kyk_deseri_txin: txin is NULL");
    check(txin -> sc == NULL, "Failed to kyk_deseri_txin: txin -> sc is not NULL");
    check(buf, "Failed to kyk_deseri_txin: buf is NULL");
    arg_checked = 1;
    

    bufp = (unsigned char*)buf;

    kyk_reverse_pack_chars(txin -> pre_txid, bufp, sizeof(txin -> pre_txid));
    bufp += sizeof(txin -> pre_txid);

    beej_unpack(bufp, "<L", &txin -> pre_tx_inx);
    bufp += sizeof(txin -> pre_tx_inx);

    len = kyk_unpack_varint(bufp, &txin -> sc_size);
    bufp += len;

    txin -> sc = calloc(txin -> sc_size, sizeof(*txin -> sc));
    check(txin -> sc, "Failed to kyk_deseri_txin: txin -> sc calloc failed");
    memcpy(txin -> sc, bufp, txin -> sc_size);
    bufp += txin -> sc_size;

    beej_unpack(bufp, "<L", &txin -> seq_no);
    bufp += sizeof(txin -> seq_no);

    *byte_num = bufp - buf;

    return 0;
    
error:
    if(arg_checked){
	if(txin -> sc) {
	    free(txin -> sc);
	    txin -> sc = NULL;
	}
    }
    return -1;
}


int kyk_deseri_txout_list(struct kyk_txout* txout_list,
			  size_t txout_count,
			  const uint8_t* buf,
			  size_t* byte_num)
{
    const unsigned char* bufp = NULL;
    struct kyk_txout* txout = NULL;
    int res = -1;
    size_t i = 0;
    size_t len = 0;

    check(txout_list, "Failed to kyk_deseri_txout_list: txout_list is NULL");
    check(buf, "Failed to kyk_deseri_txout_list: buf is NULL");

    bufp = (const unsigned char*) buf;

    for(i = 0; i < txout_count; i++){
	txout = txout_list + i;
	res = kyk_deseri_txout(txout, bufp, &len);
	check(res == 0, "Failed to kyk_deseri_txout_list: kyk_deseri_txout failed");
	bufp += len;
    }

    *byte_num = bufp - buf;

    return 0;

error:

    return -1;
    
}

int kyk_deseri_txout(struct kyk_txout* txout,
		     const uint8_t* buf,
		     size_t* byte_num)
{

    unsigned char* bufp = NULL;
    size_t len = 0;
    int arg_checked = 0;

    check(txout, "Failed to kyk_deseri_txout: txout is NULL");
    check(txout -> sc == NULL, "Failed to kyk_deseri_txout: txout -> sc is not NULL");
    check(buf, "Failed to kyk_deseri_txout: buf is NULL");
    arg_checked = 1;

    bufp = (unsigned char*)buf;
    beej_unpack(bufp, "<Q", &txout -> value);
    bufp += sizeof(txout -> value);

    len = kyk_unpack_varint(bufp, &txout -> sc_size);
    bufp += len;

    txout -> sc = calloc(txout -> sc_size, sizeof(*txout -> sc));
    check(txout -> sc, "Failed to kyk_deseri_txout: txout -> sc calloc failed");
    memcpy(txout -> sc, bufp, txout -> sc_size);
    bufp += txout -> sc_size;

    *byte_num = bufp - buf;
    
    return 0;

error:
    if(arg_checked){
	if(txout -> sc) {
	    free(txout -> sc);
	    txout -> sc = NULL;
	}
    }
    return -1;
}

int kyk_free_utxo_chain(struct kyk_utxo_chain* utxo_chain)
{
    struct kyk_utxo* curr;
    
    if(utxo_chain){
	curr = utxo_chain -> hd;
	while(curr){
	    kyk_free_utxo(curr);
	    curr = curr -> next;
	}

	free(utxo_chain);
    }

    return 0;
    
}

int kyk_free_utxo(struct kyk_utxo* utxo)
{
    if(utxo){
	
	if(utxo -> btc_addr){
	    free(utxo -> btc_addr);
	    utxo -> btc_addr = NULL;
	}

	if(utxo -> sc){
	    free(utxo -> sc);
	    utxo -> sc = NULL;
	}
	
	free(utxo);
    }

    return 0;
}

int kyk_get_utxo_size(const struct kyk_utxo* utxo, size_t* utxo_size)
{
    size_t total = 0;
    check(utxo, "Failed to kyk_get_utxo_size: utxo is NULL");

    total += sizeof(utxo -> txid);
    total += sizeof(utxo -> blkhash);
    total += sizeof(utxo -> addr_len);
    total += utxo -> addr_len;
    total += sizeof(utxo -> outidx);
    total += sizeof(utxo -> value);
    total += get_varint_size(utxo -> sc_size);
    total += utxo -> sc_size;
    total += sizeof(utxo -> spent);

    *utxo_size = total;

    return 0;

error:

    return -1;
 }

int kyk_seri_utxo(uint8_t* buf, const struct kyk_utxo* utxo, size_t* check_num)
{
    uint8_t* bufp = NULL;
    size_t len = 0;
    size_t total = 0;

    check(buf, "Failed to kyk_seri_utxo: buf is NULL");
    check(utxo, "Failed to kyk_seri_utxo: utxo is NULL");
    check(utxo -> addr_len > 0, "Failed to kyk_seri_utxo: utxo -> addr_len is invalid");
    check(utxo -> sc_size > 0, "Failed to kyk_seri_utxo: utxo -> sc_size is invalid");    

    bufp = buf;

    memcpy(bufp, utxo -> txid, sizeof(utxo -> txid));
    len = sizeof(utxo -> txid);
    total += len;
    bufp += len;
    
    memcpy(bufp, utxo -> blkhash, sizeof(utxo -> blkhash));
    len = sizeof(utxo -> blkhash);
    total += len;
    bufp += len;

    *bufp = utxo -> addr_len;
    len = sizeof(utxo -> addr_len);
    total += len;
    bufp += len;

    memcpy(bufp, utxo -> btc_addr, utxo -> addr_len);
    len = utxo -> addr_len;
    total += len;
    bufp += len;

    len = beej_pack(bufp, "<L", utxo -> outidx);
    total += len;
    bufp += len;

    len = beej_pack(bufp, "<Q", utxo -> value);
    total += len;
    bufp += len;

    len = kyk_pack_varint(bufp, utxo -> sc_size);
    total += len;
    bufp += len;

    memcpy(bufp, utxo -> sc, utxo -> sc_size);
    len = utxo -> sc_size;
    total += len;
    bufp += len;

    *bufp = utxo -> spent;
    len = sizeof(utxo -> spent);
    total += len;
    bufp += len;

    if(check_num){
	*check_num = total;
    }

    return 0;

error:

    return -1;
}

int kyk_deseri_utxo(struct kyk_utxo** new_utxo, const uint8_t* buf, size_t* check_num)
{
    struct kyk_utxo* utxo = NULL;
    const uint8_t* bufp = NULL;
    size_t len = 0;
    size_t total = 0;

    check(new_utxo, "Failed to kyk_deseri_utxo: utxo is NULL");
    check(buf, "Failed to kyk_deseri_utxo: buf is NULL");

    bufp = buf;
    utxo = calloc(1, sizeof(*utxo));
    check(utxo, "Failed to kyk_deseri_utxo: utxo calloc failed");

    memcpy(utxo -> txid, bufp, sizeof(utxo -> txid));
    len = sizeof(utxo -> txid);
    total += len;
    bufp += len;

    memcpy(utxo -> blkhash, bufp, sizeof(utxo -> blkhash));
    len = sizeof(utxo -> blkhash);
    total += len;
    bufp += len;

    memcpy(&utxo -> addr_len, bufp, sizeof(utxo -> addr_len));
    len = sizeof(utxo -> addr_len);
    total += len;
    bufp += len;

    check(utxo -> addr_len > 0, "Failed to kyk_deseri_utxo: utxo -> add_len is invalid");
    utxo -> btc_addr = calloc(utxo -> addr_len, sizeof(*utxo -> btc_addr));
    check(utxo -> btc_addr, "Failed to kyk_deseri_utxo: utxo -> btc_addr calloc failed");
    memcpy(utxo -> btc_addr, bufp, utxo -> addr_len);
    len = utxo -> addr_len;
    total += len;
    bufp += len;

    beej_unpack(bufp, "<L", &utxo -> outidx);
    len = sizeof(utxo -> outidx);
    total += len;
    bufp += len;

    beej_unpack(bufp, "<Q", &utxo -> value);
    len = sizeof(utxo -> value);
    total += len;
    bufp += len;

    len = kyk_unpack_varint(bufp, &utxo -> sc_size);
    check(utxo -> sc_size > 0, "Failed to kyk_deseri_utxo: utxo -> sc_size is invalid");
    total += len;
    bufp += len;

    utxo -> sc = calloc(utxo -> sc_size, sizeof(*utxo -> sc));
    check(utxo -> sc, "Failed to kyk_deseri_utxo: utxo -> sc calloc failed");
    memcpy(utxo -> sc, bufp, utxo -> sc_size);
    len = utxo -> sc_size;
    total += len;
    bufp += len;

    memcpy(&utxo -> spent, bufp, sizeof(utxo -> spent));
    len = sizeof(utxo -> spent);
    total += len;
    bufp += len;

    *new_utxo = utxo;

    if(check_num){
	*check_num = total;
    }

    return 0;

error:
    if(utxo) kyk_free_utxo(utxo);
    return -1;
}

int kyk_init_utxo_chain(struct kyk_utxo_chain* utxo_chain)
{
    check(utxo_chain, "Failed to kyk_init_utxo_chain: utxo_chain is NULL");

    utxo_chain -> hd = NULL;
    utxo_chain -> tail = NULL;
    utxo_chain -> curr = NULL;
    utxo_chain -> len = 0;

    return 0;
    
error:
    return -1;
}

int kyk_deseri_utxo_chain(struct kyk_utxo_chain** new_utxo_chain,
			  const uint8_t* buf,
			  size_t count,
			  size_t* check_num)
{
    struct kyk_utxo_chain* utxo_chain = NULL;
    struct kyk_utxo* utxo = NULL;
    const uint8_t* bufp = NULL;
    size_t len = 0;
    size_t total = 0;
    size_t i = 0;
    int res = -1;

    check(new_utxo_chain, "Failed to kyk_deseri_utxo_chain: new_utxo_chain is NULL");
    check(buf, "Failed to kyk_deseri_utxo_chain: buf is NULL");

    utxo_chain = calloc(1, sizeof(*utxo_chain));
    check(utxo_chain, "Failed to kyk_deseri_utxo_chain: utxo_chain calloc failed");
    kyk_init_utxo_chain(utxo_chain);

    bufp = buf;

    for(i = 0; i < count; i++){
	res = kyk_deseri_utxo(&utxo, bufp, &len);
	check(res == 0, "Failed to kyk_deseri_utxo_chain: kyk_deseri_utxo failed");
	check(utxo, "Failed to kyk_deseri_utxo_chain: utxo is NULL");
	kyk_utxo_chain_append(utxo_chain, utxo);
	bufp += len;
	total += len;
    }

    *new_utxo_chain = utxo_chain;

    if(check_num){
	*check_num = total;
    }
    
    return 0;
    
error:

    if(utxo_chain) kyk_free_utxo_chain(utxo_chain);
    
    return -1;
}

int kyk_utxo_chain_append(struct kyk_utxo_chain* utxo_chain,
			  struct kyk_utxo* utxo)
{

    check(utxo_chain, "Failed to kyk_utxo_chain_append: utxo_chain is NULL");
    check(utxo, "Failed to kyk_utxo_chain_append: utxo is NULL");

    if(utxo_chain -> hd == NULL){
	utxo_chain -> hd = utxo;
	utxo_chain -> curr = utxo;
    }

    if(utxo_chain -> tail){
	check(utxo_chain -> tail -> next == NULL, "Failed to kyk_utxo_chain_append: invalid utxo_chain -> tail");
	utxo_chain -> tail -> next = utxo;
    }

    utxo_chain -> tail = utxo_chain -> tail -> next;

    utxo_chain -> len += 1;
    
    return 0;

error:

    return -1;
}
