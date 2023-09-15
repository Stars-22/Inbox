#include <bits/stdc++.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <conio.h>
//#pragma comment (lib, "ws2_32.dll")
using namespace std;
string version = "1.0.1"; //版本号 
void* th_listen(void* arg); //声明线程-接收
string time_(); //声明函数-获取时间
bool change_message = false; //是否接收到消息
char message_listen[256];
int fd;
bool quit = false;
int main(){
	system("title 收件箱客户端");
	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	char buf[256];
	memset(&buf, 0, sizeof(buf)); 
	//创建通信的套接字
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket");
		return -1;
	}
	//连接服务器IP port
	char IP[20];
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(9999);
	cout<<"收件箱客户端-V"<<version<<"  by繁星Stars"<<endl<<endl;
	printf("请输入服务器IP:");
	scanf("%s", &IP);
	printf("等待连接ing...\n");
	saddr.sin_addr.s_addr = inet_addr(IP);
	//saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //0 = 0.0.0.0
	while(true){
		int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
		if(ret != -1){
			break;
		}
	}
	cout<<"连接成功"<<endl<<endl;
	//创建线程-接收
	pthread_t tid; //监听线程ID 
	pthread_create(&tid, NULL, th_listen, NULL); //创建监听线程
	pthread_detach(tid); //分离线程 
	//设置名字 
	char name[8];
	int name_size;
	while(true){
		cout<<"请为自己设置昵称(限8字符或8字符以内，汉字占2字符，回车确定): ";
		cin>>buf;
		if(buf[8] == 0){
			sprintf(name, buf);
			send(fd, name, strlen(name)+1, 0);
			break;
		} 
		cout<<"当前输入不符合昵称限制"<<endl; 
	}
	string blank = "";
	for(int i=7;i>0;i--){
		blank = blank + ' ';
		if(name[i] != 0)break;
	}
	//界面初始化
	system("cls"); 
	cout<<"收件箱客户端-V"<<version<<"  by繁星Stars"<<endl;
	cout<<"  如果输入不了中文，最小化一下再打开"<<endl;
	cout<<"  按's'键开始发送消息，回车确定"<<endl;
	cout<<"    服务器IP: "<<IP<<endl;
	cout<<"    当前昵称: "<<name<<endl<<endl; 
	//通信
	while(true){
		memset(&buf, 0, sizeof(buf)); 
		//发送数据
		if(kbhit()){
			char k = getch();
			if(k != 's')continue;
			cout<<"请输入想要发送的信息(限256符，输入ESC取消): ";
			cin>>buf;
			if(buf[0] == 'E' && buf[1] == 'S' && buf[2] == 'C'){
				cout<<"已取消"<<endl; 
				continue;
			}
			if(buf[0] == 'E' && buf[1] == 'X' && buf[2] == 'I' && buf[3] == 'T'){
				quit = true;
				break;
			}
			send(fd, buf, strlen(buf)+1, 0);
			cout<<time_()<<"  ->"<<blank<<name<<": "<<buf<<endl;
			continue;
		}
		if(quit)break;
		if(change_message){
			cout<<time_()<<"  <-"<<blank<<name<<": "<<message_listen<<endl;
			change_message = false;
		}
	}
	//关闭文件描述符 
	closesocket(fd);
	system("pause");
	return 0;
}

void* th_listen(void* arg){ //-接收线程
	int len = 0;
	while(true){
		while(true){
			if(change_message){
				sleep(1);
				continue;
			}
			memset(&message_listen, 0, sizeof(message_listen));
			len = recv(fd, message_listen, sizeof(message_listen), 0);
			change_message = true;
			break;
		}
		if(quit) return NULL; //线程结束
		if(len == 0){
			quit = true;
			printf("\n服务器已经断开了连接...\n");
			return NULL; //线程结束
		}
		if(len == -1 && !quit){
			quit = true;
			perror("recv");
			return NULL; //线程结束
		}
	}
	return NULL; //线程结束
}

string time_(){ //-获取时间函数 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d %H:%M:%S",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
