#!/usr/bin/env python
#
# Copyright 2015 Dominic Spill <dominicgs@gmail.com)
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

import sys

from greatfet import GreatFET

if __name__ == '__main__':
    device = GreatFET()
    if device:
        print 'Found GreatFET!'
    else:
        print 'No GreatFET devices found.'
        sys.exit()

    print "Board ID %d - %s" % (device.board_id(), device.board_name())

    #serial_no = vendor_request_in(usb_vendor_request_read_partid_serialno, length=30)
    #print "Serial no: " + ''.join(["%02X " % x for x in serial_no])
    #
    ##vendor_request_out(usb_vendor_request_led_toggle, 4)
    #
    #vendor_request_out(usb_vendor_request_enable_usb1)