#include "rc522_config.h"
#include "stm32f10x.h"

static void             RC522_SPI_Config             ( void );

void RC522_Init ( void )
{
	RC522_SPI_Config ();
	
	macRC522_Reset_Disable();
	
	macRC522_CS_Disable();
}

static void RC522_SPI_Config ( void )
{
  /* SPI_InitTypeDef  SPI_InitStructure */
  GPIO_InitTypeDef GPIO_InitStructure;
  

	/*!< Configure SPI_RC522_SPI pins: CS */
  macRC522_GPIO_CS_CLK_FUN ( macRC522_GPIO_CS_CLK, ENABLE );
  GPIO_InitStructure.GPIO_Pin = macRC522_GPIO_CS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = macRC522_GPIO_CS_Mode;
  GPIO_Init(macRC522_GPIO_CS_PORT, &GPIO_InitStructure);
	
  /*!< Configure SPI_RC522_SPI pins: SCK */
	macRC522_GPIO_SCK_CLK_FUN ( macRC522_GPIO_SCK_CLK, ENABLE );
  GPIO_InitStructure.GPIO_Pin = macRC522_GPIO_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = macRC522_GPIO_SCK_Mode;
  GPIO_Init(macRC522_GPIO_SCK_PORT, &GPIO_InitStructure);
	
  /*!< Configure SPI_RC522_SPI pins: MOSI */
	macRC522_GPIO_MOSI_CLK_FUN ( macRC522_GPIO_MOSI_CLK, ENABLE );
  GPIO_InitStructure.GPIO_Pin = macRC522_GPIO_MOSI_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = macRC522_GPIO_MOSI_Mode;
  GPIO_Init(macRC522_GPIO_MOSI_PORT, &GPIO_InitStructure);

  /*!< Configure SPI_RC522_SPI pins: MISO */
	macRC522_GPIO_MISO_CLK_FUN ( macRC522_GPIO_MISO_CLK, ENABLE );
  GPIO_InitStructure.GPIO_Pin = macRC522_GPIO_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = macRC522_GPIO_MISO_Mode;
  GPIO_Init(macRC522_GPIO_MISO_PORT, &GPIO_InitStructure);	
	
	
  /*!< Configure SPI_RC522_SPI pins: RST */
	macRC522_GPIO_RST_CLK_FUN ( macRC522_GPIO_RST_CLK, ENABLE );
  GPIO_InitStructure.GPIO_Pin = macRC522_GPIO_RST_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = macRC522_GPIO_RST_Mode;
  GPIO_Init(macRC522_GPIO_RST_PORT, &GPIO_InitStructure);
}


