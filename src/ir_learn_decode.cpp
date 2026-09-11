#include "ir_learn_decode.h"
#include <string.h>

uint16_t ir_symbols_to_us(const ir_edge_pair_t *syms, size_t nsyms,
                          uint16_t *out, uint16_t out_max, bool *truncated) {
    if (truncated) *truncated = false;
    uint16_t n = 0;
    for (size_t i = 0; i < nsyms; ++i) {
        if (syms[i].d0 == 0) return n;                 /* end-of-frame */
        if (n >= out_max) { if (truncated) *truncated = true; return n; }
        out[n++] = syms[i].d0;
        if (syms[i].d1 == 0) return n;                 /* end-of-frame */
        if (n >= out_max) { if (truncated) *truncated = true; return n; }
        out[n++] = syms[i].d1;
    }
    return n;
}

static inline bool in_range(uint16_t val, uint16_t expected, uint16_t tol) {
    return (val >= expected - tol) && (val <= expected + tol);
}

bool ir_decode_pulses(const uint16_t *timings, uint16_t count, ir_decoded_t *out) {
    if (!out) return false;
    memset(out, 0, sizeof(ir_decoded_t));
    out->proto = IR_PROTO_RAW;
    out->proto_name = "RAW";

    if (!timings || count < 20) return false;

    /* 1. Try NEC (9000us mark + 4500us space, ~67 pulses total) */
    if (in_range(timings[0], 9000, 1500) && in_range(timings[1], 4500, 1000) && count >= 66) {
        uint32_t data = 0;
        bool ok = true;
        for (int i = 0; i < 32; ++i) {
            uint16_t mark = timings[2 + i * 2];
            uint16_t space = timings[3 + i * 2];
            if (!in_range(mark, 560, 250)) { ok = false; break; }
            if (in_range(space, 1690, 450)) {
                data |= (1UL << i);
            } else if (in_range(space, 560, 250)) {
                /* 0 bit */
            } else {
                ok = false; break;
            }
        }
        if (ok) {
            out->proto = IR_PROTO_NEC;
            out->proto_name = "NEC";
            out->bits = 32;
            out->address = data & 0xFFFF;
            out->command = (data >> 16) & 0xFF;
            return true;
        }
    }

    /* 2. Try Samsung (4500us mark + 4500us space, 32 bits) */
    if (in_range(timings[0], 4500, 1000) && in_range(timings[1], 4500, 1000) && count >= 66) {
        uint32_t data = 0;
        bool ok = true;
        for (int i = 0; i < 32; ++i) {
            uint16_t mark = timings[2 + i * 2];
            uint16_t space = timings[3 + i * 2];
            if (!in_range(mark, 560, 250)) { ok = false; break; }
            if (in_range(space, 1690, 450)) {
                data |= (1UL << i);
            } else if (in_range(space, 560, 250)) {
                /* 0 bit */
            } else {
                ok = false; break;
            }
        }
        if (ok) {
            out->proto = IR_PROTO_SAMSUNG;
            out->proto_name = "SAMSUNG";
            out->bits = 32;
            out->address = data & 0xFFFF;
            out->command = (data >> 16) & 0xFF;
            return true;
        }
    }

    /* 3. Try Sony SIRC (2400us mark + 600us space, 12, 15, or 20 bits) */
    if (in_range(timings[0], 2400, 500) && in_range(timings[1], 600, 250)) {
        uint32_t data = 0;
        int nbits = (count - 2) / 2;
        if (nbits >= 12) {
            if (nbits > 20) nbits = 20;
            bool ok = true;
            for (int i = 0; i < nbits; ++i) {
                uint16_t mark = timings[2 + i * 2];
                if (in_range(mark, 1200, 300)) {
                    data |= (1UL << i);
                } else if (in_range(mark, 600, 250)) {
                    /* 0 bit */
                } else {
                    ok = false; break;
                }
            }
            if (ok) {
                out->proto = IR_PROTO_SONY;
                out->proto_name = "SONY SIRC";
                out->bits = (uint8_t)nbits;
                out->command = data & 0x7F;       /* 7-bit command */
                out->address = (data >> 7);       /* 5/8/13 bit address */
                return true;
            }
        }
    }

    return false;
}
