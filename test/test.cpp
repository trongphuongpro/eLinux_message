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

	const char* s[4] = {"Hello atmega328, I'm Beaglebone Black", "trongphuongpro",
						"codelungtung", "uart testing"};


	while (1) {
		for (uint8_t i = 0; i < 4; i++) {
			msg.send(preamble_1, 0xAA, 0xBB, s[i], strlen(s[i]));
		}

		for (uint8_t i = 0; i < 4; i++) {
			msg.send(preamble_2, 0xAA, 0xBB, s[i], strlen(s[i]));
		}

		sleep(5);
	}
}