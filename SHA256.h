#ifndef OMASHA256_H
#define OMASHA256_H
#include <cstdint>
void Sha256(uint8_t* in, uint8_t* out);
void Sha256_round(uint32_t* s, uint8_t* data);
void Sha256_round_padding(uint32_t* s);
void SWeird(uint32_t* s, const uint32_t* const pad);

#endif
