#include <iostream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "message.h"


using namespace std;
using namespace BBB;
using namespace eLinux;


uint8_t preamble_1[4] = {0xAA, 0xBB, 0xCC, 0xDD};
uint8_t preamble_2[4] = {0xAB, 0xBC, 0xCD, 0xDE};


int main() {
	Message msg(UART::UART1, 9600, 8);

	const char* s[4] = {"Beaglebone Black", "trongphuongpro",
						"codelungtung", "uart testing"};

	puts("Let's go!");

	while (1) {
		for (uint8_t i = 0; i < 4; i++) {
			printf("Send %d bytes\n", strlen(s[i]));
			msg.send(preamble_1, i, i, s[i], strlen(s[i]));
		}
		sleep(1);
	}
}


// int main() {
// 	Message msg(UART::UART1, 9600, 8);

// 	while (1) {
// 		if (msg.currentStep == Message::finish) {
// 			printf("[Preamble] ");
// 			for (int i = 0; i < 4; i++) {
// 				printf("%x", msg.rxPacket.preamble[i]);
// 			}
			
// 			printf("\n[Destination] %d", msg.rxPacket.address[0]);
// 			printf("\n[Source] %d", msg.rxPacket.address[1]);
// 			printf("\n[Size] %d", msg.rxPacket.payloadSize);
		
// 			printf("\n[Payload] ");
// 			for (int i = 0; i < msg.rxPacket.payloadSize; i++) {
// 				printf("%c", msg.rxPacket.payload[i]);
// 			}

// 			printf("\n[Checksum] %x", msg.rxPacket.checksum);

// 			printf("\n[Status] %s", (msg.rxPacket.error) ? "ERROR" : "OK");

// 			puts("\n-----------------------------\n");
			
// 			msg.currentStep = Message::parsingPreamble;
// 		}
// 		usleep(1); /**< very important */
// 	}
// }