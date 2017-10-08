

#include "rfmesh.h"

#define NRF_NUM (1)

static uint32_t nrf_handlers[NRF_NUM] = {0};

static void nrf_donothing(uint8_t *data,uint8_t size) {};

uint16_t crc_Fletcher16( uint8_t const *data, uint8_t count )
{
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	int index;

	for( index = 0; index < count; ++index )
	{
		sum1 = (sum1 + data[index]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	return (sum2 << 8) | sum1;
}

// size(size+data) : data : crc
void crc_set(uint8_t *data)
{
	uint8_t size = data[0];
	uint16_t crc = crc_Fletcher16(data,size);//check the data without excluding the crc
	data[size]   = (crc >> 8);
	data[size+1] = (crc & 0xFF);
}

// size(size+data) : data : crc
uint8_t crc_check(uint8_t const *data)
{
	uint8_t result = 1;
	uint8_t size = data[0];
	uint16_t crc = crc_Fletcher16(data,size);//check the data without excluding the crc
	if( (data[size] != (crc >> 8) ) || (data[size+1] != (crc & 0xFF) ) )
	{
		result = 0;
	}
	return result;
}

#define NO_DEBUG_CHECK_ALL

void nrf_irq()
{
    //on multi instance should derive the id from the irq
    RfMesh *handler = (RfMesh*)nrf_handlers[0];
    //decide here which irq to call
    uint8_t status = handler->nrf.readStatus();

    if(status & nrf::bit::status::RX_DR)
    {
        uint8_t rx_pipe_nb = (status & nrf::bit::status::RX_P_NO)>>1;
        uint8_t max_reread = 100;//just to avoid infinite loop, but with such a number loss is already likely
        while((rx_pipe_nb != 0x07) && (--max_reread!=0))//while RX_FIFO Not Empty
        {
            uint8_t data[32];
            #ifdef NRF_DYNAMIC_PAYLOAD
            uint8_t rf_size = getRxPayloadWidth();
            if(rf_size > 32)
            {
                handler->nrf.flushRX();
            }
            #else
            uint8_t rf_size = 32;
            #endif
            handler->nrf.readBuffer(nrf::cmd::R_RX_PLOAD,data,rf_size);
            uint8_t user_size = 30;
            bool is_provided = false;
            if(data[0]<=30)//30 + 2xBytes for CRC
            {
                user_size = data[0];
                if(crc_check(data))//crc error ignored
                {
                    handler->_callbacks[static_cast<int>(RfMesh::CallbackType::Message)](data,user_size);
                    is_provided = true;
                }
            }
            #ifndef NO_DEBUG_CHECK_ALL
            if(!is_provided)
            {
                handler->_callbacks[static_cast<int>(RfMesh::CallbackType::Message)](data,user_size);
            }
            #endif
            //reread the status to check if you need to get another buffer
            status = handler->nrf.readStatus();
            rx_pipe_nb = (status & nrf::bit::status::RX_P_NO)>>1;
        }
        if(max_reread == 0)
        {
            //do something as we might have been stuck or Under DoS Attack
        }
    }

    // Clear any pending interrupts
    handler->nrf.writeRegister(nrf::reg::STATUS,    nrf::bit::status::MAX_RT | nrf::bit::status::TX_DS | nrf::bit::status::RX_DR );

}

RfMesh::RfMesh(Serial *ps,PinName ce, PinName csn, PinName sck, PinName mosi, PinName miso,PinName irq):
                            //1:Gnd, 2:3.3v, 3:ce, 4:csn, 5:sck, 6:mosi, 7:miso, 8:irq
                            nrf(ps, ce, csn, sck, mosi, miso),
                            pser(ps),
                            nRFIrq(irq)
{
    for (size_t i = 0; i < sizeof _callbacks / sizeof _callbacks[0]; i++) 
    {
        _callbacks[i] = nrf_donothing;
    }
    
    nRFIrq.fall(&nrf_irq);//can set : callback(this, &Counter::increment)
    nRFIrq.mode(PullNone);
    nRFIrq.enable_irq();
    //current implementation with a single instance
    nrf_handlers[0] = (uint32_t)this;
}

void RfMesh::init()
{
    wait_ms(100);//Let the Power get stable

    pser->printf("Hello Mesh .... nRF24L01+ Dump :\r\n");
    print_nrf();

    pser->printf("Configuration\r\n");
    nrf.setMode(nrf::Mode::PowerDown);//Power Down

    //Flush any previously used RX and TX FIFOs
    pser->printf("Flushing Buffers\r\n");
    nrf.command(nrf::cmd::FLUSH_TX);
    nrf.command(nrf::cmd::FLUSH_RX);
    nrf.setbit(nrf::reg::STATUS,nrf::bit::status::RX_DR);//write one to clear status bit
    nrf.clearbit(nrf::reg::CONFIG,nrf::bit::config::MASK_RX_DR);//enable Rx DR interrupt

    nrf.disableAutoAcknowledge();
    nrf.disableRetransmission();

    nrf.enableRxPipes(nrf::bit::en_rxadd::ERX_P0);
    nrf.setPipeWidth(0,32);


    pser->printf("Power Up\r\n");
    nrf.setMode(nrf::Mode::Standby);//PowerUp


    pser->printf("set_DataRate(2Mbps)\r\n");
    nrf.setDataRate(nrf::datarate::d_2Mbps);
    pser->printf("set_CrcConfig(NoCrc)\r\n");
    nrf.setCrcConfig(nrf::crc::NoCrc);
    pser->printf("set_RF_Channel(2) 2402 MHz\r\n");
    nrf.selectChannel(2);

    /*pser->printf("setTxAddress()\r\n");
    nrf.setTxAddress(DEFAULT_NRF24L01P_ADDRESS,5);
    pser->printf("setRxAddress()\r\n");
    nrf.setRxAddress(DEFAULT_NRF24L01P_ADDRESS,5);
    pser->printf("setCrcWidth()\r\n");
    */


    nrf.setMode(nrf::Mode::Rx);
    nrf.ce_pin_highEnable();
    
}


void RfMesh::attach(Callback<void(uint8_t *data,uint8_t size)> func,RfMesh::CallbackType type)
{
    _callbacks[static_cast<int>(type)] = func;
}

void RfMesh::print_nrf()
{
    nrf.dump_regs();
    nrf.print_info();
    int irq_status = nRFIrq.read();
    pser->printf("irq pin %d\n",irq_status);
}

