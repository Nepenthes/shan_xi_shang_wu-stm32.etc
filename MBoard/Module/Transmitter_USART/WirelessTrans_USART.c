#include <WirelessTrans_USART.h>

const char *TestCMD[] = {

	"天王盖地虎"
};

const char *TestREP[] = {

	"小鸡炖蘑菇\r\n"
};

extern ARM_DRIVER_USART Driver_USART1;

osThreadId tid_USARTTest_Thread;

osThreadDef(USARTTest_Thread,osPriorityNormal,1,1024);

osMutexDef (uart_mutex);    // Declare mutex

void USARTInit1(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	/* USART1 GPIO config */
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	USART_Cmd(USART1, ENABLE);
}

void USARTInit2(void){
	
	/*Initialize the USART driver */
	Driver_USART1.Initialize(myUSART_callback);
	/*Power up the USART peripheral */
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 4800 Bits/sec */
	Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
	Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);

	Driver_USART1.Send("Press Enter to receive a message\n", 33);
	//Driver_USART1.Send((char*)(TestREP[0]),strlen((char*)(TestREP[0])));
}

void myUSART_callback(uint32_t event)
{
//  uint32_t mask;
//  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
//         ARM_USART_EVENT_TRANSFER_COMPLETE |
//         ARM_USART_EVENT_SEND_COMPLETE     |
//         ARM_USART_EVENT_TX_COMPLETE       ;
//  if (event & mask) {
////    /* Success: Wakeup Thread */
////    osSignalSet(tid_myUART_Thread, 0x01);
//  }
//  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
//    __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//  }
//  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
//    __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//  }
}

void USARTTest_Thread(const void *argument){

	char cmd[30] = "abc";
	osMutexId  (uart_mutex_id); // Mutex ID   
	
	uart_mutex_id = osMutexCreate  (osMutex (uart_mutex));
	
	for(;;){

		osDelay(10);													//必需延时，防乱序
		Driver_USART1.Receive(cmd,strlen(TestCMD[0]));
		if(strstr(cmd,TestCMD[0])){								//子串比较
		//if(!strcmp(TestCMD[0],cmd)){							//全等比较
		
			if (uart_mutex_id != NULL){
				
				Driver_USART1.Send((char*)(TestREP[0]),strlen((char*)(TestREP[0])));
				osMutexRelease(uart_mutex_id);
			}		
			memset(cmd,0,30*sizeof(char));
		}
	}
}

void USARTTest(void){
	
	tid_USARTTest_Thread = osThreadCreate(osThread(USARTTest_Thread),NULL);
}
