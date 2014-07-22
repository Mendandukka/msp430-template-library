#ifndef __USCI_UART_H
#define __USCI_UART_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>
#include <usci.h>

struct USCI_UART_STATUS {
	bool framing_error: 1;
	bool rx_available: 1;
};

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const long SPEED = 9600>
struct USCI_UART_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	static constexpr uint8_t idle_mode(void) { return LPM0_bits; }
	static USCI_UART_STATUS status;
	static volatile int tx_count;
	static volatile int rx_count;
	static uint8_t rx_buffer;
	static uint8_t *tx_buffer;

	static constexpr uint16_t USCI_DIV_INT = CLOCK::frequency / SPEED;
	static constexpr uint16_t USCI_BR0_VAL = USCI_DIV_INT & 0x00FF;
	static constexpr uint16_t USCI_BR1_VAL = (USCI_DIV_INT >> 8) & 0xFF;

	static constexpr uint16_t USCI_DIV_FRAC_NUMERATOR = CLOCK::frequency - (USCI_DIV_INT * SPEED);
	static constexpr uint16_t USCI_DIV_FRAC_NUM_X_8 = USCI_DIV_FRAC_NUMERATOR * 8;
	static constexpr uint16_t USCI_DIV_FRAC_X_8 = USCI_DIV_FRAC_NUM_X_8 / SPEED;

	static constexpr uint16_t USCI_BRS_VAL =
		((USCI_DIV_FRAC_NUM_X_8 - (USCI_DIV_FRAC_X_8 * SPEED)) * 10) / SPEED < 5 ?
			USCI_DIV_FRAC_X_8 << 1 :
			(USCI_DIV_FRAC_X_8+1) << 1;

	static void init(void) {
		CLOCK::claim();
		*USCI::CTL1 |= UCSWRST;
		*USCI::BR0 = USCI_BR0_VAL;
		*USCI::BR1 = USCI_BR1_VAL;
		*USCI::MCTL = USCI_BRS_VAL;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*USCI::CTL1 = UCSSEL_1;
		} else {
			*USCI::CTL1 = UCSSEL_2;
		}
		*USCI::CTL1 &= ~UCSWRST;
		__bis_SR_register(GIE);
		USCI::enable_tx_irq();
		USCI::enable_rx_irq();
	}

	static void enable(void) {
		CLOCK::claim();
	}

	static void disable(void) {
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count) {
		tx_buffer = tx_data;
		tx_count = count;
		*USCI::TXBUF = *tx_buffer++;
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
	}

	static bool handle_tx_irq(void) {
		bool resume = false;

		if (IFG2 & (MODULE == USCI_A ? UCA0TXIFG : UCB0TXIFG)) {
			tx_count--;
			if (tx_count > 0) {
				*USCI::TXBUF = *tx_buffer++;
			} else {
				USCI::clear_tx_irq();
				resume = true;
			}
		}
		return resume;
	}

	static bool handle_rx_irq(void) {
		bool resume = false;

		if (IFG2 & (MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG)) {
			rx_buffer = *USCI::RXBUF;
			rx_count = 1;
			USCI::clear_rx_irq();
			resume = true;
		}
		return resume;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void putc(char data) {
		tx_count = 1;
		*USCI::TXBUF = data;
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void puts(const char *data) {
		unsigned n;

		for (n = 0; data[n] != '\0'; n++) ;
		transfer<TIMEOUT>((uint8_t *) data, n);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static char getc() {
		status.rx_available = false;
		rx_count = 0;
		while (!TIMEOUT::triggered() && rx_count == 0) {
			enter_idle();
		}
		if (!TIMEOUT::triggered()) status.rx_available = true;
		return rx_buffer;
	}
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
volatile int USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
volatile int USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
USCI_UART_STATUS USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::status;

#endif
