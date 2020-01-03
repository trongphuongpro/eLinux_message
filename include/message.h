/** 
 * @file message.h
 * @brief Class for message protocol
 *  
 * This C++ library is used to create Data Link Layer for existed Physical Layers,
 * such as UART, SPI, I2C,...
 * This file contains the class and extern variables
 * that you will need, in order to use this library.
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date January 2, 2020
 */


#ifndef __MESSAGE__
#define __MESSAGE__

#include "crc32.h"
#include "uart.h"

/** 
 * @brief massage preamble size
 */	
#define MESSAGE_PREAMBLE_SIZE		4


/** 
 * @brief maximum payload size
 */		
#define MESSAGE_MAX_PAYLOAD_SIZE	100


/**
 * @brief namespace eLinux
 */
namespace eLinux {


/**
 * @brief class Message used for transmitting/receiving message packet
 */
class Message: public BBB::UART {
public:

	/** 
	 * @brief enum contains code for each step of transmitting/receiving procedure
	 */  
	enum steps {parsingPreamble=0, /**< step 1: parse the preamble */
				parsingAddress, /**< step 2: receive destination and source address */
				parsingSize, /**< step 3: receive payload size */
				parsingPayload, /**< step 4: receive payload */
				parsingChecksum, /**< step 5: receive CRC-32 checksum */
				finish /**< step 6: finish receiving prodedure */
			} currentStep; /**< @brief variable contains current state of procedure */;


	/**
	 * @brief Constructor
	 * @param bus UART bus number;
	 * @param baudrate UART baudrate;
	 * @param bit data size: 5,6,7 or 8 bit
	 */
	Message(BBB::UART::PORT port, int baudrate, uint8_t datasize);

	/**
	 * Destructor
	 */
	~Message();


	/** 
	 * @brief Send message packet
	 *
	 * Assemble message packet and transmit.
	 * @param [in] baudrate UART baudrate.
	 * @param [in] destination Receiver's address.
	 * @param [in] source Transmitter's address.
	 * @param [in] payload message need to be sent.
	 * @param [in] len length of message. 
	 * @return nothing.
	 */
	void send(const void* preamble,
				uint8_t destination, 
				uint8_t source, 
				const void* payload, 
				uint8_t len);


	/** 
	 * @brief Check the integrity of the data
	 * @return 0: OK, -1: Error
	 */
	int verifyChecksum();

	void setPreamble(void* preamble, uint8_t size);

	uint8_t preamble[MESSAGE_PREAMBLE_SIZE]; /**< @brief preamble of message packet */
	uint8_t address[2]; /**< @brief destination and source address: 2 bytes */
	uint8_t payloadSize; /**< @brief size of payload  */
	uint8_t payload[MESSAGE_MAX_PAYLOAD_SIZE]; /**< @brief array contains payload */
	crc32_t checksum; /**< @brief CRC-32 checksum */

private:

	void createPacket(const void* preamble,
						uint8_t destination, 
						uint8_t source, 
						const void* payload, 
						uint8_t len);

};

} /* namespace eLinux */

#endif /* __MESSAGE__ */