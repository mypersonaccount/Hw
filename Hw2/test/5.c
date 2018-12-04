#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>


int sockfd;//客户端socket
char* IP = "127.0.0.1";//服务器的IP
short PORT = 4444;//服务器服务端口
typedef struct sockaddr SA;
char name[30];
int getflag = 0;
int fdwr;
int nrd = 0;

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("无法连接到服务器");
        exit(-1);
    }
    printf("客户端启动成功\n");
}

void start(){
    pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    
	char buf2[100] = {};
    sprintf(buf2,"%s进入了聊天室",name);
	strcat(buf2,":all");
    send(sockfd,buf2,strlen(buf2),0);
    
	getchar();
	while(1){
        char buf[100] = {};
		char msg[131] = {};
		char person[100] = {};
		char filename[1024];
		fgets(buf,sizeof(buf),stdin);

		if (buf[0]=='b' && buf[1]=='y' && buf[2]=='e'){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"%s退出了聊天室",name);
			strcat(buf2,":all");
            send(sockfd,buf2,strlen(buf2),0);
            break;
        }

		if(buf[0]=='\n')	continue;

		if(buf[0]=='@'){
			sprintf(msg,"%s",buf);
			strcat(msg,":one");
			send(sockfd,msg,strlen(msg),0);
		}else if(buf[0]=='l' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t'){
			strcat(buf,":list");
			send(sockfd,buf,strlen(buf),0);
		}else if(buf[0]=='s' && buf[1]=='e' && buf[2]=='n' && buf[3]=='d'){
			printf("filename : ");			
			scanf("%s",filename);
			
			char buff[4096]={};
			char tmp[1024]={};
			int fdrd;
			int nrd = 0;

			if ( -1 == (fdrd=(open(filename, O_RDONLY))) )
				perror("open fail");
			
			strcpy(tmp,filename);
			strcat(filename,":send");
			send(sockfd,filename,strlen(filename),0);
			sleep(1);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0){
				//strcat(buff,":write");
				send(sockfd, buff, nrd, 0);
				usleep(1);
			}

			sleep(1);			
			printf("put %s success\n", tmp);
			strcat(tmp,":end");
			send(sockfd,tmp,strlen(tmp),0);
			
			printf("傳送檔案的對象 : ");
			scanf("%s",person);
			strcat(person,":check");
			send(sockfd,person,strlen(person),0);
			sleep(1);
			close(fdrd);
		}else if(buf[0]=='w' && buf[1]=='a' && buf[2]=='n' && buf[3]=='t'){

			char buff[4096] = {};
			printf("下載的檔案名稱 : ");
			scanf("%s",filename);
			if (-1 == (fdwr=open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");

			strcat(filename,":get");
			send(sockfd, filename, strlen(filename), 0);
			getflag = 1;
		}else{
			sprintf(msg,"%s 說 %s",name,buf);
			strcat(msg,":all");
			send(sockfd,msg,strlen(msg),0);
		}
		
    }
    close(sockfd);
}

void* recv_thread(void* p){
    while(1){
        char buf[4096] = {};
		nrd = recv(sockfd,buf,sizeof(buf),0);
		//printf("nrd = %d , getflag = %d , fdwr = %d\n",nrd,getflag,fdwr);
        
		if (nrd <= 0)
        	continue;    
			//return NULL;

		if(strcmp(buf,"sendover")==0){
			printf("get success\n");
			close(fdwr);
			getflag = 0;
		}

		if(!getflag)
        	printf("%s\n",buf);
		else{
			int nwr = 0;
			if ((nwr = write(fdwr, buf, nrd)) < 0 )
				perror("write fail");
			printf("buff = %s , nrd = %d\n",buf,nrd);		
		}
    }
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>


int sockfd;//客户端socket
char* IP = "127.0.0.1";//服务器的IP
short PORT = 4444;//服务器服务端口
typedef struct sockaddr SA;
char name[30];
int getflag = 0;
int fdwr;
int nrd = 0;

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("无法连接到服务器");
        exit(-1);
    }
    printf("客户端启动成功\n");
}

