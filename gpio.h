#ifndef __GPIO_H
#define __GPIO_H

#include <msp430.h>

enum GPIO_DIRECTION {
	INPUT = 0,
	OUTPUT = 1
};

enum GPIO_INTERRUPT_EDGE {
	RISING = 0,
	FALLING = 1
};

static constexpr int ports[][9] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	{P1IN_, P1OUT_, P1DIR_, P1IFG_, P1IES_, P1IE_, P1SEL_, P1SEL2_, P1REN_},
#ifdef P2IN_
	{P2IN_, P2OUT_, P2DIR_, P2IFG_, P2IES_, P2IE_, P2SEL_, P2SEL2_, P2REN_},
#ifdef P3IN_
	{P3IN_, P3OUT_, P3DIR_, 0, 0, 0, P3SEL_, P3SEL2_, P3REN_},
#ifdef P4IN_
	{P4IN_, P4OUT_, P4DIR_, 0, 0, 0, P4SEL_, P4SEL2_, P4REN_},
#ifdef P5IN_
	{P5IN_, P5OUT_, P5DIR_, 0, 0, 0, P5SEL_, P5SEL2_, P5REN_},
#ifdef P6IN_
	{P6IN_, P6OUT_, P6DIR_, 0, 0, 0, P6SEL_, P6SEL2_, P6REN_},
#ifdef P7IN_
	{P7IN_, P7OUT_, P7DIR_, 0, 0, 0, P7SEL_, P7SEL2_, P7REN_},
#ifdef P8IN_
	{P8IN_, P8OUT_, P8DIR_, 0, 0, 0, P8SEL_, P8SEL2_, P8REN_}
#endif
#endif
#endif
#endif
#endif
#endif
#endif
};

template<const char PORT, const char PIN,
	const GPIO_DIRECTION DIRECTION,
	const bool INTERRUPT_ENABLE = false,
	const GPIO_INTERRUPT_EDGE INTERRUPT_EDGE = RISING,
	const char FUNCTION_SELECT = 0,
	const bool RESISTOR_ENABLE= false>
struct GPIO_PIN_T {
	static constexpr volatile unsigned char *PxIN = (unsigned char *) ports[PORT][0];
	static constexpr volatile unsigned char *PxOUT = (unsigned char *) ports[PORT][1];
	static constexpr volatile unsigned char *PxDIR = (unsigned char *) ports[PORT][2];
	static constexpr volatile unsigned char *PxIFG = (unsigned char *) ports[PORT][3];
	static constexpr volatile unsigned char *PxIES = (unsigned char *) ports[PORT][4];
	static constexpr volatile unsigned char *PxIE = (unsigned char *) ports[PORT][5];
	static constexpr volatile unsigned char *PxSEL = (unsigned char *) ports[PORT][6];
	static constexpr volatile unsigned char *PxSEL2 = (unsigned char *) ports[PORT][7];
	static constexpr volatile unsigned char *PxREN = (unsigned char *) ports[PORT][8];

	static constexpr unsigned char bit_value = 1 << PIN;

	static constexpr unsigned char direction = (DIRECTION == OUTPUT ? bit_value : 0);
	static constexpr unsigned char interrupt_enable = (INTERRUPT_ENABLE ? bit_value : 0);
	static constexpr unsigned char interrupt_edge =  (INTERRUPT_EDGE == FALLING ? bit_value : 0);
	static constexpr unsigned char function_select =  (FUNCTION_SELECT & 0b01 ? bit_value : 0);
	static constexpr unsigned char function_select2 =  (FUNCTION_SELECT & 0b10 ? bit_value : 0);
	static constexpr unsigned char resistor_enable =  (RESISTOR_ENABLE ? bit_value : 0);

