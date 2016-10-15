#ifndef _RFM_
#define _RFM_

#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <stddef.h>
#include <linux/types.h>

#include "transfer.h"
#include "rfm_data_struct.h"
#include "clan_protocol.h"





/* MODULE PARAMETERS */



#define RF_READ_STATUS     0x0000
#define RF_IDLE_MODE       0x820D
#define RF_SLEEP_MODE      0x8205
#define RF_TXREG_WRITE     0xB800
#define RF_RECEIVER_ON     0x82DD
#define RF_XMITTER_ON      0x823D
#define RF_RX_FIFO_READ    0xB000
#define RF_RX_FIFO_STOP    0xCA81
#define RF_RX_FIFO_START   0xCA83

#define RF_STATUS_BIT_LBAT          (0x0400)
#define RF_STATUS_BIT_FFEM          (0x0200)
#define RF_STATUS_BIT_FFOV_RGUR     (0x2000)
#define RF_STATUS_BIT_RSSI          (0x0100)
#define RF_STATUS_BIT_FFIT_RGIT     (0x8000)

#define RFM69_MASK_REGWRITE         (0x80)

#define RFM69_REG_IRQFLAGS2         (0x28)
#define RFM69_REG_FIFO              (0x00)
#define RFM69_REG_OPMODE            (0x01)
#define RFM69_REG_RSSIVALUE         (0x24)
#define RFM69_REG_DIOMAPPING1       (0x25)

#define RFM69_MODE_STANDBY          (0x04)
#define RFM69_MODE_RECEIVER         (0x10)
#define RFM69_MODE_XMITTER          (0x0C)

#define RFM69_DIOMAPPING1_DIO0_00   (0x00)
#define RFM69_DIOMAPPING1_DIO0_10   (0x80)

#define RFM69_IRQ2_FIFOFULL         (0x80)
#define RFM69_IRQ2_FIFONOTEMPTY     (0x40)
#define RFM69_IRQ2_FIFOOVERRUN      (0x10)

#define RFM69_RSSIVAL_TO_DBM(val)   (((int)(-(val)))>>1)
#define RFM69_RSSIVAL_SEND_MIN      (-82)


#define OPEN_WAIT_MILLIS   			(50)

#define RF_ChannelWidth   					0.300     	//SzerokoÂœÃ¦ kanaÂ³u/ odstÃªpy pomiedzy kanaÂ³ami w MHz
#define RF_BaseFrq        					860.32    	//Czestotliwosc nosna kanaÂ³u 0 w MHz

#define RF_AMOUNT_CHANNELS					40			//liczba kanalow od 1 do 20 + 0 (base)
#define RF_BASE_CHANNEL						1			//kanal bazowy

#define BIT_RATE					7

#define RF_BuffSizeRX				20

const char Preambu[]={0xAA, 0xAA, 0xAA};  //Preambu³a
const char SynKay[]= {0x2D, 0xD4};        //s³owo synchronizacyjne


/* Exported macro ------------------------------------------------------------*/
#define RF_SetFreqReg(ch) (short int)(200*(RF_BaseFrq + ch*RF_ChannelWidth) -172000)



static uint8_t spi_write_reg(uint8_t reg, uint8_t val)
{
	struct spi_transfer trans[3];
	struct spi_message m;
	uint8_t rxbuf;

	spi_message_init(&m);

	memset(trans, 0, sizeof(trans));

	trans[0].tx_buf = &reg;
	trans[0].len = 1;
	spi_message_add_tail(&trans[0], &m);

	trans[1].tx_buf = &val;
	trans[1].len = 1;
	spi_message_add_tail(&trans[1], &m);

	trans[2].rx_buf = &rxbuf;
	trans[2].len = 1;
	spi_message_add_tail(&trans[2], &m);

	//spin_lock_irqsave(&spilock, spilockflags);
	spi_sync(active_boards[0].spi_device, &m);
	//spin_unlock_irqrestore(&spilock, spilockflags);

	return rxbuf;
}



struct spi_transfer
rfm_make_spi_transfer(struct rfm12_data* rfm12,
		uint16_t cmd, uint8_t* tx_buf, uint8_t* rx_buf)
{
   struct spi_transfer tr = {
     .tx_buf           = tx_buf,
     .rx_buf           = rx_buf,
     .len              = 2,
     .cs_change        = 0,
     .bits_per_word    = 0,
     .delay_usecs      = 0,
     .speed_hz         = rfm12->spi_speed_hz
   };

   tx_buf[0] = (cmd >> 8) & 0xff;
   tx_buf[1] = cmd & 0xff;

   return tr;
}



