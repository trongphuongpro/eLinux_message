/** 
 * @file uart.cpp
 * @brief This file contains implementation for class UART - a wrapper for
 * Linux's UART C-library
 * @author Nguyen Trong Phuong (aka trongphuongpro)
 * @date December 29, 2019
 */


#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "uart.h"


using namespace std;

namespace BBB {

UART::UART(UART::PORT port, int baudrate, uint8_t datasize) {
	this->port = port;
	this->baudrate = baudrate;
	this->datasize = datasize;
	this->filename = UART_PATH + to_string(port);
	this->file = -1;

	this->threadRunning = false;
	this->callbackFunction = NULL;

	open();
}


UART::~UART() {
	if (this->file != -1)
		close();
}


int UART::open() {
	if ((this->file = ::open(this->filename.c_str(), O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		perror("UART: Failed to open the file.");
		return -1;
	}

	struct termios options;

	tcgetattr(this->file, &options);
	options.c_cflag = this->datasize | CREAD | CLOCAL;
	options.c_iflag = IGNPAR | ICRNL;

	options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

	cfsetispeed(&options, this->baudrate);
	cfsetospeed(&options, this->baudrate);

	tcflush(this->file, TCIFLUSH);
	tcsetattr(this->file, TCSANOW, &options);

	return 0;
}


void UART::close() {
	::close(this->file);
	this->file = -1;
}


int UART::send(uint8_t data) {
	int ret;

	if ((ret=::write(this->file, &data, 1)) < 0) {
		perror("UART: Failed to write to the output");
	}

	return ret;
}


int UART::sendBuffer(const void* buffer, uint32_t len) {
	int ret;

	if ((ret=::write(this->file, buffer, len)) < 0) {
		perror("UART: Failed to write to the output");
	}

	return ret;
}


int UART::receive() {
	uint8_t data;

	if (::read(this->file, &data, 1) < 0) {
		//perror("UART: Failed to read from the input");
		return -1;
	}

	return data;
}


int UART::receiveBuffer(void* buffer, uint32_t len) {
	int ret;

	if ((ret=::read(this->file, buffer, len)) < 0) {
		//perror("UART: Failed to read from the input");
	}

	return ret;
}


int UART::waitData() {
	int nr_events, epollfd;
	struct epoll_event event;

	epollfd = epoll_create1(EPOLL_CLOEXEC);

	if (epollfd < 0) {
		perror("UART: Failed to create epollfd");
		return -1;
	}

	event.events = EPOLLIN | EPOLLET | EPOLLPRI;
	event.data.fd = this->file;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->file, &event) == -1) {
		perror("UART: Failed to add control interface");
		::close(epollfd);

		return -1;
	}


	nr_events = epoll_wait(epollfd, &event, 1, -1);

	if (nr_events == -1) {
		perror("GPIO: Poll Wait fail");
		::close(epollfd);

		return -1;
	}

	::close(epollfd);

	return 0;
}


void *threadedPoll(void* arg) {
	UART *bus = static_cast<UART*>(arg);

	while (bus->threadRunning) {
		if (bus->waitData() == 0) {
			bus->callbackFunction(bus->callbackArgument);
		}
	}

	return 0;
}


void UART::onReceiveData(CallbackType callback, void *arg) {
	this->threadRunning = true;
	this->callbackFunction = callback;
	this->callbackArgument = arg;

	if (pthread_create(&this->thread,
						NULL,
						threadedPoll,
						this)) {

		perror("UART: Failed to create the poll thread");
		this->threadRunning = false;
	}
}

} /* namespace BBB */