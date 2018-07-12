// qy_douliu.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>
#include "gameServer.h"

#include <stdlib.h>
#include <fcntl.h>

bool single_instance(const char* pid_file)
{
	int fd = -1;
	char buf[32];
	fd = open(pid_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		return false;
	}
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	if (fcntl(fd, F_SETLK, &lock) < 0) {
		return false;
	}
	ftruncate(fd, 0);
	pid_t pid = getpid();
	int len = snprintf(buf, 32, "%d", pid);
	write(fd, buf, len);

	return true;
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
    /*
	struct rlimit rt;
	rt.rlim_cur = 1024;
	rt.rlim_max = 20480;
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
		perror("setrlimit failed.");
		return 0;
	}
    */

	GameServer* server = GameServer::GetInstance();
	if (server->InitServer("config.conf")) {
		server->StartServer();
	}
	else {
		printf("初始化服务失败.\n");
	}

	server->Release();
	return 0;
}
