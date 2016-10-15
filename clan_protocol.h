/*
 * clan_protocol.h
 *
 *  Created on: 21 lip 2016
 *      Author: narwp
 */

#ifndef CLAN_PROTOCOL_H_
#define CLAN_PROTOCOL_H_



//struktura danych rozkazu RF
typedef struct __RF_Order
{
        unsigned int             ID    : 20,     //Id Slave
                                 Order : 4,      //Kod rozkazu
                                 Auxi  : 8;      //Pole pomocnicze, np. Nr rejestru (konieczne z powodu wyrównania bajtowego)
} _RF_Order;


typedef struct __RF_6B
        {
        unsigned short  CRC;                      //suma crc
        }_RF_6B;


typedef struct __RF_8B
        {
        unsigned short  Val;                      //wartoœæ
        unsigned short  CRC;                      //suma crc

        }_RF_8B;




#pragma pack(1)
typedef union __RF_Command
{
  struct
  {
    unsigned int             ID    : 20,     //Id Slave
                             Order : 4,      //Kod rozkazu
                             Auxi  : 8;      //Pole pomocnicze, np. Nr rejestru (konieczne z powodu wyrównania bajtowego)
    union                                    //Dane + CRC
    {
      _RF_6B  B6;                            //Dane + CRC dla rozkazu d³ugoœci 6B
      _RF_8B  B8;                            //Dane + CRC dla rozkazu d³ugoœci 8B
    };
  };
  struct  //struktura rozkazu o zmianê ID
  {
    unsigned long long       CHID_CUR_ID    : 20,     //Bierz¹cy Id - Slave
                             CHID_Order     : 4,      //Kod rozkazu
                             CHID_KODE      : 4,      //kod kontrolny
                             CHID_NEW_ID    : 20,     //Nowy Id - Slave
                             CHID_CRC       : 16;     //suma crc
  };
    struct  //struktura rozkazu wywo³ania ogólnego (broadcasting)
  {
    unsigned long long       BC_ID1       : 20,     //Id rozg³oœne
                             BC_Order     : 4,      //Kod rozkazu
                             BC_REG      : 4,      //pytanie (kto ma po³¹czenie z wêz³em, potrzebuje rejestr nr..
                             BC_ID2       : 20,     //Id szukane
                             BC_CRC       : 16;     //suma crc
  };


  char Streem[8];                             //Strumieñ rozkazu
}_RF_Command, *_pRF_Command;
  #pragma pack()


#endif /* CLAN_PROTOCOL_H_ */
