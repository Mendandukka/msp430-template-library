#ifndef __USCI_I2C_H
#define __USCI_I2C_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>
#include <usci.h>

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const bool MASTER = true,
	const long FREQUENCY = 100000>
struct USCI_I2C_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	volatile static int rx_tx_count;
	static uint8_t *rx_tx_buffer;

	static void init(void) {
		*USCI::CTL1 |= UCSWRST;
		*USCI::CTL0 = (MASTER ? UCMST : 0) | UCMODE_3 | UCSYNC;
		*USCI::BR0 = (CLOCK::frequency / FREQUENCY) & 0xff;
		*USCI::BR1 = (CLOCK::frequency / FREQUENCY) >> 8;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*USCI::CTL1 = UCSSEL_1;
		} else {
			*USCI::CTL1 = UCSSEL_2;
		}
		rx_tx_count = 0;
		USCI::enable_rx_tx_irq();
	}

	static void enable(void) {
	}

	static void disable(void) {
	}

	static void set_slave_addr(const uint8_t slave_addr) {
		*USCI::I2CSA = slave_addr;
	}

	static void stop(void) {
		*USCI::CTL1 |= UCTXSTP;
		while (*USCI::CTL1 & UCTXSTP);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void send(const uint8_t *data, int length, bool stop) {
		CLOCK::claim();
		*USCI::CTL1 |= UCTR | UCTXSTT;
		while (length--) {
			while (!USCI::tx_irq_pending());
			*USCI::TXBUF = *data++;
		}
		while (!USCI::tx_irq_pending());
		if (stop) {
			*USCI::CTL1 |= UCTXSTP;
		}
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void receive(uint8_t *data, int length) {
		CLOCK::claim();
		*USCI::CTL1 &= ~UCTR;
		*USCI::CTL1 |= UCTXSTT;
		while (!TIMEOUT::triggered() && (*USCI::CTL1 & UCTXSTT));
		while (length--) {
			if (length == 0) {
				*USCI::CTL1 |= UCTXSTP;
			}
			while (!USCI::rx_irq_pending());
			*data++ = *USCI::RXBUF;
		}
		while (*USCI::STAT & UCBBUSY);
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(const uint8_t *data, int length, bool write) {
		CLOCK::claim();
		if (write) {
			*USCI::CTL1 |= UCTR;
		} else {
			*USCI::CTL1 &= ~UCTR;
		}
		rx_tx_buffer = const_cast<uint8_t *>(data);
		rx_tx_count = length;
		*USCI::CTL1 |= UCTXSTT;
		if (!write && length == 1) {
			while (!TIMEOUT::triggered() && (*USCI::CTL1 & UCTXSTT));
			*USCI::CTL1 |= UCTXSTP;
		}

		while (!TIMEOUT::triggered() && rx_tx_count > 0) {
			enter_idle();
		}
		while (*USCI::STAT & UCBBUSY);
		CLOCK::release();
	}

	static bool handle_tx_irq(void) {
		bool resume = false;
		if (USCI::rx_irq_pending()) {
			uint8_t rx_data = *USCI::RXBUF;
			if (rx_tx_count) {
				if (rx_tx_count <= 2) {
					*USCI::CTL1 |= UCTXSTP;
					resume = true;
				}
				*rx_tx_buffer++ = rx_data;
				rx_tx_count--;
			}
		}
		if (USCI::tx_irq_pending()) {
			if (rx_tx_count) {
				*USCI::TXBUF = *rx_tx_buffer++;
				rx_tx_count--;
			} else {
				*USCI::CTL1 |= UCTXSTP;
				USCI::clear_tx_irq();
				resume = true;
			}
		}
		return resume;
	}

	static void handle_rx_irq(void) {
		if (*USCI::STAT & UCNACKIFG) {
			*USCI::CTL1 |= UCTXSTT;
			*USCI::STAT &= ~UCNACKIFG;
		}
	}
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const long FREQUENCY>
volatile int USCI_I2C_T<MODULE, INSTANCE, CLOCK, MASTER, FREQUENCY>::rx_tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const long FREQUENCY>
uint8_t *USCI_I2C_T<MODULE, INSTANCE, CLOCK, MASTER, FREQUENCY>::rx_tx_buffer;

#endif

