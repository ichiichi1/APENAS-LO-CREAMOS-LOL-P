/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "udpecho.h"
#include "tcpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_pit.h"
#include "fsl_dac.h"

#define paquetes 200
#define PACKET_COMPARISON 2
#define DEMO_DAC_BASEADDR DAC0


uint16_t round_buf1[paquetes] = {0};
uint16_t round_buf2[paquetes] = {0};
uint16_t round_buf3[paquetes] = {0};

uint8_t buff_num = 0;

uint8_t dac_out = 0;
uint8_t dac_out_num = 0;

uint16_t paquetes_totales = 0;
uint8_t y = 0;
uint8_t error = 0;
uint32_t num = 0;

void PIT0_IRQHandler()
{
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	if(dac_out == 0)
		{
	    DAC_SetBufferValue(DEMO_DAC_BASEADDR, 0U, ((round_buf1[dac_out_num] >> 4)+2047));
			if(dac_out_num == paquetes)
			{
			dac_out++;
			dac_out_num = 0;
			}
		}
		else if(dac_out == 1)
		{
		DAC_SetBufferValue(DEMO_DAC_BASEADDR, 0U, ((round_buf2[dac_out_num] >> 4)+2047));
		if(dac_out_num == paquetes)
			{
			dac_out++;
			dac_out_num = 0;
			}
		}
		else if(dac_out == 2)
		{
		DAC_SetBufferValue(DEMO_DAC_BASEADDR, 0U, ((round_buf3[dac_out_num] >> 4)+2047));
		if(dac_out_num == paquetes)
			{
			dac_out++;
			dac_out_num = 0;
			}
		}
		dac_out_num++;
		paquetes_totales++;
}

static void
server_thread(void *arg)
{
	struct netconn *conn;
	struct netbuf *buf;

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	uint16_t puerto;

	//LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);

	while (1)
	{
		puerto = get_Port();
		netconn_bind(conn, IP_ADDR_ANY, puerto);

		netconn_recv(conn, &buf);
//		netbuf_data(buf, (void**)&data, &len);
		if(buff_num == 0)
				{
				netbuf_copy(buf,round_buf1,sizeof(round_buf1));
				buff_num = 1;
				}
				else if(buff_num == 1)
				{
				netbuf_copy(buf,round_buf2,sizeof(round_buf2));
				buff_num = 2;
				}
				else if(buff_num == 2)
				{
				netbuf_copy(buf,round_buf3,sizeof(round_buf3));
				buff_num = 0;
				}

				netbuf_delete(buf);

		//PIT_StartTimer(PIT, kPIT_Chnl_0);

	}
}

void
udpecho_init(void)
{
	sys_thread_new("server", server_thread, NULL, 300, 2);
}

#endif /* LWIP_NETCONN */
