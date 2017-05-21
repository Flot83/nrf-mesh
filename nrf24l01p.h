
#ifndef __NRF24L01P__
#define __NRF24L01P__

#include "mbed.h"

namespace nrf
{
    enum class Mode
    {
        Uninitialised,
        PowerDown,
        Standby,
        Tx,
        Rx
    };

    namespace reg
    {
        uint8_t const CONFIG        =   0x00;  // 'Config' register address
        uint8_t const EN_AA         =   0x01;  // 'Enable Auto Acknowledgment' register address
        uint8_t const EN_RXADDR     =   0x02;  // 'Enabled RX addresses' register address
        uint8_t const SETUP_AW      =   0x03;  // 'Setup address width' register address
        uint8_t const SETUP_RETR    =   0x04;  // 'Setup Auto. Retrans' register address
        uint8_t const RF_CH         =   0x05;  // 'RF channel' register address
        uint8_t const RF_SETUP      =   0x06;  // 'RF setup' register address
        uint8_t const STATUS        =   0x07;  // 'Status' register address
        uint8_t const OBSERVE_TX    =   0x08;  // 'Observe TX' register address
        uint8_t const CD            =   0x09;  // 'Carrier Detect' register address
        uint8_t const RX_ADDR_P0    =   0x0A;  // 'RX address pipe0' register address
        uint8_t const RX_ADDR_P1    =   0x0B;  // 'RX address pipe1' register address
        uint8_t const RX_ADDR_P2    =   0x0C;  // 'RX address pipe2' register address
        uint8_t const RX_ADDR_P3    =   0x0D;  // 'RX address pipe3' register address
        uint8_t const RX_ADDR_P4    =   0x0E;  // 'RX address pipe4' register address
        uint8_t const RX_ADDR_P5    =   0x0F;  // 'RX address pipe5' register address
        uint8_t const TX_ADDR       =   0x10;  // 'TX address' register address
        uint8_t const RX_PW_P0      =   0x11;  // 'RX payload width, pipe0' register address
        uint8_t const RX_PW_P1      =   0x12;  // 'RX payload width, pipe1' register address
        uint8_t const RX_PW_P2      =   0x13;  // 'RX payload width, pipe2' register address
        uint8_t const RX_PW_P3      =   0x14;  // 'RX payload width, pipe3' register address
        uint8_t const RX_PW_P4      =   0x15;  // 'RX payload width, pipe4' register address
        uint8_t const RX_PW_P5      =   0x16;  // 'RX payload width, pipe5' register address
        uint8_t const FIFO_STATUS   =   0x17;  // 'FIFO Status Register' register address
        uint8_t const DYNPD		    =   0x1C;  // Enable Dynamic Payload Length
        uint8_t const FEATURE		=   0x1D;  // Feature Register
    }
    namespace bit
    {
        uint8_t const RF_CH_Mask            = 0x7F;

        uint8_t const CONFIG_Mask_Reserved  =	0x7F;
        uint8_t const CONFIG_PRIM_RX		=	(1<<0);
        uint8_t const CONFIG_PWR_UP			=   (1<<1);
        uint8_t const CONFIG_CRCO			=   (1<<2);
        uint8_t const CONFIG_EN_CRC			=   (1<<3);
        uint8_t const CONFIG_MASK_MAX_RT	=	(1<<4);
        uint8_t const CONFIG_MASK_TX_DS		=   (1<<5);
        uint8_t const CONFIG_MASK_RX_DR	    =	(1<<6);

        uint8_t const CONFIG_neg_EN_CRC		=   (~0x08);
        uint8_t const CONFIG_neg_PWR_UP		=   (~0x02);
        uint8_t const CONFIG_neg_PRIM_RX	=   (~0x01);

        uint8_t const STATUS_TX_FULL        =   (1<<0);
        uint8_t const STATUS_RX_P_NO        =   (0x7<<1);
        uint8_t const STATUS_MAX_RT         =   (1<<4);
        uint8_t const STATUS_TX_DS          =   (1<<5);
        uint8_t const STATUS_RX_DR          =   (1<<6);

