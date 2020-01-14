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
uint8_t preamble_2[4] = {0xAB, 0xBC, 0xCD, 0xDE};


/*int main() {
	UART bus(UART::UART1, 9600);
	MessageBox<UART> msg(bus);

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
}*/


int main() {
	UART bus(UART::UART1, 9600);
	MessageBox<UART> msg(bus);

	Message packet;
	packet.payload = new uint8_t[MESSAGE_MAX_PAYLOAD_SIZE];

	while (1) {
		if (msg.isAvailable()) {
			msg.pop(packet);

			printf("\n[Address] %d", packet.address);

			printf("\n[Size] %d", packet.payloadSize);
		
			printf("\n[Payload] ");
			for (int i = 0; i < packet.payloadSize; i++) {
				printf("%c", packet.payload[i]);
			}
		}
		sleep(5); /**< very important */
	}
}