#include "../include/daemon.h"

void createDaemon(const char *programName)
{
	int i;
	int fdIn, fdOut, fdErr;
	pid_t pid;
	struct rlimit resourceLimit;
	
	//set file creation mask to 0
	umask(0);
	
	//create new child process
	if ((pid = fork()) < 0) {
		perror("can’t fork");
		exit(0);
	}
	//child get new process id after parent is closed
	else if (pid != 0)
		exit(0);
	
	//becomes the leader of new session, of new process group and it is disassociated from the terminal 
	if(setsid()<0)
	{
		perror("can’t create process group");
		exit(0);
	}
	
	//fork and exit parent again, now we are assured that we are disassociated from the terminal
	if ((pid = fork()) < 0) {
		perror("can’t fork");
	}
	else if (pid != 0) /* parent */
		exit(0);
	
	//we have to set currect directory to root directory, otherwise if the parent is on mounted file system we can not unmount this file system...
	if (chdir("/") < 0) {
		perror("can not change current directory to home directory");
	}
	
	
	//highest number of file descriptors, save to rlimit structure
	if (getrlimit(RLIMIT_NOFILE, &resourceLimit) < 0) {
		perror("can’t get file descriptors limit");
	}
	//if RLIM_INFINITY == -1 this means value is larger than can be represented in a 32 bit, ulong
	if (resourceLimit.rlim_max == RLIM_INFINITY)
		resourceLimit.rlim_max = 100;
	// close all file descriptors
	for (i = 0; i < resourceLimit.rlim_max; i++)
		close(i);
		
	// set file descriptor to /dev/null
	fdIn = open("/dev/null", O_RDWR); //standard input
	fdOut = dup(0); //copy of old fd //standard output
	fdErr = dup(0); //standard error
	
	//open log file
	openlog(programName, LOG_CONS, LOG_DAEMON);
	if (fdIn != 0 || fdOut != 1 || fdErr != 2) {
		syslog(LOG_ERR, "unexpected problems with file descriptors %d %d %d",fdIn, fdOut, fdErr);
		exit(1);
	}
}

int lockFile(int fd)
{
	struct flock fl;
	//advisory locking
	fl.l_type = F_WRLCK; // write lock
	fl.l_start = 0;
	fl.l_whence = SEEK_SET; //for beginning of file is going to start locking (fl.l_start)
	fl.l_len = 0;
	int res = fcntl(fd, F_SETLK, &fl);
	return res; // if return -1 file is locked
}

int daemonIsRunning()
{
	int		fd;
	char	*daemonID;
	char* daemonLockFile;
	
	asprintf(&daemonLockFile, "%s/daemon.pid", sc.executionDirectory);

	//open with rw permissions, create, 00700
	fd = open(daemonLockFile, O_RDWR|O_CREAT, S_IRWXU);
	if (fd < 0) {
		loggerServer(LOG_ERR, "can't open", daemonLockFile, NULL);
		exit(1);
	}
	if(lockFile(fd) < 0) {
		if(errno == EACCES || errno == EAGAIN) {
			loggerServer(LOG_ERR, "file is locked by another process", "", NULL);
			close(fd);
			return 1;
		}
		loggerServer(LOG_ERR, "can't lock", daemonLockFile, NULL);
		exit(1);
	}
	free(daemonLockFile);
	//we clear the file
	ftruncate(fd, 0);
	//cast pid_t to long
	asprintf(&daemonID, "%ld", (long)getpid());
	write(fd, daemonID, strlen(daemonID));
	free(daemonID);
	return 0;
}
