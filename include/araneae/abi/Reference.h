#ifndef ARANEAE_ABI_REFERENCE_H
#define ARANEAE_ABI_REFERENCE_H

// clang-format off
/*
 * Universal encoding of rerefence (64 bit).
 *    [63:56] - Type (8 bits)
 *    [55:40] - Flags (16 bits):
 *        bit 0:    F_IMM       -- payload is immediate
 *        bit 1:    F_OFF_STACK -- payload is offset in the heap from HP (HP + Payload = desired address)
 *        bit 2:    F_OFF_HEAP  -- payload is offset in the stack from SP (SP + Payload = desired address) 
 *        bit 3:    F_GLOBAL    -- payload is global address in the data section 
 *        bit 4-16: reserved for the future use 
 *    [39:32] - Reserved for the future use (8 bits)
 *    [31:0]  - Payload (32 bits)
 */
// clang-format on
#define F_IMM 0b0000000000000001
#define F_OFF_STACK 0b0000000000000010
#define F_OFF_HEAP 0b0000000000000100
#define F_GLOBAL 0b0000000000001000

#define MakeRefInternal(cast, type, flags, payload)                            \
  (cast 0 | ((cast type & 0xFF) << 56) | ((cast flags & 0xFFFF) << 40) |       \
   (cast payload & 0xFFFFFFFF))
#define GetPayload(ref) (ref & 0x00000000FFFFFFFF)
#define GetType(ref) ((ref >> 56) & 0xFF)
#define ChangeTypeTo(ref, type)                                                \
  ((ref &                                                                      \
    0b0000000011111111111111111111111111111111111111111111111111111111) |      \
   (type << 56))
#define IsImmediate(ref) ((ref >> 40) & 0xFFFF & F_IMM)
#define IsStackOffset(ref) ((ref >> 40) & 0xFFFF & F_OFF_STACK)
#define IsHeapOffset(ref) ((ref >> 40) & 0xFFFF & F_OFF_HEAP)
#define IsGlobalOffset(ref) ((ref >> 40) & 0xFFFF & F_GLOBAL)
#define IsMemory(ref)                                                          \
  (IsStackOffset(ref) || IsHeapOffset(ref) || IsGlobalOffset(ref))

#define ReferenceByteSize 8

#endif /* ARANEAE_ABI_REFERENCE_H */