        uint8_t const RF_SETUP_RF_PWR_MASK       =   (0x3<<1);
        uint8_t const RF_SETUP_RF_PWR_MIN_18DBM  =   (0x0<<1);
        uint8_t const RF_SETUP_RF_PWR_MIN_12DBM  =   (0x1<<1);
        uint8_t const RF_SETUP_RF_PWR_MIN_6DBM   =   (0x2<<1);
        uint8_t const RF_SETUP_RF_PWR_0DBM       =   (0x3<<1);

        uint8_t const RF_SETUP_RF_DR_HIGH_BIT    =   (1 << 3);
        uint8_t const RF_SETUP_RF_DR_LOW_BIT     =   (1 << 5);
        uint8_t const RF_SETUP_RF_DR_MASK        =   ((1 << 3)|(1 << 5));
        uint8_t const RF_SETUP_RF_DR_250KBPS     =   (1<<5);
        uint8_t const RF_SETUP_RF_DR_1MBPS       =   (0);
        uint8_t const RF_SETUP_RF_DR_2MBPS       =   (1<<3);
    }
    namespace cmd
    {
        uint8_t const READ_REG       = 0x00;  // Define read command to register
        uint8_t const WRITE_REG      = 0x20;  // Define write command to register
        uint8_t const RD_RX_PLOAD    = 0x61;  // Define RX payload register address
        uint8_t const WR_TX_PLOAD    = 0xA0;  // Define TX payload register address
        uint8_t const FLUSH_TX       = 0xE1;  // Define flush TX register command
        uint8_t const FLUSH_RX       = 0xE2;  // Define flush RX register command
        uint8_t const REUSE_TX_PL    = 0xE3;  // Define reuse TX payload register command
        uint8_t const R_RX_PL_WID    = 0x60;  // Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO (Flush if >32)
        uint8_t const NOP            = 0xFF;  // Define No Operation, might be used to read status register
    }

    namespace delay
    {
        int const Tundef2pd_us =  100000 ;  // 100mS
        int const Tstby2a_us   =     130 ;  // 130uS
        int const Thce_us      =      10 ;  //  10uS
        int const Tpd2stby_us  =    4500 ;  // 4.5mS worst case
        int const Tpece2csn_us =       4 ;  //   4uS
    }
    
    namespace datarate
    {
        uint16_t const d_2Mbps = 2000;
    }
    uint8_t const mode_uninitialised = 0;
    uint8_t const mode_power_down    = 1;
    uint8_t const mode_standby       = 2;
    uint8_t const mode_tx            = 3;
    uint8_t const mode_rx            = 4;


}


class Nrf24l01p
{
public:
    //nRF24L01+ pins:
    //1:Gnd, 2:3.3v, 3:ce, 4:csn, 5:sck, 6:mosi, 7:miso, 8:irq 
    Nrf24l01p(PinName ce, PinName csn, PinName sck, PinName mosi, PinName miso);

    //--------------------- Level 2 - Modes ----------------------------
    void setMode(nrf::Mode m);
    //--------------------- Level 2 - Config ----------------------------
    void set_DataRate();
    void select_Channel();
    void get_Channel();
    void set_TxAddress();
    void set_RxAddress();
    //--------------------- Level 1 ----------------------------
    uint8_t writeRegister(uint8_t reg,uint8_t val);
    uint8_t readRegister(uint8_t reg);
    uint8_t readStatus();

    void setbit(uint8_t reg, uint8_t bit);
    void clearbit(uint8_t reg, uint8_t bit);

    void writeBuffer(uint8_t reg,uint8_t *buf,uint8_t size);
    void readBuffer(uint8_t reg,uint8_t *buf,uint8_t size);

    void command(uint8_t reg);

    //--------------------- Level 0 ----------------------------
    void ce_pin_highEnable();
    void ce_pin_lowDisable();

    void csn_pin_lowSelect();
    void csn_pin_highClear();

public:
    SPI         spi;
    DigitalOut  csn_pin;
    DigitalOut  ce_pin;

    nrf::Mode     mode;
};


#endif /* __NRF24L01P__ */