typedef struct __RF_Rx
{
  char Buff[RF_BuffSizeRX];
  char ReciveBytes;
}_RF_Rx;






//Sta³e ------------------------------------------------------------------------

const uint8_t   bCRC_Hi[] = {0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,
                          0x0,0xC1,0x81,0x40,0x0,0xC1,0x81,0x40,0x1,0xC0,
                         0x80,0x41,0x1,0xC0,0x80,0x41,0x0,0xC1,0x81,0x40,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x0,0xC1,
                         0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,0x80,0x41,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x0,0xC1,
                         0x81,0x40,0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x0,0xC1,0x81,0x40,
                          0x1,0xC0,0x80,0x41,0x1,0xC0,0x80,0x41,0x0,0xC1,
                         0x81,0x40,0x1,0xC0,0x80,0x41,0x0,0xC1,0x81,0x40,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x0,0xC1,0x81,0x40,
                          0x1,0xC0,0x80,0x41,0x0,0xC1,0x81,0x40,0x1,0xC0,
                         0x80,0x41,0x1,0xC0,0x80,0x41,0x0,0xC1,0x81,0x40,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,
                          0x0,0xC1,0x81,0x40,0x0,0xC1,0x81,0x40,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,
                          0x1,0xC0,0x80,0x41,0x0,0xC1,0x81,0x40,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40,0x0,0xC1,0x81,0x40,
                          0x1,0xC0,0x80,0x41,0x1,0xC0,0x80,0x41,0x0,0xC1,
                         0x81,0x40,0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,
                          0x0,0xC1,0x81,0x40,0x1,0xC0,0x80,0x41,0x1,0xC0,
                         0x80,0x41,0x0,0xC1,0x81,0x40};

const uint8_t  bCRC_Lo[] = { 0x0,0xC0,0xC1,0x1,0xC3,0x3,0x2,0xC2,0xC6,0x6,
                          0x7,0xC7,0x5,0xC5,0xC4,0x4,0xCC,0xC,0xD,0xCD,
                          0xF,0xCF,0xCE,0xE,0xA,0xCA,0xCB,0xB,0xC9,0x9,
                          0x8,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
                         0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
                         0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
                         0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
                         0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
                         0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
                         0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
                         0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
                         0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
                         0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
                         0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
                         0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
                         0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
                         0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
                         0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
                         0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
                         0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
                         0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
                         0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
                         0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
                         0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
                         0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
                         0x43,0x83,0x41,0x81,0x80,0x40};

//___________________________Koniec sta³ych ____________________________________



//!-----------------------------------------------------------------------------
//!  Nazwa funkcji :    crc16
//!  Opis:              Oblicza sumê kontroln¹ CRC 16
//!-----------------------------------------------------------------------------
//!  modbusframe  - adres pierwszej cyfry
//!  Length       - zakres liczb do wyliczenia CRC
//!-----------------------------------------------------------------------------
uint16_t crc16(uint8_t* modbusframe,int Length)
{
	uint32_t index;
    uint8_t crc_Low = 0xFF;
    uint8_t crc_High = 0xFF;

    for (int i_tmp = 0; i_tmp<Length; i_tmp++)
    {
    index = crc_High ^ modbusframe[i_tmp];
    crc_High = crc_Low ^ bCRC_Hi[index] ;
    crc_Low = bCRC_Lo[index];
    }

    return (crc_High << 8 | crc_Low);
}//_______________________ Koniec funkcji crc16 ________________________________





