/*
    i2c-dev.h - i2c-bus driver, char device interface

    Copyright (C) 1995-97 Simon G. Vogl
    Copyright (C) 1998-99 Frodo Looijaard <frodol@dds.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef _UAPI_LINUX_I2C_DEV_H
#define _UAPI_LINUX_I2C_DEV_H

typedef enum
{
    ATCA_SUCCESS = 0x00, 		//!< Function succeeded.
    ATCA_COMM_FAIL, 			//!< Communication with device failed. Same as in hardware dependent modules.
	ATCA_TX_FAIL, 				//!< Failed to write
	ATCA_RX_FAIL, 				//!< Timed out while waiting for response.
	ATCA_PARAM_FAIL,			//!< input param ERROR
} ATCA_STATUS;

ATCA_STATUS hal_i2c_send(unsigned char *txdata, int txlength);
ATCA_STATUS hal_i2c_receive(unsigned char *rxdata, int rxlength);
ATCA_STATUS hal_i2c_wake();
ATCA_STATUS hal_i2c_idle();

#endif /* _UAPI_LINUX_I2C_DEV_H */

