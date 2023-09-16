#include <bits/stdc++.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <conio.h>
//#pragma comment (lib, "ws2_32.dll")
using namespace std;
string version = "1.0.2"; //版本号
const int Max = 32; //客户端最大连接数
void* th_accept(void* arg); //声明线程-监听
void* th_listen(void* arg); //声明线程-接收
void* chat(void* arg); //声明线程-聊天
void ip_(); //获声明函数-取服务器IP
string time_(); //声明函数-获取时间
string day_(); //声明函数-获取日期
void write_(string fpm); //声明函数-文件写入
////////////////////////////////////////////////////////////////
pthread_mutex_t lock; //声明互斥锁
pthread_t tid1; //监听线程ID 
pthread_t tid_chat; //聊天线程ID 
//-创建客户端信息结构体
struct SockInfo{ //客户端信息结构体
    int fd; //文件描述符
    pthread_t tid; //线程ID
    string ip; //客户端IP
    int port; //客户端端口
    string message_listen; //接收的信息
    string name; //客户端昵称
};
struct SockInfo infos[Max]; //客户端信息数
pthread_t key[Max]; //客户端对应线程ID
char* sip; //服务器IP
char* data_name; //聊天记录文件名 
bool change_state = false; //用户状态是否需要更新 
bool change_message = false; //聊天记录是否需要更新 
////////////////////////////////////////////////////////////////
int main(){
	system("title 收件箱服务端");
	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	ip_(); //获取本机IP 
	string data_name_;
	data_name_ = day_() + ".txt"; //获取当前日期 
	data_name=(char*)data_name_.data();
	pthread_mutex_init(&lock, NULL); //初始化锁变量
	for(int i=0; i<Max; i++){ //数据初始化(清空线程信息)
		infos[i].fd = -1;
		infos[i].tid = -1;
		memset(&key,0 ,sizeof(key));
		infos[i].message_listen = "|NULL|";
		infos[i].name = "|NULL|";
	}
	pthread_create(&tid1, NULL, th_accept, NULL); //创建监听线程
	pthread_detach(tid1); //分离线程
	pthread_create(&tid_chat, NULL, chat, NULL); //创建聊天线程 
	pthread_join(tid_chat, NULL); //等待聊天线程退出
	WSACleanup();
	return 0;
}
////////////////////////////////////////////////////////////////
void* th_accept(void* arg){ //-监听线程
	//-创建监听的套接字
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){ //输出错误
		perror("socket");
		closesocket(fd);
		return NULL;
	}
	//-绑定本地IP port
	struct sockaddr_in addr; //服务端信息 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = INADDR_ANY; //0 = 0.0.0.0 
	int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret == -1){ //输出错误
		perror("bind");
		closesocket(fd);
		return NULL;
	}
	//-设置监听
	ret = listen(fd, 128);
	if(ret == -1){ //输出错误
		perror("listen");
		closesocket(fd);
		return NULL;
	}
	//-开始监听
	int len = sizeof(struct sockaddr);
	struct sockaddr_in caddr; //客户端信息 
	struct in_addr ipaddr; //客户端IP 
	while(true){
		//-寻找空的子线程
		int key_;
		pthread_mutex_lock(&lock); //获取钥匙
		for(int i=0; i<Max; i++){
			if(infos[i].fd == -1){
				key_ = i;
				break;
			}
			if(i == Max-1){
				sleep(2);
				i = 0;
			}
		}
		pthread_mutex_unlock(&lock); //释放钥匙
		//-阻塞并等待客户端连接
		int cfd = accept(fd, (struct sockaddr*)&caddr, &len);
		if(cfd == -1){ //输出错误
			perror("accept");
			return NULL;
		}
		pthread_mutex_lock(&lock); //获取钥匙
		change_state = true;
		infos[key_].fd = cfd; //传入文件描述符
		ipaddr.s_addr = caddr.sin_addr.s_addr;
		infos[key_].ip = inet_ntoa(ipaddr); //传入IP
		infos[key_].port = ntohs(caddr.sin_port); //传入端口
		pthread_create(&key[key_], NULL, th_listen, NULL); //创建接收线程
        pthread_detach(key[key_]); //分离线程 
        pthread_mutex_unlock(&lock); //释放钥匙
	}
	closesocket(fd); //释放文件描述符
	return NULL; //线程结束
}
////////////////////////////////////////////////////////////////
void* th_listen(void* arg){ //-接收线程
	bool name_ = false;
	char name1[9];
	memset(&name1, 0, sizeof(name1)); 
	string close;
	//-查询线程ID对应客户端信息序号
	pthread_t th_t = pthread_self(); //获取线程ID
	int key_; //本线程对应客户端信息序号
	pthread_mutex_lock(&lock); //获取钥匙
	for(int i=0; i<Max; i++){ //寻找对应客户端信息序号
		if(pthread_equal(key[i], th_t) != 0){
			key_ = i;
			break;
		}
	}
	int cfd = infos[key_].fd; //获取文件描述符
	pthread_mutex_unlock(&lock); //释放钥匙
	while(true){ //接收数据
		int len = 0;
		char buf[256];
		memset(&buf, 0, sizeof(buf)); 
		if(name_ == false){
			len = recv(cfd, name1, sizeof(name1), 0); //传入用户昵称
			//read(cfd, NULL, 0);
		}
		else {
			len = recv(cfd, buf, sizeof(buf), 0);
		}
		if(len > 0){
			if(name_ == false){
				string fpm;
				pthread_mutex_lock(&lock); //获取钥匙
				change_state = true;
				infos[key_].name = name1;
				fpm = time_()+"   系统信息 : "+infos[key_].name+" 上线了";
				write_(fpm);
				pthread_mutex_unlock(&lock); //释放钥匙
				name_ = true;
				continue;
			}
			while(true){
				pthread_mutex_lock(&lock); //获取钥匙
				if(infos[key_].message_listen == "|NULL|"){
					change_message = true;
					infos[key_].message_listen = buf;
					pthread_mutex_unlock(&lock); //释放钥匙
					break;
				}
				pthread_mutex_unlock(&lock); //释放钥匙
				sleep(2);
			}
		}
		if(len == 0 || len < 0){
			pthread_mutex_unlock(&lock); //释放钥匙
			break; 
		}
		pthread_mutex_unlock(&lock); //释放钥匙
	}
	string fpm;
	pthread_mutex_lock(&lock); //获取钥匙
	if(infos[key_].name != "|NULL|"){
		fpm = time_() + "   系统信息 : " + infos[key_].name + " 离线了";
		write_(fpm);
	}
	change_state = true;
	infos[key_].fd = -1; //释放信息空间
	key[key_] = -1; //释放信息空间
	infos[key_].name = "|NULL|"; //释放信息空间 
	pthread_mutex_unlock(&lock); //释放钥匙
	closesocket(cfd); //释放文件描述符
	return NULL; //线程结束
}
////////////////////////////////////////////////////////////////
void* chat(void* arg){ //-聊天线程 
	bool have = false; 
	int user_order[Max];
	cout<<"通信服务端-V"<<version<<"  by繁星Stars"<<endl;
	cout<<"  服务器IP: "<<sip<<endl<<endl;
	cout<<"等待连接...";
	while(true){
		pthread_mutex_lock(&lock); //获取钥匙
		if(change_state){ //更新用户状态
			have = true;
			change_state = false;
			int num = 0;
			system("cls");
			cout<<"收件箱服务端-V"<<version<<"  by繁星Stars"<<endl;
			cout<<"  如果输入不了中文，最小化一下再打开"<<endl;
			cout<<"    服务器IP: "<<sip<<endl<<endl;
			cout<<"在线用户:"<<endl;
			for(int i=0; i<Max; i++){ //输出用户列表
				if(infos[i].fd != -1){
					user_order[num] = i;
					num++;
					cout<<num<<".";
					cout<<" "<<infos[i].name;
					for(int t=0; t<11-infos[i].name.size(); t++)cout<<" ";
					cout<<infos[i].ip<<":"<<infos[i].port<<endl;
				}
			}
			cout<<endl; 
			pthread_mutex_unlock(&lock); //释放钥匙
			//-重载入聊天记录 
			freopen(data_name,"r",stdin);
			string message;
			char k;
			while(true){
				k = getchar();
				if (k != EOF){
					getline(cin, message);
					message = k + message;
					cout<<message<<endl;
				}
				else break;
			}
			freopen("CON","r",stdin);
			cin.clear();
		}
		if(change_message){ //收到消息 
			change_message = false;
			for(int i=0; i<Max; i++){
				if(infos[i].message_listen != "|NULL|"){
					string temporary = "";
					for(int t=0; t<8-infos[i].name.size(); t++) temporary = temporary + ' ';
					string fpm;
					fpm = time_() + "  <-" + temporary + infos[i].name + ": " + infos[i].message_listen;
					write_(fpm);
					cout<<"\b\r"<<fpm<<endl;
					infos[i].message_listen = "|NULL|";
				}
			}
			pthread_mutex_unlock(&lock); //释放钥匙 
		}
		if(have) cout<<"\r请输入想发给消息的用户序号(回车确定):";
		sleep(1); 
		if(kbhit() && have){
			int give;
			char send_message[256];
			memset(&send_message, 0, sizeof(send_message)); 
			cin>>give;
			if(infos[user_order[give-1]].fd == -1){
				cout<<"当前用户不存在"<<endl;
				system("pause");
				change_state = true;
				pthread_mutex_unlock(&lock); //释放钥匙
				continue;
			} 
			cout<<"请输入发送给"<<infos[user_order[give-1]].name<<"的消息(限256字符，取消发送请输入ESC，回车确定):"; 
			scanf("%s", send_message);
			if(send_message[0] == 'E' && send_message[1] == 'S' && send_message[2] == 'C'){
				pthread_mutex_unlock(&lock); //释放钥匙
				continue;
			}
			send(infos[user_order[give-1]].fd, send_message, strlen(send_message) + sizeof(char), 0);	// 发送聊天消息
			string temporary = "";
			for(int t=0; t<8-infos[user_order[give-1]].name.size(); t++) temporary = temporary + ' ';
			string fpm;
			fpm = time_() + "  ->" + temporary + infos[user_order[give-1]].name + ": " + send_message;
			write_(fpm);
			change_state = true;
			pthread_mutex_unlock(&lock); //释放钥匙
		}
		pthread_mutex_unlock(&lock); //释放钥匙
	}
	return NULL; //线程结束
}
////////////////////////////////////////////////////////////////
void ip_(){ //-获取本机IP函数 
	char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host;
    host = gethostbyname(hostname);
    sip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
}
////////////////////////////////////////////////////////////////
string time_(){ //-获取时间函数 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d %H:%M:%S",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
string day_(){ //-获取日期函数 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
////////////////////////////////////////////////////////////////
void write_(string fpm){ //-文件写入函数 
	FILE *fp;
	char fp_m[300];
	strcpy(fp_m,fpm.c_str());
	fp = fopen(data_name, "a");
	fprintf(fp, "%s\n", fp_m);
	fclose(fp);
}
