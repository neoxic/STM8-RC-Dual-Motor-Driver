/*
** Copyright (C) 2021 Arseny Vakhrushev <arseny.vakhrushev@me.com>
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

#pragma once

// #define INDEP // Independent mode
// #define DEBUG // Debug mode (UART_TX on pin D5 at 115200 baud)
// #define CLK_16MHZ // Use F_CPU=16Mhz instead of 8Mhz
// #define CLK_EXT // Use HSE (external) clock
#define PWM_MIN 10 // Minimum non-zero PWM duty cycle (%)
#define PWM_MAX 100 // Maximum non-zero PWM duty cycle (%)
#define PWM_DIV 0 // PWM frequency divider (0..15), F_PWM=F_CPU*(PWM_MAX-PWM_MIN)/(45000*2^PWM_DIV)
// #define PWM_INV // PWM active low
// #define FWD1_INV // Forward 1 active low
// #define FWD2_INV // Forward 2 active low
// #define REV1_INV // Reverse 1 active low
// #define REV2_INV // Reverse 2 active low
#define LED_INV // LED active low

// RC-to-RX channel mapping
#define IBUS_CH1 3
#define IBUS_CH2 4

// Output pins
#define FWD1_ODR PC_ODR
#define FWD1_DDR PC_DDR
#define FWD1_PIN 0x80 // C7
#define FWD2_ODR PC_ODR
#define FWD2_DDR PC_DDR
#define FWD2_PIN 0x10 // C4
#define REV1_ODR PD_ODR
#define REV1_DDR PD_DDR
#define REV1_PIN 0x04 // D2
#define REV2_ODR PA_ODR
#define REV2_DDR PA_DDR
#define REV2_PIN 0x08 // A3
#if defined STM8S003Fx || defined STM8S103Fx // 20-pin MCU
#define LED_ODR PB_ODR
#define LED_DDR PB_DDR
#define LED_PIN 0x20 // B5
#else
#define LED_ODR PE_ODR
#define LED_DDR PE_DDR
#define LED_PIN 0x20 // E5
#endif

// ------------------------------------------------------------------------- //

#define DISABLE_INTERRUPTS() __asm__("sim")
#define ENABLE_INTERRUPTS()  __asm__("rim")
#define WAIT_FOR_INTERRUPT() __asm__("wfi")

#define sfr(x) (*(volatile uint8_t *)(x))

#define PA_ODR sfr(0x5000)
#define PA_DDR sfr(0x5002)
#define PA_CR1 sfr(0x5003)
#define PB_ODR sfr(0x5005)
#define PB_DDR sfr(0x5007)
#define PB_CR1 sfr(0x5008)
#define PC_ODR sfr(0x500a)
#define PC_DDR sfr(0x500c)
#define PC_CR1 sfr(0x500d)
#define PD_ODR sfr(0x500f)
#define PD_DDR sfr(0x5011)
#define PD_CR1 sfr(0x5012)
#define PE_ODR sfr(0x5014)
#define PE_DDR sfr(0x5016)
#define PE_CR1 sfr(0x5017)
#define PF_ODR sfr(0x5019)
#define PF_DDR sfr(0x501b)
#define PF_CR1 sfr(0x501c)

#define CLK_ICKR     sfr(0x50c0)
#define CLK_SWR      sfr(0x50c4)
#define CLK_SWCR     sfr(0x50c5)
#define CLK_CKDIVR   sfr(0x50c6)
#define CLK_HSITRIMR sfr(0x50cc)

#define WWDG_CR sfr(0x50d1)

#define TIM1_CR1   sfr(0x5250)
#define TIM1_IER   sfr(0x5254)
#define TIM1_SR1   sfr(0x5255)
#define TIM1_EGR   sfr(0x5257)
#define TIM1_CCMR1 sfr(0x5258)
#define TIM1_CCMR2 sfr(0x5259)
#define TIM1_CCMR3 sfr(0x525a)
#define TIM1_CCMR4 sfr(0x525b)
#define TIM1_CCER1 sfr(0x525c)
#define TIM1_CCER2 sfr(0x525d)
#define TIM1_PSCRH sfr(0x5260)
#define TIM1_PSCRL sfr(0x5261)
#define TIM1_ARRH  sfr(0x5262)
#define TIM1_ARRL  sfr(0x5263)
#define TIM1_RCR   sfr(0x5264)
#define TIM1_CCR1H sfr(0x5265)
#define TIM1_CCR1L sfr(0x5266)
#define TIM1_CCR2H sfr(0x5267)
#define TIM1_CCR2L sfr(0x5268)
#define TIM1_CCR3H sfr(0x5269)
#define TIM1_CCR3L sfr(0x526a)
#define TIM1_CCR4H sfr(0x526b)
#define TIM1_CCR4L sfr(0x526c)

#define CFG_GCR sfr(0x7f60)

#define TIM1_UIRQ  11
#define TIM1_CCIRQ 12

#if defined STM8S003xx || defined STM8S103xx

#define UART_SR   sfr(0x5230)
#define UART_DR   sfr(0x5231)
#define UART_BRR1 sfr(0x5232)
#define UART_BRR2 sfr(0x5233)
#define UART_CR2  sfr(0x5235)

#define TIM2_CR1   sfr(0x5300)
#define TIM2_EGR   sfr(0x5306)
#define TIM2_CCMR1 sfr(0x5307)
#define TIM2_CCMR2 sfr(0x5308)
#define TIM2_CCER1 sfr(0x530a)
#define TIM2_PSCR  sfr(0x530e)
#define TIM2_ARRH  sfr(0x530f)
#define TIM2_ARRL  sfr(0x5310)
#define TIM2_CCR1H sfr(0x5311)
#define TIM2_CCR1L sfr(0x5312)
#define TIM2_CCR2H sfr(0x5313)
#define TIM2_CCR2L sfr(0x5314)

#define UART_RXIRQ 18

#elif defined STM8S005xx || defined STM8S105xx

#define PG_ODR sfr(0x501e)
#define PG_DDR sfr(0x5020)
#define PG_CR1 sfr(0x5021)
#define PH_ODR sfr(0x5023)
#define PH_DDR sfr(0x5025)
#define PH_CR1 sfr(0x5026)
#define PI_ODR sfr(0x5028)
#define PI_DDR sfr(0x502a)
#define PI_CR1 sfr(0x502b)

#define UART_SR   sfr(0x5240)
#define UART_DR   sfr(0x5241)
#define UART_BRR1 sfr(0x5242)
#define UART_BRR2 sfr(0x5243)
#define UART_CR2  sfr(0x5245)

#define TIM2_CR1   sfr(0x5300)
#define TIM2_EGR   sfr(0x5304)
#define TIM2_CCMR1 sfr(0x5305)
#define TIM2_CCMR2 sfr(0x5306)
#define TIM2_CCER1 sfr(0x5308)
#define TIM2_PSCR  sfr(0x530c)
#define TIM2_ARRH  sfr(0x530d)
#define TIM2_ARRL  sfr(0x530e)
#define TIM2_CCR1H sfr(0x530f)
#define TIM2_CCR1L sfr(0x5310)
#define TIM2_CCR2H sfr(0x5311)
#define TIM2_CCR2L sfr(0x5312)

#define UART_RXIRQ 21

#else
#error Unsupported MCU
#endif
