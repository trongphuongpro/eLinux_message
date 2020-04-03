/** 
 * @file message.cpp
 * @brief Implementations for MessageBox protocol
 *  
 * This C++ library is used to create Data Link Layer for existed Physical Layers,
 * such as UART, SPI, I2C,...
 *
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date January 2, 2020
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "message.h"

using namespace std;

namespace eLinux {


/** 
 * @brief Struct containing message frame
 */
struct MessageFrame_t {
	uint8_t preamble[MESSAGE_PREAMBLE_SIZE]; /**< @brief preamble of message frame */
	uint8_t address[2]; /**< @brief destination and source address: 2 bytes */
	uint8_t payloadSize; /**< @brief size of payload  */
	uint8_t payload[MESSAGE_MAX_PAYLOAD_SIZE]; /**< @brief array contains payload */
	crc32_t checksum; /**< @brief CRC-32 checksum */
} __attribute__((packed));


template <class T>
MessageBox<T>::MessageBox(T& _device): device{_device} {

	this->txFrame = new MessageFrame_t;
	this->rxFrame = new MessageFrame_t;

	this->callback[0] = parsePreamble;
	this->callback[1] = parseAddress;
	this->callback[2] = parseSize;
	this->callback[3] = parsePayload;
	this->callback[4] = parseChecksum;

	this->currentStep = kParsingPreamble;

	this->device.onReceiveData(ISR, this);
}


template <class T>
MessageBox<T>::~MessageBox() {
	clear();
	delete this->rxFrame;
	delete this->txFrame;
}


template <class T>
void MessageBox<T>::send(const void* preamble,
					uint8_t destination, 
					uint8_t source, 
					const void* payload, 
					uint8_t len)
{
	createFrame(preamble, destination, source, payload, len);

	this->device.sendBuffer(this->txFrame->preamble, MESSAGE_PREAMBLE_SIZE);
	this->device.sendBuffer(this->txFrame->address, 2);
	this->device.send(this->txFrame->payloadSize);
	this->device.sendBuffer(this->txFrame->payload, this->txFrame->payloadSize);
	this->device.sendBuffer(&(this->txFrame->checksum), sizeof(crc32_t));

	usleep(500000); /**< pause minimum 500ms between packets */
}


template <class T>
void MessageBox<T>::setPreamble(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
	this->validPreamble[0] = b1;
	this->validPreamble[1] = b2;
	this->validPreamble[2] = b3;
	this->validPreamble[3] = b4;
}


template <class T>
void MessageBox<T>::createFrame(const void* _preamble,
							uint8_t destination, 
							uint8_t source, 
							const void* _payload, 
							uint8_t len) 
{
	uint8_t* preamble = (uint8_t*)_preamble;
	uint8_t* payload = (uint8_t*)_payload;

	// PREAMBLE
	for (uint8_t i = 0; i < MESSAGE_PREAMBLE_SIZE; i++) {
		this->txFrame->preamble[i] = preamble[i];
	}


	// ADDRESS
	this->txFrame->address[0] = destination;
	this->txFrame->address[1] = source;


	// PAYLOAD SIZE
	this->txFrame->payloadSize = (len > MESSAGE_MAX_PAYLOAD_SIZE) ? 
									MESSAGE_MAX_PAYLOAD_SIZE : len;


	for (uint8_t i = 0; i < this->txFrame->payloadSize; i++) {
		this->txFrame->payload[i] = payload[i];
	}


	// CHECKSUM CRC32
	this->txFrame->checksum = crc32_concat(crc32_compute(this->txFrame, 
											sizeof(this->txFrame->preamble) 
											+ sizeof(this->txFrame->address) 
											+ sizeof(this->txFrame->payloadSize)),
								this->txFrame->payload, this->txFrame->payloadSize);
}


template <class T>
void MessageBox<T>::parsePreamble(void *packet) {
	MessageBox* rxMessage = static_cast<MessageBox*>(packet);

	if (rxMessage->currentStep == kParsingPreamble) {
		static int counter;

		rxMessage->rxFrame->preamble[counter] = rxMessage->device.receive();

		if (rxMessage->rxFrame->preamble[counter] == rxMessage->validPreamble[counter]) {
			counter++;
		}
		else {
			counter = 0;
		}

		// go to next currentStep if 4-byte preamble is read.
		if (counter == MESSAGE_PREAMBLE_SIZE) {
			counter = 0;
			rxMessage->currentStep = kParsingAddress;
		}
	}
}


