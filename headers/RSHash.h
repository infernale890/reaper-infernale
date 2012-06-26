#ifndef RSHASH_H
#define RSHASH_H

void BlockHash_Init();
void BlockHash_DeInit();
void BlockHash_1(uint8_t* p512bytes, uint8_t* final_hash);
void BlockHash_1_mine_V1(uint8_t *p512bytes, uint8_t* final_hash, uint8_t* results);
void BlockHash_1_mine_V2(uint8_t *p512bytes, uint8_t* final_hash, uint8_t* results);
void BlockHash_1_mine_V3(uint8_t *p512bytes, uint8_t* final_hash, uint8_t* results);

#endif