_RF_Command  *pRF_Command;
uint8_t RF_ReciveFarm(uint8_t *Buff, uint8_t Size, struct rfm12_data* rfm12)
{
        char FrameOk=0;

        if (Size==8)
        {
           pRF_Command=(_pRF_Command)(Buff-7); //rzutowanie na strukturê rozkazu
           if (pRF_Command->B8.CRC == crc16(pRF_Command->Streem, Size-2))  FrameOk=1;
        }
        else if (Size==6)
        {
           pRF_Command=(_pRF_Command)(Buff-5); //rzutowanie na strukturê rozkazu
           if (pRF_Command->B6.CRC == crc16(pRF_Command->Streem, Size-2))  FrameOk=1;
        }
        else
        {
          if ((Size>8) && (FrameOk==0))
          {
            pRF_Command=(_pRF_Command)(Buff-7); //rzutowanie na strukturê rozkazu
            if (pRF_Command->B8.CRC == crc16(pRF_Command->Streem, 6))  FrameOk=1;
          }

          if ((Size>6) && (FrameOk==0))
          {
           pRF_Command=(_pRF_Command)(Buff-5); //rzutowanie na strukturê rozkazu
           if (pRF_Command->B6.CRC == crc16(pRF_Command->Streem, 4)) FrameOk=1;

          }
        }

        if (FrameOk)
        {
			interpreterReceivedData(pRF_Command, rfm12);
        }


}


//-----------------------------------------------------------------------------
//!  Nazwa funkcji :    RF_ReciveByte
//!  Funkcja interpretacji otrzymanego bajtu
//-----------------------------------------------------------------------------
uint8_t RF_ReciveByte(uint8_t Res, struct rfm12_data* rfm12)
{
	static _RF_Rx RF_Rx;

          for (char i=0; i<RF_BuffSizeRX-1; i++) {RF_Rx.Buff[i]=RF_Rx.Buff[i+1];}
          RF_Rx.Buff[RF_BuffSizeRX-1]=Res;

          if (RF_Rx.ReciveBytes<=RF_BuffSizeRX) RF_Rx.ReciveBytes++;

          //sprawdzenie koñca
          if (
          (RF_Rx.Buff[RF_BuffSizeRX-3]==Preambu[0]) &&
            (RF_Rx.Buff[RF_BuffSizeRX-2]==Preambu[1]) &&
          (RF_Rx.Buff[RF_BuffSizeRX-1]==Preambu[2])
          )
          {
            //if ((SlaveMode==0) && (RF_Status.ConfResive==0)) RF_Status.ConfResive=1;  //znacznik odebrania danych (dla kontroli kana³u)
            //if ((SlaveMode==1) && (RF_Status.ConfResiveSlave==0)) RF_Status.ConfResiveSlave=1;  //znacznik odebrania danych (dla kontroli kana³u)

            //RF_Status.RfFuse=1;
        	 // printk(KERN_INFO RFM12B_DRV_NAME "Recive");
            RF_ReciveFarm(&RF_Rx.Buff[RF_BuffSizeRX-4], RF_Rx.ReciveBytes-3, rfm12);  //funkcja interpretacji otrzymanej ramki

             RF_Rx.ReciveBytes=0;

            return 1;
          }
          return 0;
}

void rfm12_fifoReset(struct rfm12_data* rfm12)
{
	struct spi_message msg;
	struct spi_transfer tr[3];
	uint8_t tx_buf[10];

	rfm12->state = RFM12_STATE_LISTEN;
	rfm12->spi_speed_hz = RFM12B_SPI_MAX_HZ_RF12;

	spi_message_init(&msg);

	tr[0] = rfm_make_spi_transfer(rfm12, RF_RX_FIFO_STOP, tx_buf, NULL);
	tr[0].cs_change = 1;
	spi_message_add_tail(&tr[0], &msg);

	tr[1] = rfm_make_spi_transfer(rfm12, RF_RX_FIFO_START, tx_buf+2, NULL);
	tr[1].cs_change = 1;
	spi_message_add_tail(&tr[1], &msg);

	tr[2] = rfm_make_spi_transfer(rfm12, RF_READ_STATUS, tx_buf+4, NULL);
	spi_message_add_tail(&tr[2], &msg);

	spi_sync(rfm12->spi, &msg);
}

static int
rfm12_read(struct rfm12_data* rfm12)
{
	struct spi_message msg;
	struct spi_transfer tr[5];
	uint8_t tx_buf[26];
	union{
		uint8_t buf[4];
		uint16_t buf_W[2];
	}rx;

	rx.buf[0] = 0;

	rfm12->state = RFM12_STATE_RECV;
	rfm12->spi_speed_hz = RFM12B_SPI_MAX_HZ_RF12;

	spi_message_init(&msg);

	tr[0] = rfm_make_spi_transfer(rfm12, RF_READ_STATUS, tx_buf+0, rx.buf+0);
	tr[0].cs_change = 1;
	spi_message_add_tail(&tr[0], &msg);

	tr[1] = rfm_make_spi_transfer(rfm12, RF_RX_FIFO_READ, tx_buf+2, rx.buf+2);
	spi_message_add_tail(&tr[1], &msg);

	spi_sync(rfm12->spi, &msg);

	if (RF_ReciveByte(rx.buf[3], rfm12))
	{
		rfm12_fifoReset(rfm12);
	}
}







