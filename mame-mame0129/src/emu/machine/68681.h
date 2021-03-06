#ifndef _68681_H
#define _68681_H

typedef struct _duart68681_config duart68681_config;
struct _duart68681_config
{
	void (*irq_handler)(const device_config *device, UINT8 vector);
	void (*tx_callback)(const device_config *device, int channel, UINT8 data);
	UINT8 (*input_port_read)(const device_config *device);
	void (*output_port_write)(const device_config *device, UINT8 data);
};

#define DUART68681 DEVICE_GET_INFO_NAME(duart68681)
DEVICE_GET_INFO(duart68681);

#define MDRV_DUART68681_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, DUART68681, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_DUART_68681_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, DUART68681)


READ8_DEVICE_HANDLER(duart68681_r);
WRITE8_DEVICE_HANDLER(duart68681_w);

void duart68681_rx_data( const device_config* device, int ch, UINT8 data );

#endif //_68681_H
