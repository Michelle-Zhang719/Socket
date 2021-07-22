#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
 #include <time.h>
 
void handler(int sig)
{
	
	while (waitpid(-1,  NULL,   WNOHANG) > 0)
	{
	}
}
int main(int argc, char *argv[])
{
	FILE * fp = NULL;
	if (argc != 2)
	{
		printf("please input:%s port\n", argv[0]);
	}
	unsigned short port = atoi(argv[1]);// 本地端口
	
	//创建文件名socket+ 配置本地网络信息+绑定+监听 
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket");
		exit(-1);
	}
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port); // 端口
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP

	int err_log = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if (err_log != 0)
	{
		perror("binding");
		close(sockfd);
		exit(-1);
	}

	err_log = listen(sockfd, 10);
	if (err_log != 0)
	{
		perror("listen");
		close(sockfd);
		exit(-1);
	}
	
	//创建文件内容socket+ 配置本地网络信息+绑定+监听 
	unsigned short fileport = 8080;//随机端口 
	srand(time(NULL));
	fileport = 8080 + rand() % 10;
	
	int sockfile = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfile < 0)
	{
		perror("socket");
		exit(-1);
	}
	struct sockaddr_in file_addr;
	bzero(&file_addr, sizeof(file_addr));
	file_addr.sin_family = AF_INET;
	file_addr.sin_port = htons(fileport); // 端口
	file_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP

	err_log = bind(sockfile, (struct sockaddr*)&file_addr, sizeof(file_addr));
	if (err_log != 0)
	{
		perror("binding");
		close(sockfile);
		exit(-1);
	}

	err_log = listen(sockfile, 10);
	if (err_log != 0)
	{
		perror("listen");
		close(sockfile);
		exit(-1);
	}

	signal(SIGCHLD,  handler); //处理僵尸子进程 	
	//主进程
	while (1)
	{
		char cli_ip[INET_ADDRSTRLEN] = { 0 };
		struct sockaddr_in client_addr;
		socklen_t cliaddr_len = sizeof(client_addr);

		int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &cliaddr_len);
		if (connfd < 0)
		{
			perror("accept");
			close(sockfd);
			exit(-1);
		}

		pid_t pid = fork();
		if (pid < 0)//创建失败 
		{
			perror("fork");
			_exit(-1);
		}
		else if (0 == pid)//子进程 
		{
			close(sockfd);//关闭监听套接字
			char filename[255] = { 0 };
			char buf[1024] = { 0 };
			int recv_len = 0;
			// 打印客户端的 ip 和端口  
			memset(cli_ip, 0, sizeof(cli_ip)); // 清空  
			inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
			printf("Receive a client ip=%s,port=%d  ",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			printf("Client process %d start.\n", getpid());
			//接收文件名 
			if ((recv_len = recv(connfd, filename, sizeof(filename), 0)) > 0)
			{
				printf("File name:%s received successfully!\n", filename); // 打印文件名 
				//发送端口号 
				char SendPort[7] = { 0 };
				sprintf(SendPort, "%d", fileport); 
				send(connfd, SendPort, strlen(SendPort), 0); 
				//accept 
				struct sockaddr_in fileclient_addr;
				socklen_t fileclient_len = sizeof(fileclient_addr);
				int connfile = accept(sockfile, (struct sockaddr*)&fileclient_addr, &fileclient_len);
				if (connfile < 0)
				{
					perror("accept");
					close(sockfile);
					exit(-1);
				}
				//while ((recv_len = recv(connfile, filename, sizeof(filename), 0)) > 0) 
				while(1)
				{
					fp = fopen(filename, "r");
					if (fp != NULL)
					{
						while (fgets(buf, sizeof(buf), fp) != NULL)
						{
							send(connfile, buf, strlen(buf), 0);
						}
						fclose(fp);
						printf("File content send successfully\n");
						break; 
					}
					else
					{
						char *str = "failed to open file.\n";
						send(connfile, str, strlen(str), 0);
						printf("%s",str);
						break;
					}
					close(connfile);
				}
			}	
			printf("Client port %d closed successfully!   ", ntohs(client_addr.sin_port));
			printf ("Client process %d exits successfully!\n",getpid());
		
			exit(0);
 
		}
		else if (pid > 0)//父进程 
		{
			printf("----------------------------------------------------------------\n");
			printf("parent process exit.\n");
			close(connfd);//关闭已连接套接字  
		}
	}
	close(sockfd);
	close(sockfile);
	return 0;
}