static int
rfm12_initRx(struct rfm12_data* rfm12)
{
	struct spi_message msg;
	struct spi_transfer tr, tr2;
	uint8_t tx_buf[26];

	   rfm12->state = RFM12_STATE_LISTEN;
	   rfm12->spi_speed_hz = RFM12B_SPI_MAX_HZ_RF12;

	   spi_message_init(&msg);

	   tr = rfm_make_spi_transfer(rfm12, RF_RX_FIFO_READ, tx_buf+0, NULL);
	   tr.cs_change = 1;
	   spi_message_add_tail(&tr, &msg);

	   tr2 = rfm_make_spi_transfer(rfm12, RF_RECEIVER_ON, tx_buf+2, NULL);
	   spi_message_add_tail(&tr2, &msg);

	   spi_sync(rfm12->spi, &msg);
}















static int
rfm12_setup(struct rfm12_data* rfm12)
{
   struct spi_transfer tr, tr2, tr3, tr4, tr5, tr6, tr7, tr8, tr9,
     tr10, tr11, tr12, tr13;
   struct spi_message msg;
   uint8_t tx_buf[26];
   int err;

   rfm12->state = RFM12_STATE_CONFIG;
   rfm12->spi_speed_hz = RFM12B_SPI_MAX_HZ_RF12;

   spi_message_init(&msg);

   tr = rfm_make_spi_transfer(rfm12, RF_READ_STATUS, tx_buf+0, NULL);
   tr.cs_change = 1;
   spi_message_add_tail(&tr, &msg);

   tr2 = rfm_make_spi_transfer(rfm12, RF_SLEEP_MODE, tx_buf+2, NULL);
   tr2.cs_change = 1;
   spi_message_add_tail(&tr2, &msg);

   tr3 = rfm_make_spi_transfer(rfm12, RF_TXREG_WRITE, tx_buf+4, NULL);
   spi_message_add_tail(&tr3, &msg);

   err = spi_sync(rfm12->spi, &msg);

   if (err)
     goto pError;

   msleep(OPEN_WAIT_MILLIS);

   // ok, we're now ready to be configured.
   spi_message_init(&msg);

//   tr = rfm_make_spi_transfer(rfm12, 0x8067 |
//         ((rfm12->band_id & 0xff) << 4), tx_buf+0, NULL);
   tr = rfm_make_spi_transfer(rfm12, 0x8067, tx_buf+0, NULL);

   tr.cs_change = 1;
   spi_message_add_tail(&tr, &msg);

   tr2 = rfm_make_spi_transfer(rfm12, 0xA000|RF_SetFreqReg(RF_BASE_CHANNEL) , tx_buf+2, NULL);
   tr2.cs_change = 1;
   spi_message_add_tail(&tr2, &msg);

   tr3 = rfm_make_spi_transfer(rfm12, 0xC600 | rfm12->bit_rate, tx_buf+4, NULL);
   tr3.cs_change = 1;
   spi_message_add_tail(&tr3, &msg);

   tr4 = rfm_make_spi_transfer(rfm12, 0x94A0, tx_buf+6, NULL);
   tr4.cs_change = 1;
   spi_message_add_tail(&tr4, &msg);

   tr5 = rfm_make_spi_transfer(rfm12, 0xC2AC, tx_buf+8, NULL);
   tr5.cs_change = 1;
   spi_message_add_tail(&tr5, &msg);

   //if (0 != rfm12->group_id) {
     tr6 = rfm_make_spi_transfer(rfm12, 0xCA83, tx_buf+10, NULL);
     tr6.cs_change = 1;
     spi_message_add_tail(&tr6, &msg);

  //   tr7 = rfm_make_spi_transfer(rfm12, 0xCE00 |
  //      rfm12->group_id, tx_buf+12, NULL);
    tr7 = rfm_make_spi_transfer(rfm12, 0xCED4, tx_buf+12, NULL);


     tr7.cs_change = 1;
     spi_message_add_tail(&tr7, &msg);
//   } else {
//     tr6 = rfm_make_spi_transfer(rfm12, 0xCA8B, tx_buf+10, NULL);
//     tr6.cs_change = 1;
//     spi_message_add_tail(&tr6, &msg);
//
//     tr7 = rfm_make_spi_transfer(rfm12, 0xCE2D, tx_buf+12, NULL);
//     tr7.cs_change = 1;
//     spi_message_add_tail(&tr7, &msg);
//  // }

   tr8 = rfm_make_spi_transfer(rfm12, 0xC483, tx_buf+14, NULL);
   tr8.cs_change = 1;
   spi_message_add_tail(&tr8, &msg);

   tr9 = rfm_make_spi_transfer(rfm12, 0x9850, tx_buf+16, NULL);
   tr9.cs_change = 1;
   spi_message_add_tail(&tr9, &msg);

   tr10 = rfm_make_spi_transfer(rfm12, 0xCC77, tx_buf+18, NULL);
   tr10.cs_change = 1;
   spi_message_add_tail(&tr10, &msg);

   tr11 = rfm_make_spi_transfer(rfm12, 0xE000, tx_buf+20, NULL);
   tr11.cs_change = 1;
   spi_message_add_tail(&tr11, &msg);

   tr12 = rfm_make_spi_transfer(rfm12, 0xC800, tx_buf+22, NULL);
   tr12.cs_change = 1;
   spi_message_add_tail(&tr12, &msg);

   // set low battery threshold to 2.9V
   tr13 = rfm_make_spi_transfer(rfm12, 0xC047, tx_buf+24, NULL);
   spi_message_add_tail(&tr13, &msg);

   err = spi_sync(rfm12->spi, &msg);

   if (0 == err) {
     spi_message_init(&msg);

     tr = rfm_make_spi_transfer(rfm12, RF_READ_STATUS, tx_buf+0, NULL);
     spi_message_add_tail(&tr, &msg);

     err = spi_sync(rfm12->spi, &msg);
   }

pError:
   rfm12->state = RFM12_STATE_IDLE;

   return err;
}





