/*****************************************************
ip108 source code -- lht
******************************************************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<time.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>	//close()
#include<netinet/in.h>	//struct sockaddr_in
#include<net/if.h>
#include<arpa/inet.h>	//inet_ntoa

#define DEST_PORT 3002

//#define DEST_IP_ADDRESS "192.168.81.100"
#define DEST_IP_ADDRESS "202.100.171.173"

#define NET_INTERFACE "eth0"

struct time_struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};


struct time_struct time_value;

void get_localtime(struct time_struct *time_v)
{
	struct tm *tm_now;
	time_t now;

	time(&now);
	tm_now = localtime(&now);

	time_v->year = 1900 + tm_now->tm_year;
	time_v->month = 1 + tm_now->tm_mon;
	time_v->day = tm_now->tm_mday;
	time_v->hour = tm_now->tm_hour;
	time_v->minute = tm_now->tm_min;
	time_v->second = tm_now->tm_sec;
	return;
}


void process_info(int s)
{
	int send_num;
	int recv_num;
	int ref_id, i;
	struct ifreq if_req;
	struct time_struct time_data;
	//char imei[] = "123456789012345678";
	char iccid[] = "1111111111111111111";

	char send_buf[500]={0};
	char recv_buf[100]={0};

	char serial_id[30];
	char command[10];
	char status[10];

	ref_id = 0;

	while(1){		
		strcpy(if_req.ifr_name, NET_INTERFACE);
		
		ioctl(s, SIOCGIFHWADDR, &if_req);

		get_localtime(&time_data);

		sprintf(send_buf,"[FFFF%02X%02X%02X%02X%02X%02X,%04d%02d%02d%02d%02d%02d%04d,49,%s,0,100%%,0@0@0,0]",
				(unsigned char)if_req.ifr_hwaddr.sa_data[0],
				(unsigned char)if_req.ifr_hwaddr.sa_data[1],
				(unsigned char)if_req.ifr_hwaddr.sa_data[2],
				(unsigned char)if_req.ifr_hwaddr.sa_data[3],
				(unsigned char)if_req.ifr_hwaddr.sa_data[4],
				(unsigned char)if_req.ifr_hwaddr.sa_data[5],
				time_data.year, time_data.month, time_data.day,
				time_data.hour, time_data.minute, time_data.second,
				ref_id,iccid);

		ref_id++;

		if(ref_id > 9999){
			ref_id = 0;
		}

		send_num = send(s,send_buf,strlen(send_buf),0);
		if (send_num < 0){
			perror("send");
			exit(1);
		} else {
			printf("send:%s\n",send_buf);
		}
		//printf("begin recv:\n");
		recv_num = recv(s,recv_buf,sizeof(recv_buf),0);
		if(recv_num < 0){
			perror("recv");
			exit(1);
		} else {
			recv_buf[recv_num]='\0';
			for(i = 0; i < recv_num; i++){
				if(recv_buf[i] == ','){
					recv_buf[i] = ' ';
				}
			}
			sscanf(recv_buf,"[%s %s %s]", serial_id, command, status);
			printf("recv:ref=%s command=%s status=%s\n",serial_id, command, status);
		}
		sleep(10);
	}
}

int main(int argc,char *argv[])
{
	int sock_fd;
	struct sockaddr_in addr_serv;
 
	sock_fd = socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd < 0){
		perror("sock");
		exit(1);
	} else {
		printf("sock sucessful:\n");
	}
	memset(&addr_serv,0,sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port =  htons(DEST_PORT);
	addr_serv.sin_addr.s_addr = inet_addr(DEST_IP_ADDRESS);
	if( connect(sock_fd,(struct sockaddr *)&addr_serv,sizeof(struct sockaddr)) < 0){
		perror("connect");
		printf("connect (%d)\n",errno);
		exit(1);
	} else {
		printf("connect sucessful\n");
	}
	process_info(sock_fd);
	close(sock_fd);
}

