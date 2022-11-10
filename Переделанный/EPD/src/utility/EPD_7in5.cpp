#include "EPD_7in5.h"
#include "DEBUG.h"
#include "SPI_RAM.h"

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_7IN5_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(200);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_7IN5_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_7IN5_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD_7IN5_ReadBusy(void)
{
    DEBUG("e-Paper busy\r\n");
    while(DEV_Digital_Read(EPD_BUSY_PIN) == 0) {      //LOW: idle, HIGH: busy
        DEV_Delay_ms(100);
    }
    DEBUG("e-Paper busy release\r\n");
}


/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_7IN5_TurnOnDisplay(void)
{
    EPD_7IN5_SendCommand(0x12); // DISPLAY_REFRESH
    DEV_Delay_ms(100);
    EPD_7IN5_ReadBusy();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_7IN5_Init(void)
{
    EPD_7IN5_Reset();

		EPD_7IN5_SendCommand(0x01); // POWER_SETTING
    EPD_7IN5_SendData(0x37);
    EPD_7IN5_SendData(0x00);

    EPD_7IN5_SendCommand(0x00); // PANEL_SETTING
    EPD_7IN5_SendData(0xCF);
    EPD_7IN5_SendData(0x08);

    EPD_7IN5_SendCommand(0x06); // BOOSTER_SOFT_START
    EPD_7IN5_SendData(0xc7);
    EPD_7IN5_SendData(0xcc);
    EPD_7IN5_SendData(0x28);

    EPD_7IN5_SendCommand(0x04); // POWER_ON
    EPD_7IN5_ReadBusy();

    EPD_7IN5_SendCommand(0x30); // PLL_CONTROL
    EPD_7IN5_SendData(0x3c);

    EPD_7IN5_SendCommand(0x41); // TEMPERATURE_CALIBRATION
    EPD_7IN5_SendData(0x00);

    EPD_7IN5_SendCommand(0x50); // VCOM_AND_DATA_INTERVAL_SETTING
    EPD_7IN5_SendData(0x77);

    EPD_7IN5_SendCommand(0x60); // TCON_SETTING
    EPD_7IN5_SendData(0x22);

    EPD_7IN5_SendCommand(0x61); // TCON_RESOLUTION
    EPD_7IN5_SendData(EPD_7IN5_WIDTH >> 8); // source 640
    EPD_7IN5_SendData(EPD_7IN5_WIDTH & 0xff);
    EPD_7IN5_SendData(EPD_7IN5_HEIGHT >> 8); // gate 384
    EPD_7IN5_SendData(EPD_7IN5_HEIGHT & 0xff);

    EPD_7IN5_SendCommand(0x82); // VCM_DC_SETTING
    EPD_7IN5_SendData(0x1E); // decide by LUT file

    EPD_7IN5_SendCommand(0xe5); // FLASH MODE
    EPD_7IN5_SendData(0x03);

}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_7IN5_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_7IN5_WIDTH % 8 == 0)? (EPD_7IN5_WIDTH / 8 ): (EPD_7IN5_WIDTH / 8 + 1);
    Height = EPD_7IN5_HEIGHT;

    EPD_7IN5_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            for(UBYTE k = 0; k < 4; k++) {
                EPD_7IN5_SendData(0x33);
            }
        }
    }
    EPD_7IN5_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_7IN5_Display(void)
{
    UBYTE Data_Black, Data;
    UWORD Width, Height;
    Width = (EPD_7IN5_WIDTH % 8 == 0)? (EPD_7IN5_WIDTH / 8 ): (EPD_7IN5_WIDTH / 8 + 1);
    Height = EPD_7IN5_HEIGHT;

    EPD_7IN5_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            Data_Black = SPIRAM_RD_Byte(i + j * Width);
            for(UBYTE k = 0; k < 8; k++) {
                if(Data_Black & 0x80)
                    Data = 0x00;
                else
                    Data = 0x03;
                Data <<= 4;
                Data_Black <<= 1;
                k++;
                if(Data_Black & 0x80)
                    Data |= 0x00;
                else
                    Data |= 0x03;
                Data_Black <<= 1;
                EPD_7IN5_SendData(Data);
            }
        }
    }
    EPD_7IN5_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_7IN5_Sleep(void)
{
		DEBUG("Enter sleep mode\r\n");
    EPD_7IN5_SendCommand(0x02); // POWER_OFF
    EPD_7IN5_ReadBusy();
    EPD_7IN5_SendCommand(0x07); // DEEP_SLEEP
    EPD_7IN5_SendData(0XA5);;
}
