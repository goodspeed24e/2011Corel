

/**
 * @file simple_idct.h
 * simple idct header.
 */
 
void simple_idct_put(uint8_t *dest, int line_size, DCTELEM *block);
void simple_idct_add(uint8_t *dest, int line_size, DCTELEM *block);
void ff_simple_idct_mmx(int16_t *block);
void ff_simple_idct_add_mmx(uint8_t *dest, int line_size, int16_t *block);
void ff_simple_idct_put_mmx(uint8_t *dest, int line_size, int16_t *block);
void simple_idct(DCTELEM *block);

void simple_idct248_put(uint8_t *dest, int line_size, DCTELEM *block);

void simple_idct84_add(uint8_t *dest, int line_size, DCTELEM *block);
void simple_idct48_add(uint8_t *dest, int line_size, DCTELEM *block);
