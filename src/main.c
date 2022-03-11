/*
** Copyright (C) 2021-2022 Arseny Vakhrushev <arseny.vakhrushev@me.com>
**
** This firmware is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This firmware is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this firmware. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdio.h>
#include "config.h"

#define duty(x) ((450U * (x)) / (PWM_MAX - PWM_MIN))

static int16_t input(uint16_t t, uint16_t *u) {
	if (t < 800 || t > 2200) { // Invalid signal
		*u = 0;
		return 0;
	}
	if (!*u && (t < 1450 || t > 1550)) return 0; // Reset required
	*u = t;
	return t - 1500;
}

static uint16_t output(int16_t t, uint8_t *f, uint8_t *r) {
	*f = t > 50;
	*r = t < -50 ? t = -t, 1 : 0;
	if (t < 50) return 0;
	if (t > 500) return duty(PWM_MAX);
	return duty(PWM_MIN) + t - 50;
}

static uint16_t u1, u2, p1, p2;
static int16_t i1, i2;
static volatile uint8_t f1, f2, r1, r2; // 'volatile' facilitates SDCC's bres/bset optimization

static void update(void) {
#ifdef INDEP
	p1 = output(i1, &f1, &r1);
	p2 = output(i2, &f2, &r2);
#else
	p1 = output(i1 + i2, &f1, &r1);
	p2 = output(i1 - i2, &f2, &r2);
#endif
	TIM2_CCR1H = p1 >> 8;
	TIM2_CCR1L = p1;
	TIM2_CCR2H = p2 >> 8;
	TIM2_CCR2L = p2;
#ifdef FWD1_INV
	if (f1) FWD1_ODR &= ~FWD1_PIN;
	else FWD1_ODR |= FWD1_PIN;
#else
	if (f1) FWD1_ODR |= FWD1_PIN;
	else FWD1_ODR &= ~FWD1_PIN;
#endif
#ifdef FWD2_INV
	if (f2) FWD2_ODR &= ~FWD2_PIN;
	else FWD2_ODR |= FWD2_PIN;
#else
	if (f2) FWD2_ODR |= FWD2_PIN;
	else FWD2_ODR &= ~FWD2_PIN;
#endif
#ifdef REV1_INV
	if (r1) REV1_ODR &= ~REV1_PIN;
	else REV1_ODR |= REV1_PIN;
#else
	if (r1) REV1_ODR |= REV1_PIN;
	else REV1_ODR &= ~REV1_PIN;
#endif
#ifdef REV2_INV
	if (r2) REV2_ODR &= ~REV2_PIN;
	else REV2_ODR |= REV2_PIN;
#else
	if (r2) REV2_ODR |= REV2_PIN;
	else REV2_ODR &= ~REV2_PIN;
#endif
	WWDG_CR = 0xff; // Reset watchdog
}

void TIM1_CCIF(void) __interrupt(TIM1_CCIRQ) {
	uint8_t sr = TIM1_SR1;
	if (sr & 0x04) { // CC2IF=1
		uint16_t t1 = (TIM1_CCR1H << 8) | TIM1_CCR1L;
		uint16_t t2 = (TIM1_CCR2H << 8) | TIM1_CCR2L;
		i1 = input(t2 - t1, &u1);
#ifndef CLK_EXT
		static uint8_t n;
		if (n != 8 && (sr & 0x02)) { // CC1IF=1
			static uint16_t t;
			if (++n & 1) t = t1;
			else { // Use every other period between rising edges to automatically adjust HSI clock
				static int16_t q, x, y;
				int16_t p = t1 - t; // Period
				q += p - ((p + 500) / 1000) * 1000; // Cumulative error
				if (n == 8) {
					if (q > x) { // Slow down
						++CLK_HSITRIMR;
						y = -q;
						q = 0;
						n = 0;
					} else if (q < y) { // Speed up
						--CLK_HSITRIMR;
						x = -q;
						q = 0;
						n = 0;
					}
				}
			}
		}
#endif
	}
	if (sr & 0x10) { // CC4IF=1
		uint16_t t1 = (TIM1_CCR3H << 8) | TIM1_CCR3L;
		uint16_t t2 = (TIM1_CCR4H << 8) | TIM1_CCR4L;
		i2 = input(t2 - t1, &u2);
	}
	update();
}

void UART_RXNE(void) __interrupt(UART_RXIRQ) {
	static uint8_t a, b, n = 30;
	static uint16_t u;
	a = b;
	b = UART_DR;
	if (a == 0x20 && b == 0x40) { // Sync
		n = 0;
		u = 0xff9f;
		return;
	}
	if (n == 30 || ++n & 1) return;
	uint16_t v = a | (b << 8);
	if (n == 30) { // End of chunk
		if (u != v) return; // Sync lost
		update();
		// Disable RC channels to avoid conflict
		TIM1_CCER1 = 0x00;
		TIM1_CCER2 = 0x00;
		return;
	}
	uint16_t t = v & 0x0fff;
	switch (n >> 1) {
#ifdef IBUS_CH1
		case IBUS_CH1:
			i1 = input(t, &u1);
			break;
#endif
#ifdef IBUS_CH2
		case IBUS_CH2:
			i2 = input(t, &u2);
			break;
#endif
	}
	u -= a + b;
}

void TIM1_UIF(void) __interrupt(TIM1_UIRQ) {
	TIM1_SR1 = 0x00; // Clear interrupts
#ifdef DEBUG
	CFG_GCR = 0x00; // Resume main loop
#endif
	static uint8_t led;
	if (!led) led = (!!u1 + !!u2) * 2 + 1;
	else if (--led) LED_ODR ^= LED_PIN;
}

int putchar(int c) {
	while (!(UART_SR & 0x80)); // TXE=0 (TX in progress)
	UART_DR = c;
	return 0;
}

void main(void) {
#ifdef CLK_EXT // Automatic HSI->HSE clock switching
	CLK_SWCR = 0x02; // Enable switch
	CLK_SWR = 0xb4; // Enable HSE clock
	while (!(CLK_SWCR & 0x08)); // SWIF=0 (switch in progress)
	CLK_ICKR = 0x00; // Disable HSI clock
	CLK_SWCR = 0x00; // Disable switch
#else
#ifdef CLK_16MHZ
	CLK_CKDIVR = 0x00; // HSI/1=16Mhz clock
#else
	CLK_CKDIVR = 0x08; // HSI/2=8Mhz clock
#endif
#endif

#ifdef FWD1_INV
	FWD1_ODR |= FWD1_PIN;
#endif
	FWD1_DDR |= FWD1_PIN;
#ifdef FWD2_INV
	FWD2_ODR |= FWD2_PIN;
#endif
	FWD2_DDR |= FWD2_PIN;
#ifdef REV1_INV
	REV1_ODR |= REV1_PIN;
#endif
	REV1_DDR |= REV1_PIN;
#ifdef REV2_INV
	REV2_ODR |= REV2_PIN;
#endif
	REV2_DDR |= REV2_PIN;
#ifdef LED_INV
	LED_ODR |= LED_PIN;
#endif
	LED_DDR |= LED_PIN;
	PA_CR1 = 0xff;
	PB_CR1 = 0xff;
	PC_CR1 = 0xff;
	PD_CR1 = 0xff;
	PE_CR1 = 0xff;
	PF_CR1 = 0xff;
#ifdef PG_CR1
	PG_CR1 = 0xff;
	PH_CR1 = 0xff;
	PI_CR1 = 0xff;
#endif

	// RC channels 1,2
	TIM1_PSCRH = 0x00;
#ifdef CLK_16MHZ
	TIM1_PSCRL = 0x0f; // 1us capture resolution @ 16Mhz clock
#else
	TIM1_PSCRL = 0x07; // 1us capture resolution @ 8Mhz clock
#endif
	TIM1_ARRH = 0xff;
	TIM1_ARRL = 0xff;
	TIM1_RCR = 0x03; // Tick
	TIM1_EGR = 0x01; // UG=1 (force update)
	TIM1_CR1 = 0x01; // CEN=1 (enable counter)
	TIM1_CCMR1 = 0x91; // CC1S=01, IC1F=1001 (CC1 as input, IC1 on TI1, CLK/8 N=8 filter)
	TIM1_CCMR2 = 0x92; // CC2S=10, IC2F=1001 (CC2 as input, IC2 on TI1, CLK/8 N=8 filter)
	TIM1_CCMR3 = 0x91; // CC3S=01, IC3F=1001 (CC3 as input, IC3 on TI3, CLK/8 N=8 filter)
	TIM1_CCMR4 = 0x92; // CC4S=10, IC4F=1001 (CC4 as input, IC4 on TI3, CLK/8 N=8 filter)
	TIM1_CCER1 = 0x31; // CC1E=1, CC2E=1, CC2P=1 (enable IC1 on rising edge, IC2 on falling edge)
	TIM1_CCER2 = 0x31; // CC3E=1, CC4E=1, CC4P=1 (enable IC3 on rising edge, IC4 on falling edge)
	TIM1_IER = 0x15; // UIE=1, CC2IE=1, CC4IE=1 (enable interrupts)

	// PWM channels 1,2
	TIM2_PSCR = PWM_DIV & 0x0f;
	TIM2_ARRH = (duty(100) - 1) >> 8;
	TIM2_ARRL = (duty(100) - 1) & 0xff;
	TIM2_EGR = 0x01; // UG=1 (force update)
	TIM2_CR1 = 0x01; // CEN=1 (enable counter)
	TIM2_CCMR1 = 0x68; // CC1S=00, OC1PE=1, OC1M=110 (CC1 as output, buffered CCR1, PWM mode 1)
	TIM2_CCMR2 = 0x68; // CC2S=00, OC2PE=1, OC2M=110 (CC2 as output, buffered CCR2, PWM mode 1)
#ifdef PWM_INV
	TIM2_CCER1 = 0x33; // CC1E=1, CC1P=1, CC2E=1, CC2P=1 (enable OC1 active low, OC2 active low)
#else
	TIM2_CCER1 = 0x11; // CC1E=1, CC2E=1 (enable OC1 active high, OC2 active high)
#endif

#ifdef CLK_16MHZ
	UART_BRR2 = 0x0b;
	UART_BRR1 = 0x08; // 115200 baud @ 16Mhz clock
#else
	UART_BRR2 = 0x05;
	UART_BRR1 = 0x04; // 115200 baud @ 8Mhz clock
#endif
	UART_CR2 = 0x2c; // REN=1, TEN=1, RIEN=1 (enable RX/TX/interrupts)

#ifdef DEBUG
	printf("\n");
	printf(" CH1  CH2     IN1  IN2    PWM1 PWM2    FWD  REV\n");
#endif
	for (;;) {
		CFG_GCR = 0x02; // AL=1 (suspend main loop)
		WAIT_FOR_INTERRUPT();
#ifdef DEBUG
		DISABLE_INTERRUPTS();
		uint16_t _u1 = u1, _u2 = u2, _p1 = p1, _p2 = p2;
		int16_t _i1 = i1, _i2 = i2;
		ENABLE_INTERRUPTS();
		printf("%4u %4u    %4d %4d    %4d %4d    %d %d  %d %d\n",
			_u1, _u2, _i1, _i2, _p1, _p2, f1, f2, r1, r2);
#endif
	}
}
