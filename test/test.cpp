#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "message.h"
#include "uart.h"

using namespace std;
using namespace eLinux;
using namespace BBB;

uint8_t preamble_1[4] = {0xAA, 0xBB, 0xCC, 0xDD};


int main() {
	UART bus(UART::UART1, 9600);
	MessageBox<UART> msg(bus);
	Message_t packet;

	// const char* s[4] = {"Beaglebone Black", "trongphuongpro",
	// 					"codelungtung", "uart testing"};

	puts("Let's go!");

	while (1) {
		if (msg.isAvailable()) {
			msg.pop(packet);

			printf("[Address] %d\n", packet.address);

			printf("[Size] %d\n", packet.payloadSize);
		
			printf("[Payload] ");
			for (int i = 0; i < packet.payloadSize; i++) {
				printf("%02X", packet.payload[i]);
			}
			printf("\n----------------------\n");
		}

		// for (uint8_t i = 0; i < 4; i++) {
		// 	printf("\nSend %d bytes", strlen(s[i]));
		// 	msg.send(preamble_1, i, i, s[i], strlen(s[i]));
		// }
		usleep(50000); /**< very important */
	}
}


// int main() {
// 	UART bus(UART::UART1, 9600);
// 	MessageBox<UART> msg(bus);

// 	Message packet;
// 	packet.payload = new uint8_t[MESSAGE_MAX_PAYLOAD_SIZE];

// 	while (1) {
// 		if (msg.isAvailable()) {
// 			msg.pop(packet);

// 			printf("\n[Address] %d", packet.address);

// 			printf("\n[Size] %d", packet.payloadSize);
		
// 			printf("\n[Payload] ");
// 			for (int i = 0; i < packet.payloadSize; i++) {
// 				printf("%c", packet.payload[i]);
// 			}
// 		}
// 		sleep(5); /**< very important 
// 	}
//