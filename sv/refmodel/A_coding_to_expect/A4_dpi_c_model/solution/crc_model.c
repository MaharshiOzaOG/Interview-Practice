// crc_model.c : golden C model, one heap context per channel
#include <stdio.h>
#include <stdlib.h>
#include "svdpi.h"

#define CRC_POLY 0xEDB88320u

typedef struct {
    unsigned int crc;
    unsigned int bytes;
} crc_ctx_t;

extern void sv_log(const char* msg);     // exported from SV

void* crc_new(void) {
    crc_ctx_t* c = (crc_ctx_t*)malloc(sizeof(crc_ctx_t));
    c->crc   = 0xFFFFFFFFu;
    c->bytes = 0;
    return c;
}

void crc_free(void* h) {
    free(h);                             // C allocated it -> C frees it
}

void crc_update(void* h, const svOpenArrayHandle data) {
    crc_ctx_t* c = (crc_ctx_t*)h;
    int n = svSize(data, 1);
    for (int i = 0; i < n; i++) {
        unsigned char b = *(unsigned char*)svGetArrElemPtr1(data, i);
        c->crc ^= b;
        for (int k = 0; k < 8; k++)
            c->crc = (c->crc & 1u) ? (c->crc >> 1) ^ CRC_POLY : (c->crc >> 1);
    }
    c->bytes += n;
}

unsigned int crc_final(void* h) {
    crc_ctx_t* c = (crc_ctx_t*)h;
    unsigned int r = c->crc ^ 0xFFFFFFFFu;
    char msg[96];
    snprintf(msg, sizeof(msg), "C model: %u bytes -> crc=%08x", c->bytes, r);
    sv_log(msg);                         // calling SV => import must be 'context'
    c->crc   = 0xFFFFFFFFu;              // ready for next message
    c->bytes = 0;
    return r;
}
