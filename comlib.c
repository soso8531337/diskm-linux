#include "comlib.h"

int exe_default(void *cmdline)
{
	int i = 0 ;
	char *tmp = NULL;
	char *argv[1024] = {0};
	char buff[1024] = {0};
	char *param = NULL;

	strncpy(buff, cmdline, 1024);
	param = &buff[0];
	while ((tmp = strchr(param, ' ')) != NULL) {
		*tmp='\0';
		if (*(tmp-1) != '\0')
			argv[i++] = param;
		param=tmp;
		param++;
	}
	argv[i++] = param;
	argv[i] = NULL;

	if (execvp(argv[0], argv) == -1)
		exit(-1);
	return 0;
}

/*
	for example:
	1.exec_cmd("pppd"); //will wait progress exit.
	2.exec_cmd("pppd &");//do not wait progress exit.
*/
int exec_cmd(const char * fmt,...)
{
	int pid;
	int flag = 0;
	int wpid = 0;
	int waittime = 0;
	int timeout = 600;
	int statbuf;
	char tmp[1024] = {0};
	char cmdbuf[1024] = {0,};
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmdbuf, fmt, ap);
	va_end(ap);

	if (cmdbuf[strlen(cmdbuf) - 1] == '&') {
		strncpy(tmp, cmdbuf, strlen(cmdbuf) - 2);
		flag = 1;
	} else
		arr_strcpy(tmp, cmdbuf);

	printf("Excete:%s\n", cmdbuf);
	pid = fork();
	if (pid < 0)
		return 0;
	if (pid == 0) {
		int i;
		signal(SIGUSR1, SIG_DFL);
		signal(SIGUSR2, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGCONT, SIG_DFL);
		signal(SIGSTOP, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGCHLD,SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGSEGV,SIG_DFL);
		signal(SIGPIPE, SIG_DFL);		
		for (i = 3; i < 256; i++)
			close(i);
		exe_default(tmp);
	}
#ifdef OPENWRT	
	if (flag)
		return 0;
#endif	
	while(1) {
		wpid = waitpid(pid, &statbuf, WNOHANG);
		if (wpid != 0)
			break;
		if (waittime < timeout) {
			usleep(100000);
			waittime ++;
		} else {
			kill(pid, SIGKILL);
			usleep(100000);
			wpid = waitpid(pid, &statbuf, WNOHANG);
			return -1;
		}
	}
	printf("wait %d Finish...\n", wpid);
	return 0;
}

inline unsigned char *strstr_end(unsigned char *src,
	       		int len, unsigned char *key, int klen, int end)
{
	unsigned char *tmp = src;

	while (len >= klen) {
		if (!*tmp || *tmp == end)
			break;
		if (!memcmp(tmp, key, klen)) 
			return tmp;
		tmp++;
		len--;
	}
	return NULL;
}

inline unsigned char *strstr_none(unsigned char *str,
					int len, unsigned char *key, int klen)
{
	unsigned char *tmp = str;
	
	while (len >= klen) {
		if (!memcmp(tmp, key, klen))
			return tmp;
		tmp++;
		len--;
	}
	return NULL;
}

inline uint32_t __arr_strcpy_end(unsigned char *dst,
	       	unsigned char *src, int len, int end)
{
	uint32_t i = 0;
	while (src[i] && i < len) {
		if (src[i] == end)
			break;
		dst[i] = src[i];
		i++;
	}
	dst[i] = 0;
	return i;
}

inline void parse_mac(const char *macaddr, unsigned char mac[6])
{
	unsigned int m[6];
	if (sscanf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
		   &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) != 6)
		return;
	mac[0] = m[0];
	mac[1] = m[1];
	mac[2] = m[2];
	mac[3] = m[3];
	mac[4] = m[4];
	mac[5] = m[5];
}

int do_daemon(void)
{
	pid_t pid;
	pid_t sid;

	// already a daemon
	if (getppid() == 1)
		return 0;

	pid = fork();
	if (pid < 0) {
		printf("fork() failed.\r\n");
		return pid;
	}

	if (pid > 0) {
		// exit parent process
		exit(0);
	}
	
	// Create a new SID for the child process
	sid = setsid();
	if (sid < 0) {
		printf("setsid() failed.\r\n");
		return -1;
	}

	pid = fork();
	if (pid < 0) {
		printf("fork() failed (second).\r\n");
		return pid;
	}

	if (pid > 0) {
		// exit parent process
		exit(0);
	}

	// Change the current working directory.
	if ((chdir("/")) < 0) {
		printf("chdir() failed\r\n");
		return -2;
	}
	// Redirect standard files to /dev/null
	if (!freopen("/dev/null", "r", stdin)) {
		printf("Redirection of stdin failed.\r\n");
		return -3;
	}
	if (!freopen("/dev/null", "w", stdout)) {
		printf("Redirection of stdout failed.\r\n");
		return -3;
	}
	if (!freopen("/dev/null", "w", stderr)) {
		printf("Redirection of stdout failed.\r\n");
		return -3;
	}

	return 0;
}

int handler_sig(void)
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = SIG_IGN;
	sigfillset(&act.sa_mask);
	if ((sigaction(SIGCHLD, &act, NULL) == -1) ||
		(sigaction(SIGINT, &act, NULL) == -1) ||
		(sigaction(SIGSEGV, &act, NULL) == -1)) {
		printf("Fail to sigaction\n");
		
	}

	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) == -1) {
		
		printf("Fail to signal(SIGPIPE)\n");
		
	}
	return 0;
}

