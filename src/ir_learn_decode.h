#pragma once
#include <stdint.h>
#include <stddef.h>

/* One RMT symbol: two (level,duration) halves, mirrors rmt_symbol_word_t but
 * IDF-free so the flattener is host-testable. */
struct ir_edge_pair_t { uint16_t d0; uint8_t l0; uint16_t d1; uint8_t l1; };

enum ir_proto_t {
    IR_PROTO_RAW = 0,
    IR_PROTO_NEC,
    IR_PROTO_SAMSUNG,
    IR_PROTO_SONY,
    IR_PROTO_RC5,
};

struct ir_decoded_t {
    ir_proto_t proto;
    const char *proto_name;
    uint32_t   address;
    uint32_t   command;
    uint8_t    bits;
};

/* Flatten symbol pairs into an alternating mark/space duration array (µs).
 * Stops at the first zero duration (RMT end-of-frame) or when out_max is hit
 * (sets *truncated). Returns the number of durations written. */
uint16_t ir_symbols_to_us(const ir_edge_pair_t *syms, size_t nsyms,
                          uint16_t *out, uint16_t out_max, bool *truncated);

/* Attempt to decode raw pulse timings into known protocols.
 * Returns true if recognized, false if raw only. */
bool ir_decode_pulses(const uint16_t *timings, uint16_t count, ir_decoded_t *out);
