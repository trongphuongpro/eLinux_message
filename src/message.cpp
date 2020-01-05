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


#include <stdio.h>
#include <unistd.h>
#include "message.h"


using namespace std;
using namespace BBB;

namespace eLinux {


Message::Message(UART::PORT port, int baudrate, uint8_t datasize)
	: UART(port, baudrate, datasize) {

	this->callback[0] = parsePreamble;
	this->callback[1] = parseAddress;
	this->callback[2] = parseSize;
	this->callback[3] = parsePayload;
	this->callback[4] = parseChecksum;

	this->currentStep = parsingPreamble;

	crc32_init();
	onReceiveData(ISR);
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

	writeBuffer(this->txPacket.preamble, MESSAGE_PREAMBLE_SIZE);
	writeBuffer(this->txPacket.address, 2);
	write(this->txPacket.payloadSize);
	writeBuffer(this->txPacket.payload, this->txPacket.payloadSize);
	writeBuffer(&(this->txPacket.checksum), sizeof(crc32_t));

	usleep(500000); /**< pause minimum 500ms between packets */
}


void Message::setPreamble(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	this->validPreamble[0] = b1;
	this->validPreamble[1] = b2;
	this->validPreamble[2] = b3;
	this->validPreamble[3] = b4;
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
		this->txPacket.preamble[i] = preamble[i];
	}


	// ADDRESS
	this->txPacket.address[0] = destination;
	this->txPacket.address[1] = source;


	// PAYLOAD SIZE
	this->txPacket.payloadSize = (len > MESSAGE_MAX_PAYLOAD_SIZE) ? 
									MESSAGE_MAX_PAYLOAD_SIZE : len;


	// PAYLOAD 
	for (uint8_t i = 0; i < this->txPacket.payloadSize; i++) {
		this->txPacket.payload[i] = payload[i];
	}


	// CHECKSUM CRC32
	this->txPacket.checksum = crc32_compute(this->txPacket.payload, this->txPacket.payloadSize);
}


void Message::parsePreamble(void *packet) {
	
	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingPreamble) {
		static int counter;

		rxMessage->rxPacket.preamble[counter] = rxMessage->read();

		if (rxMessage->rxPacket.preamble[counter] == rxMessage->validPreamble[counter]) {
			counter++;
		}
		else {
			counter = 0;
		}

		// go to next currentStep if 4-byte preamble is read.
		if (counter == MESSAGE_PREAMBLE_SIZE) {
			counter = 0;
			rxMessage->currentStep = parsingAddress;
		}
	}
}


void Message::parseAddress(void *packet) {
	

	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingAddress) {
		static int counter;

		rxMessage->rxPacket.address[counter++] = rxMessage->read();

		// go to next currentStep if 2-byte address is read.
		if (counter == 2) {
			counter = 0;
			rxMessage->currentStep = parsingSize;
		}
	}
}


void Message::parseSize(void *packet) {
	
	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingSize) {
		rxMessage->rxPacket.payloadSize = rxMessage->read();
		rxMessage->currentStep = parsingPayload;
	}
}


void Message::parsePayload(void *packet) {
	
	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingPayload) {
		static int counter;

		rxMessage->rxPacket.payload[counter++] = rxMessage->read();

		if (counter == rxMessage->rxPacket.payloadSize || counter == MESSAGE_MAX_PAYLOAD_SIZE) {
			counter = 0;
			rxMessage->currentStep = parsingChecksum;
		}
	}
}


void Message::parseChecksum(void *packet) {
	
	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingChecksum) {
		static int counter;

		*((uint8_t*)(&(rxMessage->rxPacket.checksum))+counter) = rxMessage->read();
		counter++;

		if (counter == sizeof(crc32_t)) {
			counter = 0;

			rxMessage->rxPacket.error = rxMessage->verifyChecksum();
			rxMessage->currentStep = finish;
		}
	}
}


int Message::verifyChecksum() {
	crc32_t ret = crc32_compute(&rxPacket, sizeof(rxPacket.preamble) 
										+ sizeof(rxPacket.address) 
										+ sizeof(rxPacket.payloadSize)
										+ rxPacket.payloadSize);

	if (ret == this->rxPacket.checksum) {
		return 0;
	}
	else {
		return -1;
	}
}


void ISR(void* arg) {
	Message *msg = static_cast<Message*>(arg);

	if (msg->currentStep < Message::finish) {
		msg->callback[msg->currentStep](msg);
	}
}
} /* namespace eLinux */