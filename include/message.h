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

#include <queue>
#include "crc32.h"

/** 
 * @brief massage preamble size
 */	
#define MESSAGE_PREAMBLE_SIZE		4


/** 
 * @brief maximum payload size
 */		
#define MESSAGE_MAX_PAYLOAD_SIZE	32


/**
 * @brief namespace eLinux
 */
namespace eLinux {


typedef void (*CallbackType)(void*);


/** 
 * @brief enum contains code for each step of transmitting/receiving procedure
 */  
enum steps {parsingPreamble=0, /**< step 1: parse the preamble */
			parsingAddress, /**< step 2: receive destination and source address */
			parsingSize, /**< step 3: receive payload size */
			parsingPayload, /**< step 4: receive payload */
			parsingChecksum, /**< step 5: receive CRC-32 checksum */
			verifyingChecksum /**< step 6: finish receiving prodedure */
		}; /**< @brief variable contains current state of procedure */


/** 
 * @brief Struct containing message
 */
struct Message {
	uint8_t address; /**< @brief source address: 1 byte */
	uint8_t payloadSize; /**< @brief size of payload  */
	uint8_t *payload; /**< @brief array contains payload */
} __attribute__((packed));

typedef Message* Message_t;

typedef struct MessageFrame* MessageFrame_t;


/**
 * @brief class Message used for transmitting/receiving message packet
 */
template <class T>
class MessageBox {
public:

	/**
	 * @brief Constructor
	 * @param bus UART bus number;
	 * @param baudrate UART baudrate;
	 * @param bit data size: 5,6,7 or 8 bit
	 */
	MessageBox(T& device);

	/**
	 * @brief Destructor
	 */
	~MessageBox();


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
	 * @brief Set valid preamble (4 bytes) for incoming packet
	 *
	 * @param b1 first byte.
	 * @param b2 second byte.
	 * @param b3 third byte.
	 * @param b4 last byte.
	 * @return nothing.
	 */
	void setPreamble(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4);


	/**
	 * @brief Pop the oldest Message from Message Box
	 * @param message pointer to Message instance;
	 * @return 0: success, -1: failed.
	 */
	int pop(Message& message);


	/**
	 * @brief Pop the oldest Message from Message Box
	 * @param message Message instance;
	 * @return 0: success, -1: failed.
	 */
	int pop(Message_t message);


	/**
	 * @brief Check if new Message is available
	 * @return true/false.
	 */
	bool isAvailable();


private:

	void createFrame(const void* preamble,
						uint8_t destination, 
						uint8_t source, 
						const void* payload, 
						uint8_t len);

	/** 
	 * @brief Check the integrity of the data
	 * @return 0: OK, -1: Error
	 */
	int verifyChecksum();
	

	/**
	 * @brief Extract message from received frame.
	 * @param rxFrame received frame.
	 * @return pointer to new Message.
	 */
	Message_t extractMessage(MessageFrame_t rxFrame);


	/**
	 * @brief Clear FIFO buffer.
	 * @return nothing.
	 */
	void clear();

	static void parsePreamble(void *);
	static void parseAddress(void *);
	static void parseSize(void *);
	static void parsePayload(void *);
	static void parseChecksum(void *);

	T& device; /**< Physical layer device */

	MessageFrame_t rxFrame; /**< @brief frame for incoming message */
	MessageFrame_t txFrame; /**< @brief frame for outgoing message */

	std::queue<Message_t> FIFO; /**< FIFO buffer containing Messages */

	steps currentStep;

	uint8_t validPreamble[MESSAGE_PREAMBLE_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD};

	CallbackType callback[5];

	friend void ISR(void *arg);
};

void ISR(void* arg);

} /* namespace eLinux */

#endif /* __MESSAGE__ */