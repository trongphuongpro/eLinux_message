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
#include "uart.h"

using namespace std;
using namespace BBB;

namespace eLinux {


template <class T>
Message<T>::Message(T& _device): device{_device} {

	this->callback[0] = parsePreamble;
	this->callback[1] = parseAddress;
	this->callback[2] = parseSize;
	this->callback[3] = parsePayload;
	this->callback[4] = parseChecksum;

	this->currentStep = parsingPreamble;

	this->device.onReceiveData(ISR);
}


template <class T>
Message<T>::~Message() {

}


template <class T>
void Message<T>::send(const void* preamble,
					uint8_t destination, 
					uint8_t source, 
					const void* payload, 
					uint8_t len)
{
	createPacket(preamble, destination, source, payload, len);

	this->device.writeBuffer(this->txPacket.preamble, MESSAGE_PREAMBLE_SIZE);
	this->device.writeBuffer(this->txPacket.address, 2);
	this->device.write(this->txPacket.payloadSize);
	this->device.writeBuffer(this->txPacket.payload, this->txPacket.payloadSize);
	this->device.writeBuffer(&(this->txPacket.checksum), sizeof(crc32_t));

	usleep(500000); /**< pause minimum 500ms between packets */
}


template <class T>
void Message<T>::setPreamble(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	this->validPreamble[0] = b1;
	this->validPreamble[1] = b2;
	this->validPreamble[2] = b3;
	this->validPreamble[3] = b4;
}


template <class T>
void Message<T>::createPacket(const void* _preamble,
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
	this->txPacket.checksum = crc32_concat(crc32_compute(&this->txPacket, 
											sizeof(this->txPacket.preamble) 
											+ sizeof(this->txPacket.address) 
											+ sizeof(this->txPacket.payloadSize)),
								this->txPacket.payload, this->txPacket.payloadSize);
}


template <class T>
void Message<T>::parsePreamble(void *packet) {
	
	Message* rxMessage = static_cast<Message*>(packet);

	if (rxMessage->currentStep == parsingPreamble) {
		static int counter;

		rxMessage->rxPacket.preamble[counter] = rxMessage->device.read();

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


template <class T>
void Message<T>::parseAddress(void *packet) {
	

	Message<T>* rxMessage = static_cast<Message<T>*>(packet);

	if (rxMessage->currentStep == parsingAddress) {
		static int counter;

		rxMessage->rxPacket.address[counter++] = rxMessage->device.read();

		// go to next currentStep if 2-byte address is read.
		if (counter == 2) {
			counter = 0;
			rxMessage->currentStep = parsingSize;
		}
	}
}


template <class T>
void Message<T>::parseSize(void *packet) {
	
	Message<T>* rxMessage = static_cast<Message<T>*>(packet);

	if (rxMessage->currentStep == parsingSize) {
		rxMessage->rxPacket.payloadSize = rxMessage->device.read();
		rxMessage->currentStep = parsingPayload;
	}
}


template <class T>
void Message<T>::parsePayload(void *packet) {
	
	Message<T>* rxMessage = static_cast<Message<T>*>(packet);

	if (rxMessage->currentStep == parsingPayload) {
		static int counter;

		rxMessage->rxPacket.payload[counter++] = rxMessage->device.read();

		if (counter == rxMessage->rxPacket.payloadSize || counter == MESSAGE_MAX_PAYLOAD_SIZE) {
			counter = 0;
			rxMessage->currentStep = parsingChecksum;
		}
	}
}


template <class T>
void Message<T>::parseChecksum(void *packet) {
	
	Message<T>* rxMessage = static_cast<Message<T>*>(packet);

	if (rxMessage->currentStep == parsingChecksum) {
		static int counter;

		((uint8_t*)&(rxMessage->rxPacket.checksum))[counter] = rxMessage->device.read();
		counter++;

		if (counter == sizeof(crc32_t)) {
			counter = 0;

			rxMessage->verifyChecksum();
			rxMessage->currentStep = finish;
		}
	}
}


template <class T>
int Message<T>::verifyChecksum() {
	crc32_t ret = crc32_concat(crc32_compute(&this->rxPacket, 
											sizeof(this->rxPacket.preamble) 
											+ sizeof(this->rxPacket.address) 
											+ sizeof(this->rxPacket.payloadSize)),
								this->rxPacket.payload, this->rxPacket.payloadSize);

	if (ret == this->rxPacket.checksum) {
		return 0;
	}
	else {
		return -1;
	}
}

} /* namespace eLinux */