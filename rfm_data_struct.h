/*
 * rfm_data_struct.h
 *
 *  Created on: 21 lip 2016
 *      Author: narwp
 */

#ifndef RFM_DATA_STRUCT_H_
#define RFM_DATA_STRUCT_H_

#include <linux/spinlock.h>
#include <linux/spi/spi.h>

#define NUM_MAX_CONCURRENT_MSG   	(3)

typedef struct {
	unsigned int   id;
	unsigned char  type;
	unsigned short in_reg[IN_REGISTERS];
	unsigned short out_reg[OUT_REGISTERS];
}Node;



typedef enum _rfm12_state_t {
   RFM12_STATE_NO_CHANGE      = 0,
   RFM12_STATE_CONFIG         = 1,
   RFM12_STATE_SLEEP            = 2,
   RFM12_STATE_IDLE            = 3,
   RFM12_STATE_LISTEN         = 4,
   RFM12_STATE_RECV            = 5,
   RFM12_STATE_RECV_FINISH      = 6,
//   RFM12_STATE_SEND_PRE1      = 7,
//   RFM12_STATE_SEND_PRE2      = 8,
//   RFM12_STATE_SEND_PRE3      = 9,
//   RFM12_STATE_SEND_SYN1      = 10,
//   RFM12_STATE_SEND_SYN2      = 11,
//   RFM12_STATE_SEND            = 12,
//   RFM12_STATE_SEND_TAIL1      = 13,
//   RFM12_STATE_SEND_TAIL2      = 14,
//   RFM12_STATE_SEND_TAIL3      = 15,
   RFM12_STATE_SEND_FINISHED  = 16
} rfm12_state_t;


struct rfm12_spi_message {
   struct spi_message   spi_msg;
   struct spi_transfer  spi_transfers[4];
   rfm12_state_t        spi_finish_state;
   uint8_t              spi_tx[8], spi_rx[10];
   void*                context;
   uint8_t              pos;
};

struct rfm12_data {
	   struct spi_device*   		spi;
	   rfm12_state_t        		state;
	   uint8_t 						bit_rate ;
	   struct rfm12_spi_message 	spi_msgs[NUM_MAX_CONCURRENT_MSG];
	   uint32_t                  	spi_speed_hz;
	   spinlock_t           		lock;
	   uint8_t                   	free_spi_msgs;
	   Node 						nodes[NODES];


//   u16                   irq;
//   void*                 irq_identifier;
//
//   dev_t                devt;


//	   struct list_head     		device_entry;
//   rfm12_module_type_t  module_type;

//   u8                   open, should_release, trysend;


   //u8                   group_id, band_id, jee_id, jee_autoack;
//   unsigned long        bytes_recvd, pkts_recvd;
//   unsigned long        bytes_sent, pkts_sent;
//   unsigned long        num_recv_overflows, num_recv_timeouts, num_recv_crc16_fail;
//   unsigned long        num_send_underruns, num_send_timeouts;
//   u8*                  in_buf, *in_buf_pos;
//   u8*                  out_buf, *out_buf_pos;
//   u8*                  in_cur_len_pos;
//   u8*                  in_cur_end, *out_cur_end;
//   u16                  crc16;
//   int                  in_cur_num_bytes, out_cur_num_bytes;
//

//
//   struct timer_list    rxtx_watchdog;
//   u8                   rxtx_watchdog_running;
//   struct timer_list    retry_sending_timer;
//   u8                   retry_sending_running;
//   wait_queue_head_t    wait_read;
//   wait_queue_head_t    wait_write;
//
//   u16                  rf12_last_status;
//
//   u8                   rf69_req_mode, rf69_recv_read_rssi;
//   int                  rf69_last_rssi;
//   void (*rf69_mode_callback)(void*);
//   void (*rf69_flush_fifo_callback)(struct rfm12_data*);
};

#endif /* RFM_DATA_STRUCT_H_ */
