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
#define MAXIMUM 4096
#define Max 1024

int sockfd;					// client's socket
char* IP = "127.0.0.1";		// server's IP
short PORT = 8080;			// server's port
typedef struct sockaddr SA;
char file[Max];
char name[30];
int getflag = 0;
int download = 0;
int fdwr;
int nrd = 0;
int reset = 0;

void menu(){
	printf("------------------------------\n");
	printf("--                          --\n");
	printf("-- 1. 群播 : 直接輸入訊息   --\n");
	printf("-- 2. 單播 : ＠接收者 訊息  --\n");
	printf("-- 3. 在線名單 : 輸入 list  --\n");
	printf("-- 4. 傳送檔案 : 輸入 send  --\n");
	printf("-- 5. 退出 : 輸入 bye       --\n");
	printf("-- 6. 幫助 : 輸入 help      --\n");
	printf("--                          --\n");
	printf("------------------------------\n");
}

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("Fail to connect to server");
        exit(-1);
    }
    printf("\n------成功啟動客戶端------\n\n");
}

void start(){
    pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    
	char buf2[Max] = {};
    strcpy(buf2,name);
	strcat(buf2,":in");
    send(sockfd,buf2,strlen(buf2),0);
    
	//getchar();
	while(1){
        char buf[Max] = {};
		char msg[Max] = {};
		char person[Max] = {};
		char filename[Max];
		fgets(buf,sizeof(buf),stdin);

		if(buf[0]=='\n')	continue;

		if (strcmp("bye\n",buf)==0 || reset)
            break;

		if(strcmp("help\n",buf)==0){
			menu();
		}else if(buf[0]=='@'){
			sprintf(msg,"%s",buf);
			strcat(msg,":one");
			send(sockfd,msg,strlen(msg),0);
		}else if(strcmp("list\n",buf)==0){
			strcat(buf,":list");
			send(sockfd,buf,strlen(buf),0);
		}else if(strcmp("send\n",buf)==0){
			printf(" <輸入> 欲傳送的檔案名稱 : ");			
			scanf("%s",filename);
			
			char buff[MAXIMUM]={};
			char tmp[Max]={};
			int fdrd;
			int nrd = 0;

			if (-1 == (access(filename, F_OK))){
				printf(" <系統> 根目錄不存在 <%s> , 傳送失敗\n", filename);
				continue;
			}

			if ( -1 == (fdrd=(open(filename, O_RDONLY))) )
				perror("Fail to open");
			
			strcpy(tmp,filename);
			strcat(filename,":send");
			send(sockfd,filename,strlen(filename),0);
			sleep(1);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0){
				//strcat(buff,":write");
				//printf("nrd = %d\n",nrd);
				send(sockfd, buff, nrd, 0);
				usleep(1);
			}

			sleep(1);			
			printf(" <系統> 傳送 %s 成功\n", tmp);
			strcat(tmp,":end");
			send(sockfd,tmp,strlen(tmp),0);
			
			printf(" <輸入> 傳送檔案的對象 : ");
			scanf("%s",person);
			strcat(person,":check");
			send(sockfd,person,strlen(person),0);
			sleep(1);
			close(fdrd);
		}else if(download && strcmp("yes\n",buf)==0){
			char buff[MAXIMUM] = {};
			//printf(" 下載的檔案名稱 : ");
			//scanf("%s",filename);
			memset(filename,0,sizeof(filename));
			strncpy(filename,file,strlen(file));
			printf(" <系統> 開始下載 <%s>\n",filename);

			if (-1 == (fdwr=open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");

			strcat(filename,":get");
			send(sockfd, filename, strlen(filename), 0);
			getflag = 1;
			download = 0;
		}else if(download && strcmp("no\n",buf)==0){
			printf(" <系統> 已取消下載 <%s>\n",file);
			download = 0;
			memset(file,0,sizeof(file));
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
        char buf[MAXIMUM] = {};
		nrd = recv(sockfd,buf,sizeof(buf),0);
		//printf("nrd = %d , getflag = %d , fdwr = %d\n",nrd,getflag,fdwr);
        
		if (nrd <= 0)
        	continue;    
			//return NULL;

		if(strcmp(buf,"terminate")==0){
			printf(" <系統> 已有同名使用者在線上 , 請隨便輸入以退出程式\n");
			reset = 1;
			close(sockfd);
			continue;
		}

		if(strcmp(buf,"fail")==0){
			printf(" <系統> 下載檔案 <%s> 失敗\n",file);
			close(fdwr);
			getflag = 0;
		}

		if(strcmp(buf,"sendover")==0){
			printf(" <系統> 下載檔案 <%s> 成功\n",file);
			close(fdwr);
			getflag = 0;
		}

		if(buf[0]=='f' && buf[1]=='i' && buf[2]=='l' && buf[3]=='e'){
			char tmp[Max];			
			sscanf(buf,"%s %s",tmp,file);
			continue;
		}

		if(strcmp(buf,"download")==0){
			download = 1;
			continue;
		}

		if(!getflag && strcmp(buf,"sendover")!=0)
        	printf("%s\n",buf);
		else if(getflag){
			int nwr = 0;
			if ((nwr = write(fdwr, buf, nrd)) < 0 )
				perror("write fail");
			//printf("buff = %s , nrd = %d\n",buf,nrd);		
		}
/*
		if(strcmp(buf,"permit")==0){
			printf("沒有要下載的檔案\n");
			close(fdwr);
			getflag = 0;
		}

		if(strcmp(buf,"notexist")==0){
			printf("檔案不存在\n");
			close(fdwr);
			getflag = 0;
		}
*/
    }
}

int main(){
	char buf[Max];

    init();
    printf(" <輸入> 請輸入您的名字 : ");
    scanf("%s",name);

	strcpy(buf,name);
	strcat(buf,":name");
	send(sockfd,buf,strlen(buf),0);
	sleep(1);

	menu();
	start();
    return 0;
}

