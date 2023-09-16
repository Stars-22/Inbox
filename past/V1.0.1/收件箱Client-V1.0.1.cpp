#include <bits/stdc++.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <conio.h>
//#pragma comment (lib, "ws2_32.dll")
using namespace std;
string version = "1.0.1"; //�汾�� 
void* th_listen(void* arg); //�����߳�-����
string time_(); //��������-��ȡʱ��
bool change_message = false; //�Ƿ���յ���Ϣ
char message_listen[256];
int fd;
bool quit = false;
int main(){
	system("title �ռ���ͻ���");
	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	char buf[256];
	memset(&buf, 0, sizeof(buf)); 
	//����ͨ�ŵ��׽���
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		perror("socket");
		return -1;
	}
	//���ӷ�����IP port
	char IP[20];
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(9999);
	cout<<"�ռ���ͻ���-V"<<version<<"  by����Stars"<<endl<<endl;
	printf("�����������IP:");
	scanf("%s", &IP);
	printf("�ȴ�����ing...\n");
	saddr.sin_addr.s_addr = inet_addr(IP);
	//saddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //0 = 0.0.0.0
	while(true){
		int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
		if(ret != -1){
			break;
		}
	}
	cout<<"���ӳɹ�"<<endl<<endl;
	//�����߳�-����
	pthread_t tid; //�����߳�ID 
	pthread_create(&tid, NULL, th_listen, NULL); //���������߳�
	pthread_detach(tid); //�����߳� 
	//�������� 
	char name[8];
	int name_size;
	while(true){
		cout<<"��Ϊ�Լ������ǳ�(��8�ַ���8�ַ����ڣ�����ռ2�ַ����س�ȷ��): ";
		cin>>buf;
		if(buf[8] == 0){
			sprintf(name, buf);
			send(fd, name, strlen(name)+1, 0);
			break;
		} 
		cout<<"��ǰ���벻�����ǳ�����"<<endl; 
	}
	string blank = "";
	for(int i=7;i>0;i--){
		blank = blank + ' ';
		if(name[i] != 0)break;
	}
	//�����ʼ��
	system("cls"); 
	cout<<"�ռ���ͻ���-V"<<version<<"  by����Stars"<<endl;
	cout<<"  ������벻�����ģ���С��һ���ٴ�"<<endl;
	cout<<"  ��'s'����ʼ������Ϣ���س�ȷ��"<<endl;
	cout<<"    ������IP: "<<IP<<endl;
	cout<<"    ��ǰ�ǳ�: "<<name<<endl<<endl; 
	//ͨ��
	while(true){
		memset(&buf, 0, sizeof(buf)); 
		//��������
		if(kbhit()){
			char k = getch();
			if(k != 's')continue;
			cout<<"��������Ҫ���͵���Ϣ(��256��������ESCȡ��): ";
			cin>>buf;
			if(buf[0] == 'E' && buf[1] == 'S' && buf[2] == 'C'){
				cout<<"��ȡ��"<<endl; 
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
	//�ر��ļ������� 
	closesocket(fd);
	system("pause");
	return 0;
}

void* th_listen(void* arg){ //-�����߳�
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
		if(quit) return NULL; //�߳̽���
		if(len == 0){
			quit = true;
			printf("\n�������Ѿ��Ͽ�������...\n");
			return NULL; //�߳̽���
		}
		if(len == -1 && !quit){
			quit = true;
			perror("recv");
			return NULL; //�߳̽���
		}
	}
	return NULL; //�߳̽���
}

string time_(){ //-��ȡʱ�亯�� 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d %H:%M:%S",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
