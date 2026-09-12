#include "rs485.h"

// RS485驱动：收发器自动切换方向，MCU侧即普通USART1
// 接收：DMA0_CH5循环模式+环形缓冲，rs485_getdata()轮询取数（不使用IDLE中断）
// 发送：DMA0_CH6+环形缓冲，TC中断链式搬运，rs485_senddata()拷入即返回

// 接收环形缓冲区
static uint8_t rx_buffer[RS485_RX_BUFFER_SIZE];
volatile uint16_t rs485_rx_write_pos = 0;	//DMA写入位置（总长度-剩余计数）
static volatile uint16_t rx_read_pos = 0;	//已取走数据的读位置

// 发送环形缓冲区
static uint8_t tx_buffer[RS485_TX_BUFFER_SIZE];
static volatile uint16_t tx_write_pos = 0;	//软件写位置（任务推进）
static volatile uint16_t tx_read_pos = 0;	//DMA发送位置（中断推进）
static volatile uint16_t tx_chunk = 0;		//当前DMA正在发送的长度
static volatile uint8_t tx_busy = 0;		//发送状态：1忙，0空闲

// 启动一段连续数据的DMA发送（任务和中断都可调用）
static void tx_kick(void)
{
	uint16_t chunk;
	uint32_t primask;

	primask = __get_PRIMASK();
	__disable_irq();

	// 正在发送则退出（TC中断会链式调用本函数继续发）
	if(tx_busy) {
		if(0 == primask) {
			__enable_irq();
		}
		return;
	}

	// 计算可发送的连续数据长度（处理环形回绕：到缓冲区末尾为止，剩余部分由TC中断续发）
	if(tx_read_pos <= tx_write_pos) {
		chunk = tx_write_pos - tx_read_pos;
	} else {
		chunk = RS485_TX_BUFFER_SIZE - tx_read_pos;
	}

	// 无数据
	if(0 == chunk) {
		if(0 == primask) {
			__enable_irq();
		}
		return;
	}

	// 启动DMA发送
	tx_chunk = chunk;
	tx_busy = 1;
	dma_channel_disable(DMA0, DMA_CH6);
	dma_memory_address_config(DMA0, DMA_CH6, DMA_MEMORY_0, (uint32_t)&tx_buffer[tx_read_pos]);
	dma_transfer_number_config(DMA0, DMA_CH6, chunk);
	dma_flag_clear(DMA0, DMA_CH6, DMA_FLAG_FTF);
	usart_flag_clear(USART1, USART_FLAG_TC);
	dma_channel_enable(DMA0, DMA_CH6);
	usart_interrupt_enable(USART1, USART_INT_TC);

	if(0 == primask) {
		__enable_irq();
	}
}

// DMA收发配置（USART1_RX=DMA0_CH5，USART1_TX=DMA0_CH6）
static void dma_config(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	// 配置DMA0通道5：USART1接收，循环模式，无需重启
	dma_deinit(DMA0, DMA_CH5);
	dma_single_data_para_struct_init(&dma_init_struct);
	dma_init_struct.direction   = DMA_PERIPH_TO_MEMORY;
	dma_init_struct.memory0_addr = (uint32_t)rx_buffer;
	dma_init_struct.memory_inc  = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.number      = RS485_RX_BUFFER_SIZE;
	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART1);
	dma_init_struct.periph_inc  = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
	dma_init_struct.priority    = DMA_PRIORITY_HIGH;
	dma_single_data_mode_init(DMA0, DMA_CH5, &dma_init_struct);
	dma_channel_subperipheral_select(DMA0, DMA_CH5, DMA_SUBPERI4);
	dma_channel_enable(DMA0, DMA_CH5);

	// 配置DMA0通道6：USART1发送，由tx_kick()设置内存地址和数量
	dma_deinit(DMA0, DMA_CH6);
	dma_single_data_para_struct_init(&dma_init_struct);
	dma_init_struct.direction   = DMA_MEMORY_TO_PERIPH;
	dma_init_struct.memory0_addr = 0;
	dma_init_struct.memory_inc  = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.number      = 0;
	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART1);
	dma_init_struct.periph_inc  = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
	dma_init_struct.priority    = DMA_PRIORITY_HIGH;
	dma_single_data_mode_init(DMA0, DMA_CH6, &dma_init_struct);
	dma_channel_subperipheral_select(DMA0, DMA_CH6, DMA_SUBPERI4);
}

