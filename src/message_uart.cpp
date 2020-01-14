/** 
 * @file message_uart.cpp
 * @brief Implementations for message protocol using UART.
 *  
 * This file is used to create Data Link Layer for UART Physical Layer.
 *
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date January 13, 2020
 */


#include <iostream>
#include "message.h"
#include "message.cpp"
#include "uart.h"

using namespace std;
using namespace BBB;

namespace eLinux {

template class MessageBox<UART>;

void ISR(void* arg) {
	MessageBox<UART> *msg = static_cast<MessageBox<UART>*>(arg);

	if (msg->currentStep < verifyingChecksum) {
		msg->callback[msg->currentStep](msg);
	}
}

} /* namespace eLinux */