void start(){
    pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    
	char buf2[100] = {};

    sprintf(buf2,"%s进入了聊天室",name);
	strcat(buf2,":all");
    send(sockfd,buf2,strlen(buf2),0);
    
	getchar();
	while(1){
        char buf[100] = {};
		char msg[131] = {};
		char person[100] = {};
		char filename[1024];
		fgets(buf,sizeof(buf),stdin);

		if (buf[0]=='b' && buf[1]=='y' && buf[2]=='e'){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"%s退出了聊天室",name);
			strcat(buf2,":all");
            send(sockfd,buf2,strlen(buf2),0);
            break;
        }

		if(buf[0]=='\n')	continue;

		if(buf[0]=='@'){
			sprintf(msg,"%s",buf);
			strcat(msg,":one");
			send(sockfd,msg,strlen(msg),0);
		}else if(buf[0]=='l' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t'){
			strcat(buf,":list");
			send(sockfd,buf,strlen(buf),0);
		}else if(buf[0]=='s' && buf[1]=='e' && buf[2]=='n' && buf[3]=='d'){
			printf("filename : ");			
			scanf("%s",filename);
			
			char buff[4096]={};
			char tmp[1024]={};
			int fdrd;
			int nrd = 0;

			if ( -1 == (fdrd=(open(filename, O_RDONLY))) )
				perror("open fail");
			
			strcpy(tmp,filename);
			strcat(filename,":send");
			send(sockfd,filename,strlen(filename),0);
			sleep(1);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0){
				//strcat(buff,":write");
				send(sockfd, buff, nrd, 0);
				usleep(1);
			}

			sleep(1);			
			printf("put %s success\n", tmp);
			strcat(tmp,":end");
			send(sockfd,tmp,strlen(tmp),0);
			
			printf("傳送檔案的對象 : ");
			scanf("%s",person);
			strcat(person,":check");
			send(sockfd,person,strlen(person),0);
			sleep(1);
			close(fdrd);
		}else if(buf[0]=='w' && buf[1]=='a' && buf[2]=='n' && buf[3]=='t'){

			char buff[4096] = {};
			printf("下載的檔案名稱 : ");
			scanf("%s",filename);
			if (-1 == (fdwr=open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");

			strcat(filename,":get");
			send(sockfd, filename, strlen(filename), 0);
			getflag = 1;
		}else{
			sprintf(msg,"%s 說 %s",name,buf);
			strcat(msg,":all");
			send(sockfd,msg,strlen(msg),0);
		}
		
    }
    close(sockfd);
}

void* recv_thread(void* p){
    while(1){
        char buf[4096] = {};
		nrd = recv(sockfd,buf,sizeof(buf),0);
		//printf("nrd = %d , getflag = %d , fdwr = %d\n",nrd,getflag,fdwr);
        
		if (nrd <= 0)
        	continue;    
			//return NULL;

		if(strcmp(buf,"sendover")==0){
			printf("get success\n");
			close(fdwr);
			getflag = 0;
		}

		if(!getflag)
        	printf("%s\n",buf);
		else{
			int nwr = 0;
			if ((nwr = write(fdwr, buf, nrd)) < 0 )
				perror("write fail");
			printf("buff = %s , nrd = %d\n",buf,nrd);		
		}
    }
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>


int sockfd;//客户端socket
char* IP = "127.0.0.1";//服务器的IP
short PORT = 4444;//服务器服务端口
typedef struct sockaddr SA;
char name[30];
int getflag = 0;
int fdwr;
int nrd = 0;

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("无法连接到服务器");
        exit(-1);
    }
    printf("客户端启动成功\n");
}

void start(){
    pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    
	char buf2[100] = {};

    sprintf(buf2,"%s进入了聊天室",name);
	strcat(buf2,":all");
    send(sockfd,buf2,strlen(buf2),0);
    
	getchar();
	while(1){
        char buf[100] = {};
		char msg[131] = {};
		char person[100] = {};
		char filename[1024];
		fgets(buf,sizeof(buf),stdin);

		if (buf[0]=='b' && buf[1]=='y' && buf[2]=='e'){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"%s退出了聊天室",name);
			strcat(buf2,":all");
            send(sockfd,buf2,strlen(buf2),0);
            break;
        }

		if(buf[0]=='\n')	continue;

		if(buf[0]=='@'){
			sprintf(msg,"%s",buf);
			strcat(msg,":one");
			send(sockfd,msg,strlen(msg),0);
		}else if(buf[0]=='l' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t'){
			strcat(buf,":list");
			send(sockfd,buf,strlen(buf),0);
		}else if(buf[0]=='s' && buf[1]=='e' && buf[2]=='n' && buf[3]=='d'){
			printf("filename : ");			
			scanf("%s",filename);
			
			char buff[4096]={};
			char tmp[1024]={};
			int fdrd;
			int nrd = 0;

			if ( -1 == (fdrd=(open(filename, O_RDONLY))) )
				perror("open fail");
			
			strcpy(tmp,filename);
			strcat(filename,":send");
			send(sockfd,filename,strlen(filename),0);
			sleep(1);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0){
				//strcat(buff,":write");
				send(sockfd, buff, nrd, 0);
				usleep(1);
			}

			sleep(1);			
			printf("put %s success\n", tmp);
			strcat(tmp,":end");
			send(sockfd,tmp,strlen(tmp),0);
			
			printf("傳送檔案的對象 : ");
			scanf("%s",person);
			strcat(person,":check");
			send(sockfd,person,strlen(person),0);
			sleep(1);
			close(fdrd);
		}else if(buf[0]=='w' && buf[1]=='a' && buf[2]=='n' && buf[3]=='t'){

			char buff[4096] = {};
			printf("下載的檔案名稱 : ");
			scanf("%s",filename);
			if (-1 == (fdwr=open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");

			strcat(filename,":get");
			send(sockfd, filename, strlen(filename), 0);
			getflag = 1;
		}else{
			sprintf(msg,"%s 說 %s",name,buf);
			strcat(msg,":all");
			send(sockfd,msg,strlen(msg),0);
		}
		
    }
    close(sockfd);
}

