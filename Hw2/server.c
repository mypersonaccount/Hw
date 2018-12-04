#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#define MAXIMUM 4096
#define Max 1024
#define SIZE 100

int sockfd;						// server's socket
int fds[SIZE];					// client's socketfd
int size = SIZE ;				// 聊天室最多容納人數
char* IP = "127.0.0.1";
short PORT = 8080;

char *NAME[SIZE];
int used[SIZE];
char *FILELIST[SIZE];
int fused[SIZE];
char transfile[Max];
char onemsg[Max];
int fdwr,wflag=0;

typedef struct sockaddr SA;

void init(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if (sockfd == -1){
        perror("Fail to create socket");
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (bind(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("Fail to bind socket");
    	exit(-1);
    }
    if (listen(sockfd,SIZE) == -1){
        perror("Fail to listen socket");
        exit(-1);
    }
}

void SendMsgToAll(char* msg){
    int i;
    for (i = 0;i < size;i++){
        if (fds[i] != 0){
            printf("send to %d\n",fds[i]);
            send(fds[i],msg,strlen(msg),0);
        }
    }
}

void SendMsgToOne(char* msg,int num){
    printf("send to %d\n",num);
    send(num,msg,strlen(msg),0);
}

void file(){
	FILE *p;
	int index;
	char tmp[Max];
	memset(fused,0,sizeof(fused));
	
	p = fopen("file.txt","r");			
	while(fscanf(p,"%s",tmp)!=EOF){
//printf("tmp = %s\n",tmp);
		fscanf(p,"%d",&index);
//printf("index = %d\n",index);	
		strncpy(FILELIST[index],tmp,strlen(tmp));
		fused[index]=1;
	}
	fclose(p);
}

void data(){
	FILE *ptr;
	int index;
	char tmp[Max];
	memset(used,0,sizeof(used));
	
	ptr = fopen("name.txt","r");			
	while(fscanf(ptr,"%s",tmp)!=EOF){
		fscanf(ptr,"%d",&index);				
		strcpy(NAME[index],tmp);
		used[index]=1;
	}
	fclose(ptr);
}

void quit(int fd){
	FILE *ptr;
	int i,count;
	char number[Max];
	char msg[Max]={};
	memset(msg,0,sizeof(msg));
	
	data();
	sprintf(msg," <系統> %s 離開聊天室",NAME[fd]);
	SendMsgToAll(msg);	

	ptr = fopen("name.txt","w");
	char list[Max];
	memset(list,0,sizeof(list));
	
	used[fd] = 0;
	for(i=0;i<SIZE;i++){
		if(used[i]){
			for(count=0;NAME[i][count]!='\0';count++);
			fwrite(NAME[i],1,count,ptr);
			sprintf(number,"\n%d\n",i);
			for(count=0;number[count]!='\0';count++);
			fwrite(number,1,count,ptr);
		}
	}			
	fclose(ptr);
}

void* service_thread(void* p){
    int fd = *(int*)p;
    printf("pthread = %d\n",fd);
    while(1){
        char buf[MAXIMUM]={};
		char person[Max]={};
		int nrd = 0;
		nrd = recv(fd,buf,sizeof(buf),0);
		printf("nrd = %d\n",nrd);
        if (nrd <= 0){
            int i;
            for (i = 0;i < size;i++){
                if (fd == fds[i]){
                    fds[i] = 0;
                    break;
                }
            }
			quit(fd);
            printf("quit : fd = %d\n",fd);
            pthread_exit((void*)i);
        }
		// server接受到的訊息傳送給所有client
		
		char copied[MAXIMUM]={};
		strncpy(copied,buf,strlen(buf));
		char action[MAXIMUM]={};
		char str[MAXIMUM]={};
		char *delim = ":";
		char *pch;
		int flag = 1;
		strncpy(str,copied,strlen(copied));
		//printf ("Splitting string \"%s\" into tokens:\n",str);
		pch = strtok(str,delim);
		while (pch != NULL){
			//printf ("%s\n",pch);
			if(flag){
				flag = 0;
				strcpy(copied,pch);			
			}else
				strcpy(action,pch);
			pch = strtok (NULL, delim);
		}

		strcpy(person,copied);
		//printf("wflag = %d , action = %s , copied = %s\n",wflag,action,copied);
		if(strcmp("in",action)==0){	
			char buf2[Max];
			sprintf(buf2," <系統> %s進入聊天室",copied);	
			SendMsgToAll(buf2);
		}else if(strcmp("all",action)==0){
			char buf2[Max];
			sprintf(buf2," (群播) >>> %s",copied);
			SendMsgToAll(buf2);
		}else if(strcmp("one",action)==0){
			char person[Max];			
			char msg[Max];
			sscanf(copied,"%s%s",person,msg);
			strcpy(onemsg,msg);

			int i,no=-1;
			for(i=0;i<sizeof(person);i++)
				person[i]=person[i+1];
			person[i]='\0';
			//printf("person = %s\n",person);
			
			data();

			for(i=0;i<SIZE;i++){
				if(used[i])
					if(strcmp(NAME[i],person)==0)	no = i;
			}
			if(no==-1){
				SendMsgToOne(" <系統> 指定的人不在聊天室",fd);
				continue;
			}

			char buf2[Max];
			strcpy(buf2," (單播) >>>>> ");
			strcat(buf2,NAME[fd]);
			strcat(buf2," 說 ");
			strcat(buf2,onemsg);
			//printf("copied = %s\n",copied);
			printf("buf2 = %s , no = %d\n",buf2,no);
			SendMsgToOne(buf2,no);
		}else if(strcmp("quit",action)==0){
			quit(fd);
		}else if(strcmp("name",action)==0){
			FILE *ptr;	
			int i,count,reset = 0;
			char number[Max];
			
			data();
			for(i=0;i<SIZE;i++){
				if(used[i])
					if(strcmp(NAME[i],copied)==0)	reset = 1;
			}
			if(reset){
				sleep(1);
				SendMsgToOne("terminate",fd);
			}//else
				//SendMsgToOne("OK",fd);
		
			ptr = fopen("name.txt","w");
			
			if(!reset){
				NAME[fd] = copied;	used[fd] = 1;
			}
			for(i=0;i<SIZE;i++){
				if(used[i]){
					for(count=0;NAME[i][count]!='\0';count++);
					fwrite(NAME[i],1,count,ptr);
					sprintf(number,"\n%d\n",i);
					for(count=0;number[count]!='\0';count++);
					fwrite(number,1,count,ptr);
				}
			}			
			fclose(ptr);			

			printf("fd = %d , name = %s\n",fd,NAME[fd]);
		}else if(strcmp("list",action)==0){
			char buff[Max];
			data();
			SendMsgToOne("\n 在線名單 :\n\n",fd);
			for(int i=0;i<SIZE;i++){
				if(used[i]){
					sprintf(buff," --- 名稱 : %s ( fd = %d ) ---\n",NAME[i],i);
					//printf("%s\n",buff);
					SendMsgToOne(buff,fd);
				}
			}
			SendMsgToOne("             < end >            ",fd);
		}else if(strcmp("send",action)==0){
			wflag = 1;
			if (-1 == (fdwr=open(copied, O_WRONLY|O_CREAT|O_TRUNC, 0666)))
				perror("open file fail");
			printf("fdwr = %d , copied = %s\n",fdwr,copied);

			FILE *ptr;
			ptr = fopen("file.txt","a");
			fwrite(copied,1,strlen(copied),ptr);
			fclose(ptr);
			strcpy(transfile,copied);
		}
		else if(strcmp("end",action)==0){
			wflag = 0;
			printf("get %s success\n", copied);
			close(fdwr);
		}
		else if(strcmp("write",action)==0 || wflag==1){
			int nwr = 0;
			//for(nrd=0;copied[nrd]!='\0';nrd++);
			printf("buf = %s\n",buf);
			printf("nrd = %d\n",nrd);
			if ( (nwr = write(fdwr, buf, nrd)) < 0 )
					perror("write fail");
		}
		else if(strcmp("check",action)==0){
			char msg[Max] = {};
			char msg2[Max] = {};
			int no,i;
			data();
			printf("person = %s , action = check\n",person);

			sprintf(msg," %s 傳送一份檔案給您 , 請問是否要下載 <%s> ? (yes / no)",NAME[fd],transfile);
			for(i=0,no=-1;i<SIZE;i++){
				if(used[i])
					if(strcmp(NAME[i],person)==0)	no = i;
			}
			if(no==-1){
				SendMsgToOne(" <系統> 指定的人不在聊天室",fd);
				continue;
			}

			char number[SIZE] = {};
			sprintf(number,"\n%d\n",no);
			FILE *ptr;
			ptr = fopen("file.txt","a");
			fwrite(number,1,strlen(number),ptr);
			fclose(ptr);

			sprintf(msg2,"file %s",transfile);

			SendMsgToOne(msg2,no);
			sleep(1);
			sprintf(msg2,"download");
			SendMsgToOne(msg2,no);
			sleep(1);
			SendMsgToOne(msg,no);
			SendMsgToOne(" <系統> 已送出接收資訊",fd);
		}
		else if(strcmp("get",action)==0){
			//file();
			int fdrd;
			int nrd = 0,num = -1,i;
			char buff[MAXIMUM];
/*
			if(!fused[fd]){
				strcpy(buff,"permit");			
				send(fd,buff,strlen(buff),0);
				continue;
			}

			if(strcmp(FILELIST[fd],buf)!=0){
				strcpy(buff,"notexist");		
				send(fd,buff,strlen(buff),0);
				continue;
			}
*/
			printf("copied = %s , num = %d\n",copied,num);

			if (-1 == (access(copied, F_OK))){
				printf("<%s> is not exist\n", copied);
				SendMsgToOne("fail",fd);
				continue;
			}

			if ( -1 == (fdrd=(open(copied, O_RDONLY))) )
				perror("open fail");
			printf("fdrd = %d\n",fdrd);
			while ( (nrd = read(fdrd, buff, sizeof(buff))) > 0 ){
				//printf("buff = %s\n",buff);
				send(fd,buff,nrd,0);
				usleep(1);
			}
			printf("send %s over\n", copied);
			close(fdrd);
			sleep(1);
			send(fd,"sendover",sizeof("sendover"),0);
		}
	}
}

void service(){
    printf("\n------啟動伺服器------\n\n");
    while(1){
        struct sockaddr_in fromaddr;
        socklen_t len = sizeof(fromaddr);
        int fd = accept(sockfd,(SA*)&fromaddr,&len);
        if (fd == -1){
            printf("Fail to accept client...\n");
            continue;
        }
        int i = 0;
        for (i = 0;i < size;i++){
            if (fds[i] == 0){
                // record client's sock
                fds[i] = fd;
                printf("fd = %d\n",fd);
                // client connected , use thread
                pthread_t tid;
                pthread_create(&tid,0,service_thread,&fd);
                break;
            }
		    if (size == i){
		        // chatroom fulled
		        char* str = " <系統> 聊天室已滿";
		        send(fd,str,strlen(str),0); 
		        close(fd);
		    }
        }
    }
}

int main(){
	FILE *ptr;
	ptr = fopen("name.txt","w+");
	fclose(ptr);
	ptr = fopen("file.txt","w+");
	fclose(ptr);

    init();
    service();
}

