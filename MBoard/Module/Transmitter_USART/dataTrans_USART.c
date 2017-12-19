#include <dataTrans_USART.h>

const uint8_t TestCMD_PAGSIZE = 20;
const uint8_t TestCMD_MAXSIZE = 80;

const char *TestCMD[TestCMD_PAGSIZE] = {

	"公示底板按键调试信息",
	"隐藏底板按键调试信息",
	"公示红外转发扩展板按键调试信息",
	"隐藏红外转发扩展板按键调试信息",
};

const char *TestREP[TestCMD_PAGSIZE] = {

	"底板按键调试信息已公示\r\n",
	"底板按键调试信息已隐藏\r\n",
	"红外转发扩展板按键调试信息已公示\r\n",
	"红外转发扩展板按键调试信息已隐藏\r\n",
};

funDebug funDebugTab[] = {funDB_keyMB_ON,funDB_keyMB_OFF,funDB_keyIFR_ON,funDB_keyIFR_OFF};

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;

extern osThreadId tid_keyMboard_Thread;	//声明主板按键任务ID，便于传递信息调试使能信号
extern osThreadId tid_keyIFR_Thread;	//声明红外转发扩展板按键任务ID，便于传递信息调试使能信号

osThreadId tid_USARTDebug_Thread;
osThreadId tid_USARTWireless_Thread;

osThreadDef(USARTDebug_Thread,osPriorityNormal,1,1024);
osThreadDef(USARTWireless_Thread,osPriorityNormal,1,1024);

osMutexDef (uart_mutex);    // Declare mutex

void funDB_keyMB_ON(void){  // 开启主板按键Debug_log

	osSignalSet (tid_keyMboard_Thread, KEY_DEBUG_ON);
}

void funDB_keyMB_OFF(void){ // 关闭主板按键Debug_log

	osSignalSet (tid_keyMboard_Thread, KEY_DEBUG_OFF);
}

void funDB_keyIFR_ON(void){  // 开启红外转发扩展板按键Debug_log

	osSignalSet (tid_keyIFR_Thread, KEY_DEBUG_ON);
}

void funDB_keyIFR_OFF(void){ // 关闭红外转发扩展板按键Debug_log

	osSignalSet (tid_keyIFR_Thread, KEY_DEBUG_OFF);
}

void USART1Debug_Init(void){

	/*Initialize the USART driver */
	Driver_USART1.Initialize(myUSART1_callback);
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

	Driver_USART1.Send("i'm usart1 for debug log\r\n", 26);
}

void USART2Wirless_Init(void){
	
	/*Initialize the USART driver */
	Driver_USART2.Initialize(myUSART2_callback);
	/*Power up the USART peripheral */
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 4800 Bits/sec */
	Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART2.Control (ARM_USART_CONTROL_TX, 1);
	Driver_USART2.Control (ARM_USART_CONTROL_RX, 1);

	Driver_USART2.Send("i'm usart2 for wireless datstransfor\r\n", 38);
}

void myUSART1_callback(uint32_t event)
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

void myUSART2_callback(uint32_t event){

	;
}

void USARTDebug_Thread(const void *argument){

	const uint8_t cmdsize = TestCMD_MAXSIZE;
	char cmd[cmdsize] = "abc";
	uint8_t loop;
	
	for(;;){

		osDelay(10);													//必需延时，防乱序
		Driver_USART1.Receive(cmd,TestCMD_MAXSIZE);
		
		for(loop = 0; loop < 4; loop ++){
		
			if(strstr(cmd,TestCMD[loop])){							//子串比较
			//if(!strcmp(TestCMD[loop],cmd)){						//全等比较
			
				osDelay(10);
					
				funDebugTab[loop]();
				Driver_USART1.Send((char*)(TestREP[loop]),strlen((char*)(TestREP[loop])));
				memset(cmd,0,cmdsize*sizeof(char));
			}
		}
	}
}

void USARTWireless_Thread(const void *argument){

//	const uint8_t cmdsize = 60;
//	char cmd[cmdsize] = "abc";
	
	for(;;){
	
		Driver_USART2.Send("哈哈哈哈\r\n",10);
		osDelay(1500);
	}
}

void USART_allInit(void){

	USART1Debug_Init();
	USART2Wirless_Init();
}

void USARTthread_Active(void){
	
	USART_allInit();
	
	tid_USARTDebug_Thread 		= osThreadCreate(osThread(USARTDebug_Thread),NULL);
	tid_USARTWireless_Thread	= osThreadCreate(osThread(USARTWireless_Thread),NULL);
}