void* recv_thread(void* p){
    while(1){
        char buf[4096] = {};
		nrd = recv(sockfd,buf,sizeof(buf),0);
		//printf("nrd = %d , getflag = %d , fdwr = %d\n",nrd,getflag,fdwr);
        
		if (nrd <= 0)
        	continue;    
			//return NULL;

		if(strcmp(buf,"sendover")==0){
			printf("get success\n");
			close(fdwr);
			getflag = 0;
		}

		if(!getflag)
        	printf("%s\n",buf);
		else{
			int nwr = 0;
			if ((nwr = write(fdwr, buf, nrd)) < 0 )
				perror("write fail");
			printf("buff = %s , nrd = %d\n",buf,nrd);		
		}
    }
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>


int sockfd;//客户端socket
char* IP = "127.0.0.1";//服务器的IP
short PORT = 4444;//服务器服务端口
typedef struct sockaddr SA;
char name[30];
int getflag = 0;
int fdwr;
int nrd = 0;

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("无法连接到服务器");
        exit(-1);
    }
    printf("客户端启动成功\n");
}

void start(){
    pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    
	char buf2[100] = {};

    sprintf(buf2,"%s进入了聊天室",name);
	strcat(buf2,":all");
    send(sockfd,buf2,strlen(buf2),0);
    
	getchar();
	while(1){
        char buf[100] = {};
		char msg[131] = {};
		char person[100] = {};
		char filename[1024];
		fgets(buf,sizeof(buf),stdin);

		if (buf[0]=='b' && buf[1]=='y' && buf[2]=='e'){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"%s退出了聊天室",name);
			strcat(buf2,":all");
            send(sockfd,buf2,strlen(buf2),0);
            break;
        }

		if(buf[0]=='\n')	continue;

		if(buf[0]=='@'){
			sprintf(msg,"%s",buf);
			strcat(msg,":one");
			send(sockfd,msg,strlen(msg),0);
		}else if(buf[0]=='l' && buf[1]=='i' && buf[2]=='s' && buf[3]=='t'){
			strcat(buf,":list");
			send(sockfd,buf,strlen(buf),0);
		}else if(buf[0]=='s' && buf[1]=='e' && buf[2]=='n' && buf[3]=='d'){
			printf("filename : ");			
			scanf("%s",filename);
			
			char buff[4096]={};
			char tmp[1024]={};
			int fdrd;
			int nrd = 0;

			if ( -1 == (fdrd=(open(filename, O_RDONLY))) )
				perror("open fail");
			
			strcpy(tmp,filename);
			strcat(filename,":send");
			send(sockfd,filename,strlen(filename),0);
			sleep(1);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0){
				//strcat(buff,":write");
				send(sockfd, buff, nrd, 0);
				usleep(1);
			}

			sleep(1);			
			printf("put %s success\n", tmp);
			strcat(tmp,":end");
			send(sockfd,tmp,strlen(tmp),0);
			
			printf("傳送檔案的對象 : ");
			scanf("%s",person);
			strcat(person,":check");
			send(sockfd,person,strlen(person),0);
			sleep(1);
			close(fdrd);
		}else if(buf[0]=='w' && buf[1]=='a' && buf[2]=='n' && buf[3]=='t'){

			char buff[4096] = {};
			printf("下載的檔案名稱 : ");
			scanf("%s",filename);
			if (-1 == (fdwr=open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");

			strcat(filename,":get");
			send(sockfd, filename, strlen(filename), 0);
			getflag = 1;
		}else{
			sprintf(msg,"%s 說 %s",name,buf);
			strcat(msg,":all");
			send(sockfd,msg,strlen(msg),0);
		}
		
    }
    close(sockfd);
}

void* recv_thread(void* p){
    while(1){
        char buf[4096] = {};
		nrd = recv(sockfd,buf,sizeof(buf),0);
		//printf("nrd = %d , getflag = %d , fdwr = %d\n",nrd,getflag,fdwr);
        
		if (nrd <= 0)
        	continue;    
			//return NULL;

		if(strcmp(buf,"sendover")==0){
			printf("get success\n");
			close(fdwr);
			getflag = 0;
		}

		if(!getflag)
        	printf("%s\n",buf);
		else{
			int nwr = 0;
			if ((nwr = write(fdwr, buf, nrd)) < 0 )
				perror("write fail");
			printf("buff = %s , nrd = %d\n",buf,nrd);		
		}
    }
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}

int main(){
	char buf[1024];

    init();
    printf("请输入您的名字：");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);

	sleep(1);
	start();
    return 0;
}
