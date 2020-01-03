/** 
 * @file message.cpp
 * @brief Implementations for message protocol
 *  
 * This C++ library is used to create Data Link Layer for existed Physical Layers,
 * such as UART, SPI, I2C,...
 *
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date January 2, 2020
 */


#include <istream>
#include <unistd.h>
#include "message.h"


using namespace std;
using namespace BBB;

namespace eLinux {


Message::Message(UART::PORT port, int baudrate, uint8_t datasize)
	: UART(port, baudrate, datasize) {

	crc32_init();
}


Message::~Message() {

}


void Message::send(const void* preamble,
					uint8_t destination, 
					uint8_t source, 
					const void* payload, 
					uint8_t len)
{
	createPacket(preamble, destination, source, payload, len);
	writeBuffer(preamble, MESSAGE_PREAMBLE_SIZE);
	writeBuffer(address, 2);
	write(payloadSize);
	writeBuffer(payload, payloadSize);
	writeBuffer(&checksum, sizeof(crc32_t));
	sleep(1);
}


void Message::createPacket(const void* _preamble,
							uint8_t destination, 
							uint8_t source, 
							const void* _payload, 
							uint8_t len) 
{
	uint8_t* preamble = (uint8_t*)_preamble;
	uint8_t* payload = (uint8_t*)_payload;

	// PREAMBLE
	for (uint8_t i = 0; i < MESSAGE_PREAMBLE_SIZE; i++) {
		this->preamble[i] = preamble[i];
	}


	// ADDRESS
	this->address[0] = destination;
	this->address[1] = source;


	// PAYLOAD SIZE
	this->payloadSize = (len > MESSAGE_MAX_PAYLOAD_SIZE) ? MESSAGE_MAX_PAYLOAD_SIZE : len;


	// PAYLOAD 
	for (uint8_t i = 0; i < this->payloadSize; i++) {
		this->payload[i] = payload[i];
	}


	// CHECKSUM CRC32
	crc32_t checksum = crc32_compute(this->payload, this->payloadSize);

	this->checksum = checksum;
}

}