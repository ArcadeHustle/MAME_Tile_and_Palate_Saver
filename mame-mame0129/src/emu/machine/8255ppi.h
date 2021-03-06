/*********************************************************************

    8255ppi.h

    Intel 8255 PPI I/O chip

*********************************************************************/

#ifndef __8255PPI_H_
#define __8255PPI_H_

#define PPI8255		DEVICE_GET_INFO_NAME(ppi8255)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ppi8255_interface ppi8255_interface;
struct _ppi8255_interface
{
	read8_device_func port_a_read;
	read8_device_func port_b_read;
	read8_device_func port_c_read;
	write8_device_func port_a_write;
	write8_device_func port_b_write;
	write8_device_func port_c_write;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PPI8255_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPI8255, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPI8255_RECONFIG(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag, PPI8255) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPI8255_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, PPI8255)



/* device interface */
DEVICE_GET_INFO(ppi8255);

READ8_DEVICE_HANDLER( ppi8255_r );
WRITE8_DEVICE_HANDLER( ppi8255_w );


void ppi8255_set_port_a_read( const device_config *device, read8_device_func port_a_read );
void ppi8255_set_port_b_read( const device_config *device, read8_device_func port_b_read );
void ppi8255_set_port_c_read( const device_config *device, read8_device_func port_c_read );

void ppi8255_set_port_a_write( const device_config *device, write8_device_func port_a_write );
void ppi8255_set_port_b_write( const device_config *device, write8_device_func port_b_write );
void ppi8255_set_port_c_write( const device_config *device, write8_device_func port_c_write );

void ppi8255_set_port_a( const device_config *device, UINT8 data );
void ppi8255_set_port_b( const device_config *device, UINT8 data );
void ppi8255_set_port_c( const device_config *device, UINT8 data );

UINT8 ppi8255_get_port_a( const device_config *device );
UINT8 ppi8255_get_port_b( const device_config *device );
UINT8 ppi8255_get_port_c( const device_config *device );

#endif /* __8255PPI_H_ */
