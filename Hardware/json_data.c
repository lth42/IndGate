#include "json_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 通道名转通道编号，无效返回JSON_CH_INVALID
static uint8_t json_ch_index(const char *name)
{
	if(0 == strcmp(name, "can")) {
		return JSON_CH_CAN;
	}
	if(0 == strcmp(name, "rs485")) {
		return JSON_CH_RS485;
	}
	if(0 == strcmp(name, "rs232")) {
		return JSON_CH_RS232;
	}
	if(0 == strcmp(name, "do")) {
		return JSON_CH_DO;
	}
	return JSON_CH_INVALID;
}

// 单个hex字符转数值，无效返回-1
static int json_hex_val(char c)
{
	if((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	if((c >= 'A') && (c <= 'F')) {
		return c - 'A' + 10;
	}
	if((c >= 'a') && (c <= 'f')) {
		return c - 'a' + 10;
	}
	return -1;
}

// 生成数据上报报文，返回实际长度（返回值>=buf_size表示缓冲区不足被截断）
int json_build_report(char *buf, int buf_size, const json_report_t *rep)
{
	int n, i, j;
	const json_point_t *pt;

	if((NULL == buf) || (NULL == rep) || (buf_size <= 0)) {
		return -1;
	}

	// 报文头 + 继电器状态
	n = snprintf(buf, buf_size,
		"{\"type\":\"" JSON_TYPE_REPORT "\",\"devid\":\"%s\",\"time\":%lu,\"do\":[%u,%u,%u,%u],\"data\":[",
		rep->devid, (unsigned long)rep->time,
		rep->do_status[0], rep->do_status[1], rep->do_status[2], rep->do_status[3]);
	if(n >= buf_size) {
		return buf_size;
	}

	// 数据点
	for(i = 0; i < rep->point_num; i++) {
		pt = &rep->points[i];

		// 点间分隔
		if(i > 0) {
			n += snprintf(buf + n, buf_size - n, ",");
			if(n >= buf_size) {
				return buf_size;
			}
		}

		switch(pt->ch) {
		case JSON_CH_CAN:
			// CAN数据点：帧ID + hex数据
			n += snprintf(buf + n, buf_size - n, "{\"ch\":\"can\",\"id\":%u,\"data\":\"", pt->can_id);
			if(n >= buf_size) {
				return buf_size;
			}
			for(j = 0; j < pt->data_len; j++) {
				n += snprintf(buf + n, buf_size - n, "%02X", pt->data[j]);
				if(n >= buf_size) {
					return buf_size;
				}
			}
			n += snprintf(buf + n, buf_size - n, "\"}");
			break;
		case JSON_CH_RS485:
			// RS485数据点：设备地址 + 采集值
			n += snprintf(buf + n, buf_size - n, "{\"ch\":\"rs485\",\"addr\":%u,\"val\":\"%s\"}", pt->addr, pt->val);
			break;
		default:
			// RS232数据点：采集值
			n += snprintf(buf + n, buf_size - n, "{\"ch\":\"rs232\",\"val\":\"%s\"}", pt->val);
			break;
		}
		if(n >= buf_size) {
			return buf_size;
		}
	}

	// 报文尾
	n += snprintf(buf + n, buf_size - n, "]}");
	return n;
}

// 生成命令应答报文，返回实际长度
int json_build_ack(char *buf, int buf_size, const json_ack_t *ack)
{
	if((NULL == buf) || (NULL == ack) || (buf_size <= 0)) {
		return -1;
	}

	return snprintf(buf, buf_size,
		"{\"type\":\"" JSON_TYPE_ACK "\",\"devid\":\"%s\",\"cmd\":\"%s\",\"result\":%d}",
		ack->devid, ack->cmd, ack->result);
}

// 解析服务器下发的命令报文，成功返回0，失败返回-1
int json_parse_cmd(const char *str, json_cmd_t *cmd)
{
	const char *p;
	char ch_name[8];
	int i, h0, h1;

	if((NULL == str) || (NULL == cmd)) {
		return -1;
	}

	memset(cmd, 0, sizeof(json_cmd_t));

	// 校验报文类型
	if(NULL == strstr(str, "\"type\":\"" JSON_TYPE_CMD "\"")) {
		return -1;
	}

	// 提取devid
	p = strstr(str, "\"devid\":\"");
	if(NULL == p) {
		return -1;
	}
	p += strlen("\"devid\":\"");
	for(i = 0; (i < JSON_DEV_ID_LEN) && (p[i] != '\"') && (p[i] != '\0'); i++) {
		cmd->devid[i] = p[i];
	}
	cmd->devid[i] = '\0';

	// 提取cmd
	p = strstr(str, "\"cmd\":\"");
	if(NULL == p) {
		return -1;
	}
	p += strlen("\"cmd\":\"");
	for(i = 0; (i < JSON_CMD_NAME_LEN) && (p[i] != '\"') && (p[i] != '\0'); i++) {
		cmd->cmd[i] = p[i];
	}
	cmd->cmd[i] = '\0';

	// 提取index（do命令：继电器序号）
	p = strstr(str, "\"index\":");
	cmd->index = (NULL != p) ? (uint8_t)atoi(p + strlen("\"index\":")) : 0;

	// 提取value（do命令：目标状态）
	p = strstr(str, "\"value\":");
	cmd->value = (NULL != p) ? (uint8_t)atoi(p + strlen("\"value\":")) : 0;

	// 提取ch（send命令：通道名转编号）
	p = strstr(str, "\"ch\":\"");
	if(NULL != p) {
		p += strlen("\"ch\":\"");
		for(i = 0; (i < 7) && (p[i] != '\"') && (p[i] != '\0'); i++) {
			ch_name[i] = p[i];
		}
		ch_name[i] = '\0';
		cmd->ch = json_ch_index(ch_name);
	} else {
		cmd->ch = JSON_CH_INVALID;
	}

	// 提取data（send命令：hex字符串转字节）
	p = strstr(str, "\"data\":\"");
	if(NULL != p) {
		p += strlen("\"data\":\"");
		cmd->data_len = 0;
		while((cmd->data_len < JSON_SEND_DATA_MAX)
			&& ((h0 = json_hex_val(p[0])) >= 0)
			&& ((h1 = json_hex_val(p[1])) >= 0)) {
			cmd->data[cmd->data_len] = (uint8_t)((h0 << 4) | h1);
			cmd->data_len++;
			p += 2;
		}
	} else {
		cmd->data_len = 0;
	}

	return 0;
}
