
/* spi.c: PPM firmware serial peripheral interface source code.
 * Copyright (C) 2013  Bradley Worley  <geekysuavo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* include the main header. */
#include "main.h"

/* spi_init: initialize the serial peripheral interface to the ADC. */
void spi_init (void) {
  /* disable SPI interrupts. */
  SPCR &= ~(1 << SPIE);

  /* set the SPI clock rate to GCLK/8. */
  SPSR |= (1 << SPI2X);
  SPCR |= (1 << SPR0);
  SPCR &= ~(1 << SPR1);

  /* set SPI falling edges as leading edges, MISO falling-edge sampling. */
  SPCR |= (1 << CPOL);
  SPCR &= ~(1 << CPHA);

  /* set SPI data ordering to MSB first. */
  SPCR &= ~(1 << DORD);

  /* enable SPI in master mode. */
  SPCR |= ((1 << MSTR) | (1 << SPE));
}

/* spi_read_adc: transfers a 16-bit word from the AD7680 SPI ADC. */
uint16_t spi_read_adc (void) {
  /* declare a storage variable for the ADC value. */
  uint8_t adc_hi, adc_mid, adc_lo;
  uint16_t adc;

  /* assert the ADC /CS pin to begin a conversion. */
  gpio_adc_select ();

  /* shift the first byte from the ADC. */
  SPDR = 0;
  while (!(SPSR & (1 << SPIF)));
  adc_hi = SPDR;

  /* shift the second byte from the ADC. */
  SPDR = 0;
  while (!(SPSR & (1 << SPIF)));
  adc_mid = SPDR;

  /* shift the third byte from the ADC. */
  SPDR = 0;
  while (!(SPSR & (1 << SPIF)));
  adc_lo = SPDR;

  /* de-assert the ADC /CS pin to end the conversion. */
  gpio_adc_deselect ();

  /* build the final adc sample word. */
  adc = ((adc_hi << 12) | (adc_mid << 4) | (adc_lo >> 4));

  /* return the sampled value. */
  return adc;
}

/* spi_write_dac: transfers a 12-bit word to the AD5320 SPI DAC. */
void spi_write_dac (uint16_t value) {
  /* declare a loop counter variable. */
  int8_t i;

  /* assert the DAC /CS pin to begin a transmission. */
  gpio_dac_select ();

  /* shift four zeros to the device. */
  gpio_swspi_mosi_low ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();

  /* shift the output word to the device. */
  for (i = 11; i >= 0; i--) {
    /* shift the currently indexed bit. */
    gpio_swspi_mosi_low ();
    GPIO_SWSPI_PORT |= (((value >> i) & 0x01) << GPIO_SWSPI_MOSI);;
    gpio_swspi_clock ();
  }

  /* de-assert the DAC /CS pin to end the transmission. */
  gpio_dac_deselect ();
}

/* spi_write_dac_8: transfers a byte to the AD5320 SPI DAC. */
void spi_write_dac_8 (uint8_t value) {
  /* declare a loop counter variable. */
  int8_t i;

  /* assert the DAC /CS pin to begin a transmission. */
  gpio_dac_select ();

  /* shift four zeros to the device. */
  gpio_swspi_mosi_low ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();

  /* shift four more zeros to the device. */
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();
  gpio_swspi_clock ();

  /* shift the output word to the device. */
  for (i = 7; i >= 0; i--) {
    /* shift the currently indexed bit. */
    if (value & (1 << i)) {
      gpio_swspi_mosi_high ();
    }
    else {
      gpio_swspi_mosi_low ();
    }

    /* clock the bit. */
    gpio_swspi_clock ();
  }

  /* de-assert the DAC /CS pin to end the transmission. */
  gpio_dac_deselect ();
}