// we want GPIO_17 (pin 11 on P5 pinout raspberry pi rev. 2 board)
// to generate interrupt
#define GPIO_ANY_GPIO                25

// text below will be seen in 'cat /proc/interrupt' command
#define GPIO_ANY_GPIO_DESC           "Some gpio pin description"

// below is optional, used in more complex code, in our case, this could be
// NULL
#define GPIO_ANY_GPIO_DEVICE_DESC    "some_device"






/****************************************************************************/
/* Interrupts variables block                                               */
/****************************************************************************/
short int irq_any_gpio    = 0;


/****************************************************************************/
/* IRQ handler - fired on interrupt                                         */
/****************************************************************************/
static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {

   unsigned long flags;

   local_irq_save(flags); // disable hard interrupts (remember them in flag 'flags')
   rfm12_read(active_boards[0].irq_data);
   local_irq_restore(flags); // restore hard interrupts

   return IRQ_HANDLED;
}


/****************************************************************************/
/* This function configures interrupts.                                     */
/****************************************************************************/
void r_int_config(void) {
   if (gpio_request(GPIO_ANY_GPIO, GPIO_ANY_GPIO_DESC)) {
      printk("GPIO request faiure: %s\n", GPIO_ANY_GPIO_DESC);
      return;
   }

   if ( (irq_any_gpio = gpio_to_irq(GPIO_ANY_GPIO)) < 0 ) {
      printk("GPIO to IRQ mapping faiure %s\n", GPIO_ANY_GPIO_DESC);
      return;
   }

   printk(KERN_NOTICE "Mapped int %d\n", irq_any_gpio);

   if (request_irq(irq_any_gpio,
                   (irq_handler_t ) r_irq_handler,
                   IRQF_TRIGGER_FALLING,
                   GPIO_ANY_GPIO_DESC,
                   GPIO_ANY_GPIO_DEVICE_DESC)) {
      printk("Irq Request failure\n");
      return;
   }

   return;
}


/****************************************************************************/
/* This function releases interrupts.                                       */
/****************************************************************************/
void r_int_release(void) {

   free_irq(irq_any_gpio, GPIO_ANY_GPIO_DEVICE_DESC);
   gpio_free(GPIO_ANY_GPIO);

   return;
}

#endif
