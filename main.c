#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERV_PORT                       (10000)
#define AIRKISS_UDP_BRD_NUM             1
#define AIRKISS_VALID_UDP_BRD_MIN_NUM   20

#define int32_t int
static int                g_sockfd = -1;
#define AIRKISS_LISTEN_PORT (12476)
static struct sockaddr_in g_udp_bdcast_addr;

static int32_t broadcast_message(void)
{
	int sockfd = -1;
	struct sockaddr_in dst_addr;
	char tx_buffer[5] = {'a', 'b', 'c', 'd', 'e'};
	int send_bytes = 0;
	int on = 1;
	uint32_t broadcast_addr = 0;
	uint32_t udp_broadcast_err_count = 0;


	/* Create UDP socket */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("create socket failed - cancel udp broadcast\n");
		return -1;
	}

	printf("Create socket successfully.\n");

	/* Set udpfd option to broadcast */
	if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) {
		printf("Set socket option failed!\n");

		close(sockfd);
		return -1;
	}
	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.sin_family      = AF_INET;
	dst_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);    
	dst_addr.sin_port        = htons(SERV_PORT);

	printf("Broadcast ip addr: , htonl: 0x%x.\n", dst_addr.sin_addr.s_addr);

	for (int i = 0; i < 2*AIRKISS_UDP_BRD_NUM; i++) {
		send_bytes = sendto(sockfd, tx_buffer, 5, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
		if (send_bytes < 0)
		{
			printf("Send UDP broadcast packet %d failed, errno: %d!\n", i + 1, errno);
			udp_broadcast_err_count++;
		}

		usleep(10000);
	}

	printf("Send UDP broadcast %s, udp_broadcast_err_count: %d.\n", 
			(0 == udp_broadcast_err_count) ? "successfully" : "failed", udp_broadcast_err_count);

	close(sockfd);

	if ((AIRKISS_UDP_BRD_NUM - udp_broadcast_err_count) < AIRKISS_VALID_UDP_BRD_MIN_NUM)
	{
		printf("Send airkiss UDP broadcast to port %d failed!\n", SERV_PORT);
		return -1;
	}

	return 0;
}

/* Store airkiss lan ssdp response packet */
static uint8_t  lan_resp_buf[200] = {"123456789"};
static uint16_t lan_resp_buf_len = sizeof(lan_resp_buf);

static void udp_bdcast_send()
{
	int ret;
	int send_bytes = 0;
	
	while (1)
	{
		//memset(lan_resp_buf, 0, sizeof(lan_resp_buf));
		lan_resp_buf_len = sizeof(lan_resp_buf);
			printf("lan_resp_buf_len: %u.\n", lan_resp_buf_len);
			
			/* Send response packet by UDP */
			send_bytes = sendto(g_sockfd, lan_resp_buf, lan_resp_buf_len, 0, (struct sockaddr *)&g_udp_bdcast_addr, sizeof(g_udp_bdcast_addr));
			if (send_bytes < 0)
			{
				printf("Send UDP broadcast packet failed, errno: %d!\n", errno);

				break;
			}

			printf("Send UDP broadcast packet successfully.\n");
			break;
	}

	return;
}

static void udp_bdcast_send_device_info()
{
	int on = 1;

	memset(&g_udp_bdcast_addr, 0, sizeof(g_udp_bdcast_addr));
	g_udp_bdcast_addr.sin_family      = AF_INET;
	g_udp_bdcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	g_udp_bdcast_addr.sin_port        = htons(AIRKISS_LISTEN_PORT);

	/* Set udpfd option to broadcast */
	if (-1 == setsockopt(g_sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) {
		printf("Set socket option failed!\n");
		return;
	}

	printf("Set udpfd option to broadcast successfully.\n");

	udp_bdcast_send();

}

/* Task entry */
void airkiss_lan_device_discover_main()
{
	struct sockaddr_in server_addr;

	printf("Enter task airkiss_lan_device_discover_main.\n");
	/* Create UDP socket */
	if ((g_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf("create socket failed - cancel udp broadcast\n");
		return ;
 	}

	printf("Create udp socket successfully.\n");

	printf("Send device info by udp broadcast.\n");

	udp_bdcast_send_device_info();
	close(g_sockfd);	
}

int main()
{
	broadcast_message();
	airkiss_lan_device_discover_main();
	return 0;
}
