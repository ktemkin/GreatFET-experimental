/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2015 Dominic Spill <dominicgs@gmail.com>
 *
 * This file is part of GreatFET.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stddef.h>

#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "usb_descriptor.h"
#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_api_board_info.h"
#include "usb_api_spiflash.h"
#include "usb_bulk_buffer.h"

static const usb_request_handler_fn vendor_request_handler[] = {
	NULL,
	usb_vendor_request_erase_spiflash,
	usb_vendor_request_write_spiflash,
	usb_vendor_request_read_spiflash,
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_read_partid_serialno,
};

static const uint32_t vendor_request_handler_count =
	sizeof(vendor_request_handler) / sizeof(vendor_request_handler[0]);

usb_request_status_t usb_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage
) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;
	
	if( endpoint->setup.request < vendor_request_handler_count ) {
		usb_request_handler_fn handler = vendor_request_handler[endpoint->setup.request];
		if( handler ) {
			status = handler(endpoint, stage);
		}
	}
	
	return status;
}

const usb_request_handlers_t usb_request_handlers = {
	.standard = usb_standard_request,
	.class = 0,
	.vendor = usb_vendor_request,
	.reserved = 0,
};

void usb_configuration_changed(
	usb_device_t* const device
) {
	if( device->configuration->number == 1 ) {
		// transceiver configuration
		cpu_clock_pll1_max_speed();
		led_on(LED1);
	} else {
		/* Configuration number equal 0 means usb bus reset. */
		cpu_clock_pll1_low_speed();
		led_off(LED1);
	}
}

void usb_set_descriptor_by_serial_number(void)
{
	iap_cmd_res_t iap_cmd_res;
	
	/* Read IAP Serial Number Identification */
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);
	
	if (iap_cmd_res.status_res.status_ret == CMD_SUCCESS) {
		usb_descriptor_string_serial_number[0] = USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		
		/* 32 characters of serial number, convert to UTF-16LE */
		for (size_t i=0; i<USB_DESCRIPTOR_STRING_SERIAL_LEN; i++) {
			const uint_fast8_t nibble = (iap_cmd_res.status_res.iap_result[i >> 3] >> (28 - (i & 7) * 4)) & 0xf;
			const char c = (nibble > 9) ? ('a' + nibble - 10) : ('0' + nibble);
			usb_descriptor_string_serial_number[2 + i * 2] = c;
			usb_descriptor_string_serial_number[3 + i * 2] = 0x00;
		}
	} else {
		usb_descriptor_string_serial_number[0] = 2;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
	}
}

void debug_led(uint8_t val) {
	if(val & 0x1)
		led_on(LED1);
	else
		led_off(LED1);
		
	if(val & 0x2)
		led_on(LED2);
	else
		led_off(LED2);
		
	if(val & 0x4)
		led_on(LED3);
	else
		led_off(LED3);
		
	if(val & 0x8)
		led_on(LED4);
	else
		led_off(LED4);
}

int main(void) {
	pin_setup();
	debug_led(1);
	//enable_1v8_power();
	cpu_clock_init();
	debug_led(2);

	usb_set_descriptor_by_serial_number();
	debug_led(3);

	usb_set_configuration_changed_cb(usb_configuration_changed);
	debug_led(4);
	usb_peripheral_reset();
	debug_led(5);
	
	usb_device_init(0, &usb_device);
	debug_led(6);
	
	usb_queue_init(&usb_endpoint_control_out_queue);
	usb_queue_init(&usb_endpoint_control_in_queue);
	debug_led(7);
	usb_queue_init(&usb_endpoint_bulk_out_queue);
	usb_queue_init(&usb_endpoint_bulk_in_queue);
	debug_led(8);

	usb_endpoint_init(&usb_endpoint_control_out);
	usb_endpoint_init(&usb_endpoint_control_in);
	debug_led(9);
	
	nvic_set_priority(NVIC_USB0_IRQ, 255);
	debug_led(10);

	usb_run(&usb_device);
	debug_led(11);
	
	int i;
	while(true) {
		led_on(LED1);
		led_off(LED2);
		led_on(LED3);
		led_off(LED4);

		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
		
		led_off(LED1);
		led_on(LED2);
		led_off(LED3);
		led_on(LED4);
		
		for (i = 0; i < 2000000; i++)	/* Wait a bit. */
			__asm__("nop");
	}
	
	return 0;
}