template <class T>
void MessageBox<T>::parseAddress(void *packet) {
	MessageBox<T>* rxMessage = static_cast<MessageBox<T>*>(packet);

	if (rxMessage->currentStep == kParsingAddress) {
		static int counter;

		rxMessage->rxFrame->address[counter++] = rxMessage->device.receive();

		// go to next currentStep if 2-byte address is read.
		if (counter == 2) {
			counter = 0;
			rxMessage->currentStep = kParsingSize;
		}
	}
}


template <class T>
void MessageBox<T>::parseSize(void *packet) {
	MessageBox<T>* rxMessage = static_cast<MessageBox<T>*>(packet);

	if (rxMessage->currentStep == kParsingSize) {
		rxMessage->rxFrame->payloadSize = rxMessage->device.receive();

		if (rxMessage->rxFrame->payloadSize > MESSAGE_MAX_PAYLOAD_SIZE) {
			rxMessage->rxFrame->payloadSize = MESSAGE_MAX_PAYLOAD_SIZE;
		}

		rxMessage->currentStep = kParsingPayload;
	}
}


template <class T>
void MessageBox<T>::parsePayload(void *packet) {
	MessageBox<T>* rxMessage = static_cast<MessageBox<T>*>(packet);

	if (rxMessage->currentStep == kParsingPayload) {
		static int counter;

		rxMessage->rxFrame->payload[counter++] = rxMessage->device.receive();

		if (counter == rxMessage->rxFrame->payloadSize) {
			counter = 0;
			rxMessage->currentStep = kParsingChecksum;
		}
	}
}


template <class T>
void MessageBox<T>::parseChecksum(void *packet) {
	MessageBox<T>* rxMessage = static_cast<MessageBox<T>*>(packet);

	if (rxMessage->currentStep == kParsingChecksum) {
		static int counter;

		((uint8_t*)&rxMessage->rxFrame->checksum)[counter++] = rxMessage->device.receive();

		if (counter == sizeof(crc32_t)) {
			counter = 0;
			rxMessage->currentStep = kVerifyingChecksum;

			if (rxMessage->verifyChecksum() == 0) {
				rxMessage->FIFO.push(rxMessage->extractMessage(rxMessage->rxFrame));
			}

			rxMessage->currentStep = kParsingPreamble;
		}
	}
}


template <class T>
int MessageBox<T>::verifyChecksum() {
	crc32_t ret = crc32_concat(crc32_compute(this->rxFrame, 
											sizeof(this->rxFrame->preamble) 
											+ sizeof(this->rxFrame->address) 
											+ sizeof(this->rxFrame->payloadSize)),
								this->rxFrame->payload, this->rxFrame->payloadSize);

	if (ret == this->rxFrame->checksum) {
		return 0;
	}
	else {
		return -1;
	}
}


template <class T>
Message_t MessageBox<T>::extractMessage(MessageFrame_t *frame) {
	Message_t message;

	message.address = frame->address[1];
	message.payloadSize = frame->payloadSize;

	memcpy(message.payload, frame->payload, message.payloadSize);

	return message;
}


template <class T>
void MessageBox<T>::clear() {
	Message_t dump;

	while (!this->FIFO.empty()) {
		pop(dump);
	}
}


template <class T>
int MessageBox<T>::pop(Message_t &message) {
	int ret = -1;

	if (!this->FIFO.empty()) {
		Message_t &data = FIFO.front();

		message.address = data.address;
		message.payloadSize = data.payloadSize;
		memcpy(message.payload, data.payload, message.payloadSize);

		FIFO.pop();
	}

	return ret;
}


template <class T>
int MessageBox<T>::pop(Message_t *message) {
	int ret = -1;

	if (!this->FIFO.empty()) {
		Message_t &data = FIFO.front();

		message->address = data.address;
		message->payloadSize = data.payloadSize;
		memcpy(message->payload, data.payload, message->payloadSize);


		FIFO.pop();
	}

	return ret;
}


template <class T>
bool MessageBox<T>::isAvailable() {
	return !this->FIFO.empty();
}

} /* namespace eLinux */