void rs485_init(void)
{
	// 使能时钟
	rcu_periph_clock_enable(RS485_CLOCK1);
	rcu_periph_clock_enable(RS485_CLOCK2);
	rcu_periph_clock_enable(RCU_DMA0);

	// USART1收发引脚：TX=PD5、RX=PD6，复用AF7
	gpio_mode_set(RS485_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,RS485_PIN_TX);
	gpio_af_set(RS485_PORT,GPIO_AF_7,RS485_PIN_TX);
	gpio_mode_set(RS485_PORT,GPIO_MODE_AF,GPIO_PUPD_PULLUP,RS485_PIN_RX);
	gpio_af_set(RS485_PORT,GPIO_AF_7,RS485_PIN_RX);

	// 串口配置：RS485_BAUDRATE 8N1
	usart_deinit(USART1);
	usart_baudrate_set(USART1,RS485_BAUDRATE);
	usart_parity_config(USART1,USART_PM_NONE);
	usart_word_length_set(USART1,USART_WL_8BIT);
	usart_stop_bit_set(USART1,USART_STB_1BIT);
	usart_transmit_config(USART1,USART_TRANSMIT_ENABLE);
	usart_receive_config(USART1,USART_RECEIVE_ENABLE);

	// 配置DMA收发，并使能USART的DMA收发请求
	dma_config();
	usart_dma_receive_config(USART1,USART_RECEIVE_DMA_ENABLE);
	usart_dma_transmit_config(USART1,USART_TRANSMIT_DMA_ENABLE);

	// 优先级5 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY，中断内可安全调用FreeRTOS FromISR接口
	nvic_irq_enable(USART1_IRQn,5,0);

	usart_enable(USART1);
}

// 非阻塞发送：数据拷入内部环形缓冲后立即返回，TC中断链式搬运
// 返回实际入队长度；缓冲满时截断（返回值<len表示部分丢弃）
uint16_t rs485_senddata(uint8_t *send_buffer, uint16_t length)
{
	uint16_t i, free_cnt, queued;
	uint32_t primask;

	// 计算缓冲区剩余空间（保留1字节区分满/空）
	primask = __get_PRIMASK();
	__disable_irq();
	if(tx_write_pos >= tx_read_pos) {
		free_cnt = RS485_TX_BUFFER_SIZE - 1 - (tx_write_pos - tx_read_pos);
	} else {
		free_cnt = tx_read_pos - tx_write_pos - 1;
	}
	__enable_irq();

	// 缓冲满时截断
	queued = (length <= free_cnt) ? length : free_cnt;

	// 拷入环形缓冲（处理回绕）
	for(i = 0; i < queued; i++) {
		tx_buffer[tx_write_pos] = send_buffer[i];
		tx_write_pos = (tx_write_pos + 1) % RS485_TX_BUFFER_SIZE;
	}

	// 尝试启动发送（忙则由TC中断续发）
	tx_kick();

	return queued;
}

// 查询发送是否忙（1忙，0空闲）
uint8_t rs485_sendbusy(void)
{
	return tx_busy;
}

// 取走已接收数据（环形缓冲），返回帧长度（0表示无数据）
uint16_t rs485_getdata(uint8_t *recv_buffer, uint16_t length)
{
	uint16_t i, w, len;

	// 轮询更新DMA写位置（总长度-剩余计数）
	rs485_rx_write_pos = RS485_RX_BUFFER_SIZE - (uint16_t)dma_transfer_number_get(DMA0, DMA_CH5);

	// 一次性读取写位置，保证数据一致
	w = rs485_rx_write_pos;

	// 计算未取走数据长度（环形）
	if(w >= rx_read_pos) {
		len = w - rx_read_pos;
	} else {
		len = RS485_RX_BUFFER_SIZE - rx_read_pos + w;
	}

	// 无新数据
	if(0 == len) {
		return 0;
	}

	// 用户缓冲区不够时截断
	if(len > length) {
		len = length;
	}

	// 拷贝数据（处理环形回绕）
	for(i = 0; i < len; i++) {
		recv_buffer[i] = rx_buffer[(rx_read_pos + i) % RS485_RX_BUFFER_SIZE];
	}

	// 更新读位置
	rx_read_pos = (rx_read_pos + len) % RS485_RX_BUFFER_SIZE;

	return len;
}

// USART1中断服务函数：TC中断推进发送位置并链式发送剩余数据
void USART1_IRQHandler(void)
{
	// 发送完成中断：DMA数据全部移出
	if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_TC) != RESET)
	{
		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_TC);
		usart_interrupt_disable(USART1, USART_INT_TC);

		// 推进已发送位置
		tx_read_pos = (tx_read_pos + tx_chunk) % RS485_TX_BUFFER_SIZE;
		tx_busy = 0;

		// 缓冲区还有数据则继续发送
		tx_kick();
	}
}
