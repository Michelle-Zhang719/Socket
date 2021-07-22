#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int err, n;
	char sendline[20], recvline[1024], newport[6];
	
	if (argc != 3)
	{
		printf("please input:%s host port\n", argv[0]);
		exit(-1);
	}
	printf("(Tips:input EXIT to close client)\n"); 
	while (1)
	{
		//创建文件名socket + 配置信息+ 连接服务器 
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1)
		{
			printf("socket error\n");
			return -1;
		}
		struct sockaddr_in addr_ser;
		bzero(&addr_ser, sizeof(addr_ser));
		addr_ser.sin_family = AF_INET;
		addr_ser.sin_port = htons(atoi(argv[2]));
		inet_pton(AF_INET, argv[1], &addr_ser.sin_addr);
		
		err = connect(sockfd, (struct sockaddr *)&addr_ser, sizeof(addr_ser));
		if (err == -1)//连接失败 
		{
			printf("connect error\n");
			return -1;
		}
		printf("-----------------------------------------------------------------------\n"); 
		printf("Please enter the file name you want to request:");
		scanf("%s", sendline);
		if (strcmp(sendline, "EXIT") == 0)
		{
			break;
		}
		if(send(sockfd, sendline, strlen(sendline), 0)>0)//发送文件名 
		{
			printf("File name send successfully\n");
		}
		if ((n = recv(sockfd, newport, 6, 0))>0)//接收端口号 
		{
			//创建文件内容socket + 配置信息+ 连接服务器 
			int sockfile = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfile == -1)
			{
				printf("socket error\n");
				return -1;
			}
			struct sockaddr_in addr_file;
			bzero(&addr_file, sizeof(addr_file));
			addr_file.sin_family = AF_INET;
			addr_file.sin_port = htons(atoi(newport));
			inet_pton(AF_INET, argv[1], &addr_file.sin_addr);
			err = connect(sockfile, (struct sockaddr *)&addr_file, sizeof(addr_file));
			if (err == -1)
			{
				printf("connect error\n");
				return -1;
			}
			//接收文件内容 
			printf("response from port:%d\n", ntohs(addr_file.sin_port));
			printf("===============================\n");
			while ((n = recv(sockfile, recvline, 1024, 0))>0)
			{
				recvline[n] = '\0';
				printf("%s", recvline);
			}
			close(sockfile);
		}
		close(sockfd);
	}

	return 0;
}