	static void init(void) {
		if (DIRECTION == OUTPUT) *PxDIR |= bit_value;
		if (INTERRUPT_ENABLE) {
			static_assert(PxIE != 0, "Port not interrupt capable");
			*PxIE |= bit_value;
		}
		if (INTERRUPT_EDGE == FALLING) {
			static_assert(PxIES != 0, "Port not interrupt capable");
			*PxIES |= bit_value;
		}
		if (FUNCTION_SELECT & 0b01) {
			static_assert(PxSEL != 0, "Port does not have alternate functions");
			*PxSEL |= bit_value;
		}
		if (FUNCTION_SELECT & 0b10) {
			static_assert(PxSEL2 != 0, "Port does not have alternate functions");
			*PxSEL2 |= bit_value;
		}
		if (RESISTOR_ENABLE) *PxREN |= bit_value;
	};

	static void set_high(void) {
		*PxOUT |= bit_value;
	};

	static void set_low(void) {
		*PxOUT &= ~bit_value;
	}

	static bool get(void) {
		return *PxIN & bit_value;
	}

	static void toggle(void) {
		*PxOUT ^= bit_value;
	}
};

struct PIN_UNUSED {
	static constexpr unsigned char direction = 0;
	static constexpr unsigned char interrupt_enable = 0;
	static constexpr unsigned char interrupt_edge = 0;
	static constexpr unsigned char function_select = 0;
	static constexpr unsigned char function_select2 = 0;
	static constexpr unsigned char resistor_enable = 0;
};

template<const int PORT,
	typename PIN0 = PIN_UNUSED,
	typename PIN1 = PIN_UNUSED,
	typename PIN2 = PIN_UNUSED,
	typename PIN3 = PIN_UNUSED,
	typename PIN4 = PIN_UNUSED,
	typename PIN5 = PIN_UNUSED,
	typename PIN6 = PIN_UNUSED,
	typename PIN7 = PIN_UNUSED>
struct GPIO_PORT_T {
	static constexpr volatile unsigned char *PxIN = (unsigned char *) ports[PORT][0];
	static constexpr volatile unsigned char *PxOUT = (unsigned char *) ports[PORT][1];
	static constexpr volatile unsigned char *PxDIR = (unsigned char *) ports[PORT][2];
	static constexpr volatile unsigned char *PxIFG = (unsigned char *) ports[PORT][3];
	static constexpr volatile unsigned char *PxIES = (unsigned char *) ports[PORT][4];
	static constexpr volatile unsigned char *PxIE = (unsigned char *) ports[PORT][5];
	static constexpr volatile unsigned char *PxSEL = (unsigned char *) ports[PORT][6];
	static constexpr volatile unsigned char *PxSEL2 = (unsigned char *) ports[PORT][7];
	static constexpr volatile unsigned char *PxREN = (unsigned char *) ports[PORT][8];

	static void init(void) {
		*PxDIR =
			PIN0::direction | PIN1::direction | PIN2::direction | PIN3::direction |
			PIN6::direction | PIN5::direction | PIN6::direction | PIN7::direction;

		*PxIE =
			PIN0::interrupt_enable | PIN1::interrupt_enable | PIN2::interrupt_enable | PIN3::interrupt_enable |
			PIN6::interrupt_enable | PIN5::interrupt_enable | PIN6::interrupt_enable | PIN7::interrupt_enable;

		*PxSEL =
			PIN0::function_select | PIN1::function_select | PIN2::function_select | PIN3::function_select |
			PIN6::function_select | PIN5::function_select | PIN6::function_select | PIN7::function_select;

		*PxSEL2 =
			PIN0::function_select2 | PIN1::function_select2 | PIN2::function_select2 | PIN3::function_select2 |
			PIN6::function_select2 | PIN5::function_select2 | PIN6::function_select2 | PIN7::function_select2;

		*PxREN =
			PIN0::resistor_enable | PIN1::resistor_enable | PIN2::resistor_enable | PIN3::resistor_enable |
			PIN6::resistor_enable | PIN5::resistor_enable | PIN6::resistor_enable | PIN7::resistor_enable;
	}
};

#endif
