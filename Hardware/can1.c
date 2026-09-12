#include "can1.h"

// CAN1非阻塞驱动：发送3个硬件邮箱自动排队（邮箱满时senddata返回0由调用方重试）
// 接收硬件FIFO0（3帧深度），can1_getdata()轮询取帧（不使用中断，风格与rs232/ethernet一致）

void can1_init(void)
{
	can_parameter_struct can_init_struct;
	can_filter_parameter_struct can_filter_struct;

	// 使能时钟：CAN1与CAN0共享过滤器SRAM，两者都必须使能
	rcu_periph_clock_enable(RCU_CAN0);
	rcu_periph_clock_enable(RCU_CAN1);
	rcu_periph_clock_enable(CAN1_CLOCK);
	rcu_periph_clock_enable(RCU_GPIOB);

	// CAN1引脚：TX=PB6、RX=PB5，复用AF9
	gpio_mode_set(CAN1_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,CAN1_PIN_TX);
	gpio_af_set(CAN1_PORT,GPIO_AF_9,CAN1_PIN_TX);
	gpio_output_options_set(CAN1_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,CAN1_PIN_TX);
	gpio_mode_set(CAN1_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,CAN1_PIN_RX);
	gpio_af_set(CAN1_PORT,GPIO_AF_9,CAN1_PIN_RX);

	can_deinit(CAN1);

	// CAN1初始化：正常模式，500kbps（APB1=42MHz，prescaler=6，1+10+3=14tq，采样点78.6%）
	can_init_struct.working_mode = CAN_NORMAL_MODE;
	can_init_struct.resync_jump_width = CAN_BT_SJW_1TQ;
	can_init_struct.time_segment_1 = CAN_BT_BS1_10TQ;
	can_init_struct.time_segment_2 = CAN_BT_BS2_3TQ;
	can_init_struct.time_triggered = DISABLE;
	can_init_struct.auto_bus_off_recovery = ENABLE;
	can_init_struct.auto_wake_up = DISABLE;
	can_init_struct.auto_retrans = ENABLE;
	can_init_struct.rec_fifo_overwrite = DISABLE;
	can_init_struct.trans_fifo_order = DISABLE;
	can_init_struct.prescaler = 6;
	can_init(CAN1, &can_init_struct);

	// 过滤器：32位掩码模式，掩码全0=接收所有帧，关联FIFO0（过滤器组CAN0/CAN1共享）
	can_filter_struct.filter_list_high = 0x0000;
	can_filter_struct.filter_list_low = 0x0000;
	can_filter_struct.filter_mask_high = 0x0000;
	can_filter_struct.filter_mask_low = 0x0000;
	can_filter_struct.filter_fifo_number = CAN_FIFO0;
	can_filter_struct.filter_number = 0;
	can_filter_struct.filter_mode = CAN_FILTERMODE_MASK;
	can_filter_struct.filter_bits = CAN_FILTERBITS_32BIT;
	can_filter_struct.filter_enable = ENABLE;
	can_filter_init(&can_filter_struct);
}

// 非阻塞发送：标准数据帧，数据拷入硬件邮箱后立即返回
// 返回1成功入队邮箱，0=3个邮箱全满（调用方稍后重试）
uint8_t can1_senddata(uint32_t frame_id, uint8_t *data, uint8_t len)
{
	can_transmit_message_struct tx_msg;
	uint8_t i;

	// CAN帧数据最长8字节
	if(len > 8) {
		len = 8;
	}

	tx_msg.tx_sfid = frame_id & 0x7FF;
	tx_msg.tx_ff = CAN_FF_STANDARD;
	tx_msg.tx_ft = CAN_FT_DATA;
	tx_msg.tx_dlen = len;
	for(i = 0; i < 8; i++) {
		tx_msg.tx_data[i] = (i < len) ? data[i] : 0;
	}

	if(CAN_NOMAILBOX == can_message_transmit(CAN1, &tx_msg)) {
		return 0;
	}
	return 1;
}

// 非阻塞接收：从FIFO0取一帧，返回1收到，0无数据
uint8_t can1_getdata(uint32_t *frame_id, uint8_t *data, uint8_t *len)
{
	can_receive_message_struct rx_msg;
	uint8_t i;

	// FIFO0无数据
	if(0 == can_receive_message_length_get(CAN1, CAN_FIFO0)) {
		return 0;
	}

	can_message_receive(CAN1, CAN_FIFO0, &rx_msg);
	*frame_id = rx_msg.rx_sfid;
	*len = rx_msg.rx_dlen;
	for(i = 0; i < 8; i++) {
		data[i] = rx_msg.rx_data[i];
	}

	return 1;
}
