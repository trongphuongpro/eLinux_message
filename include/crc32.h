/** 
 * @file crc32.h
 * @brief Function prototypes for computing CRC-32 checksum
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date 2019 Dec 28
 */


#ifndef __CRC32__
#define __CRC32__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * @brief datatype for CRC-32 checksum value
 */
typedef uint32_t crc32_t;


/** 
 * @brief compute CRC-32 checksum value for a byte array.
 * @param data pointer to an array;
 * @param len the length of data in byte.
 * @return CRC-32 checksum value.
 */
crc32_t crc32_compute(const void* data, uint32_t len);


/** 
 * @brief compute CRC-32 checksum value for 2 separated data arrays.
 * @param checksum existing checksum value.
 * @param data pointer to new data array.
 * @param len the length of new data in byte.
 * @return CRC-32 checksum value.
 */
crc32_t crc32_concat(crc32_t checksum, const void* data, uint32_t len);


/** 
 * @brief check the accuracy of computed CRC-32 checksum value.
 * @param data pointer to an array;
 * @param len the length of data in byte;
 * @param checksum computed CRC-32 checksum value.
 * @return 0: OK;
 * @return -1: Error.
 */
int crc32_selfcheck(const void* data, uint32_t len, crc32_t checksum);


/** 
 * @brief check the integrity of data.
 * @param data pointer to an array;
 * @param len the length of data in byte;
 * @return 0: OK;
 * @return -1: Error.
 */
int crc32_check(const void* data, uint32_t len);


/** 
 * @brief reverse one byte data.
 *
 * example: 0b10100011 --> 0b11000101
 * @param data one byte data.
 * @return reverseed data byte.
 */
uint8_t reverse(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* __CRC32__ */