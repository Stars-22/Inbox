#include <bits/stdc++.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <conio.h>
//#pragma comment (lib, "ws2_32.dll")
using namespace std;
string version = "1.0.2"; //�汾��
const int Max = 32; //�ͻ������������
void* th_accept(void* arg); //�����߳�-����
void* th_listen(void* arg); //�����߳�-����
void* chat(void* arg); //�����߳�-����
void ip_(); //����������-ȡ������IP
string time_(); //��������-��ȡʱ��
string day_(); //��������-��ȡ����
void write_(string fpm); //��������-�ļ�д��
////////////////////////////////////////////////////////////////
pthread_mutex_t lock; //����������
pthread_t tid1; //�����߳�ID 
pthread_t tid_chat; //�����߳�ID 
//-�����ͻ�����Ϣ�ṹ��
struct SockInfo{ //�ͻ�����Ϣ�ṹ��
    int fd; //�ļ�������
    pthread_t tid; //�߳�ID
    string ip; //�ͻ���IP
    int port; //�ͻ��˶˿�
    string message_listen; //���յ���Ϣ
    string name; //�ͻ����ǳ�
};
struct SockInfo infos[Max]; //�ͻ�����Ϣ��
pthread_t key[Max]; //�ͻ��˶�Ӧ�߳�ID
char* sip; //������IP
char* data_name; //�����¼�ļ��� 
bool change_state = false; //�û�״̬�Ƿ���Ҫ���� 
bool change_message = false; //�����¼�Ƿ���Ҫ���� 
////////////////////////////////////////////////////////////////
int main(){
	system("title �ռ�������");
	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	ip_(); //��ȡ����IP 
	string data_name_;
	data_name_ = day_() + ".txt"; //��ȡ��ǰ���� 
	data_name=(char*)data_name_.data();
	pthread_mutex_init(&lock, NULL); //��ʼ��������
	for(int i=0; i<Max; i++){ //���ݳ�ʼ��(����߳���Ϣ)
		infos[i].fd = -1;
		infos[i].tid = -1;
		memset(&key,0 ,sizeof(key));
		infos[i].message_listen = "|NULL|";
		infos[i].name = "|NULL|";
	}
	pthread_create(&tid1, NULL, th_accept, NULL); //���������߳�
	pthread_detach(tid1); //�����߳�
	pthread_create(&tid_chat, NULL, chat, NULL); //���������߳� 
	pthread_join(tid_chat, NULL); //�ȴ������߳��˳�
	WSACleanup();
	return 0;
}
////////////////////////////////////////////////////////////////
void* th_accept(void* arg){ //-�����߳�
	//-�����������׽���
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){ //�������
		perror("socket");
		closesocket(fd);
		return NULL;
	}
	//-�󶨱���IP port
	struct sockaddr_in addr; //�������Ϣ 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = INADDR_ANY; //0 = 0.0.0.0 
	int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret == -1){ //�������
		perror("bind");
		closesocket(fd);
		return NULL;
	}
	//-���ü���
	ret = listen(fd, 128);
	if(ret == -1){ //�������
		perror("listen");
		closesocket(fd);
		return NULL;
	}
	//-��ʼ����
	int len = sizeof(struct sockaddr);
	struct sockaddr_in caddr; //�ͻ�����Ϣ 
	struct in_addr ipaddr; //�ͻ���IP 
	while(true){
		//-Ѱ�ҿյ����߳�
		int key_;
		pthread_mutex_lock(&lock); //��ȡԿ��
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
		pthread_mutex_unlock(&lock); //�ͷ�Կ��
		//-�������ȴ��ͻ�������
		int cfd = accept(fd, (struct sockaddr*)&caddr, &len);
		if(cfd == -1){ //�������
			perror("accept");
			return NULL;
		}
		pthread_mutex_lock(&lock); //��ȡԿ��
		change_state = true;
		infos[key_].fd = cfd; //�����ļ�������
		ipaddr.s_addr = caddr.sin_addr.s_addr;
		infos[key_].ip = inet_ntoa(ipaddr); //����IP
		infos[key_].port = ntohs(caddr.sin_port); //����˿�
		pthread_create(&key[key_], NULL, th_listen, NULL); //���������߳�
        pthread_detach(key[key_]); //�����߳� 
        pthread_mutex_unlock(&lock); //�ͷ�Կ��
	}
	closesocket(fd); //�ͷ��ļ�������
	return NULL; //�߳̽���
}
////////////////////////////////////////////////////////////////
void* th_listen(void* arg){ //-�����߳�
	bool name_ = false;
	char name1[9];
	memset(&name1, 0, sizeof(name1)); 
	string close;
	//-��ѯ�߳�ID��Ӧ�ͻ�����Ϣ���
	pthread_t th_t = pthread_self(); //��ȡ�߳�ID
	int key_; //���̶߳�Ӧ�ͻ�����Ϣ���
	pthread_mutex_lock(&lock); //��ȡԿ��
	for(int i=0; i<Max; i++){ //Ѱ�Ҷ�Ӧ�ͻ�����Ϣ���
		if(pthread_equal(key[i], th_t) != 0){
			key_ = i;
			break;
		}
	}
	int cfd = infos[key_].fd; //��ȡ�ļ�������
	pthread_mutex_unlock(&lock); //�ͷ�Կ��
	while(true){ //��������
		int len = 0;
		char buf[256];
		memset(&buf, 0, sizeof(buf)); 
		if(name_ == false){
			len = recv(cfd, name1, sizeof(name1), 0); //�����û��ǳ�
			//read(cfd, NULL, 0);
		}
		else {
			len = recv(cfd, buf, sizeof(buf), 0);
		}
		if(len > 0){
			if(name_ == false){
				string fpm;
				pthread_mutex_lock(&lock); //��ȡԿ��
				change_state = true;
				infos[key_].name = name1;
				fpm = time_()+"   ϵͳ��Ϣ : "+infos[key_].name+" ������";
				write_(fpm);
				pthread_mutex_unlock(&lock); //�ͷ�Կ��
				name_ = true;
				continue;
			}
			while(true){
				pthread_mutex_lock(&lock); //��ȡԿ��
				if(infos[key_].message_listen == "|NULL|"){
					change_message = true;
					infos[key_].message_listen = buf;
					pthread_mutex_unlock(&lock); //�ͷ�Կ��
					break;
				}
				pthread_mutex_unlock(&lock); //�ͷ�Կ��
				sleep(2);
			}
		}
		if(len == 0 || len < 0){
			pthread_mutex_unlock(&lock); //�ͷ�Կ��
			break; 
		}
		pthread_mutex_unlock(&lock); //�ͷ�Կ��
	}
	string fpm;
	pthread_mutex_lock(&lock); //��ȡԿ��
	if(infos[key_].name != "|NULL|"){
		fpm = time_() + "   ϵͳ��Ϣ : " + infos[key_].name + " ������";
		write_(fpm);
	}
	change_state = true;
	infos[key_].fd = -1; //�ͷ���Ϣ�ռ�
	key[key_] = -1; //�ͷ���Ϣ�ռ�
	infos[key_].name = "|NULL|"; //�ͷ���Ϣ�ռ� 
	pthread_mutex_unlock(&lock); //�ͷ�Կ��
	closesocket(cfd); //�ͷ��ļ�������
	return NULL; //�߳̽���
}
////////////////////////////////////////////////////////////////
void* chat(void* arg){ //-�����߳� 
	bool have = false; 
	int user_order[Max];
	cout<<"ͨ�ŷ����-V"<<version<<"  by����Stars"<<endl;
	cout<<"  ������IP: "<<sip<<endl<<endl;
	cout<<"�ȴ�����...";
	while(true){
		pthread_mutex_lock(&lock); //��ȡԿ��
		if(change_state){ //�����û�״̬
			have = true;
			change_state = false;
			int num = 0;
			system("cls");
			cout<<"�ռ�������-V"<<version<<"  by����Stars"<<endl;
			cout<<"  ������벻�����ģ���С��һ���ٴ�"<<endl;
			cout<<"    ������IP: "<<sip<<endl<<endl;
			cout<<"�����û�:"<<endl;
			for(int i=0; i<Max; i++){ //����û��б�
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
			pthread_mutex_unlock(&lock); //�ͷ�Կ��
			//-�����������¼ 
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
		if(change_message){ //�յ���Ϣ 
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
			pthread_mutex_unlock(&lock); //�ͷ�Կ�� 
		}
		if(have) cout<<"\r�������뷢����Ϣ���û����(�س�ȷ��):";
		sleep(1); 
		if(kbhit() && have){
			int give;
			char send_message[256];
			memset(&send_message, 0, sizeof(send_message)); 
			cin>>give;
			if(infos[user_order[give-1]].fd == -1){
				cout<<"��ǰ�û�������"<<endl;
				system("pause");
				change_state = true;
				pthread_mutex_unlock(&lock); //�ͷ�Կ��
				continue;
			} 
			cout<<"�����뷢�͸�"<<infos[user_order[give-1]].name<<"����Ϣ(��256�ַ���ȡ������������ESC���س�ȷ��):"; 
			scanf("%s", send_message);
			if(send_message[0] == 'E' && send_message[1] == 'S' && send_message[2] == 'C'){
				pthread_mutex_unlock(&lock); //�ͷ�Կ��
				continue;
			}
			send(infos[user_order[give-1]].fd, send_message, strlen(send_message) + sizeof(char), 0);	// ����������Ϣ
			string temporary = "";
			for(int t=0; t<8-infos[user_order[give-1]].name.size(); t++) temporary = temporary + ' ';
			string fpm;
			fpm = time_() + "  ->" + temporary + infos[user_order[give-1]].name + ": " + send_message;
			write_(fpm);
			change_state = true;
			pthread_mutex_unlock(&lock); //�ͷ�Կ��
		}
		pthread_mutex_unlock(&lock); //�ͷ�Կ��
	}
	return NULL; //�߳̽���
}
////////////////////////////////////////////////////////////////
void ip_(){ //-��ȡ����IP���� 
	char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* host;
    host = gethostbyname(hostname);
    sip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
}
////////////////////////////////////////////////////////////////
string time_(){ //-��ȡʱ�亯�� 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d %H:%M:%S",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
string day_(){ //-��ȡ���ں��� 
	time_t t = time(0); 
	char temporary[32];
	memset(&temporary,0,sizeof(temporary));
	strftime(temporary, sizeof(temporary), "%Y-%m-%d",localtime(&t));
	string time_temporary = temporary;
	return time_temporary;
}
////////////////////////////////////////////////////////////////
void write_(string fpm){ //-�ļ�д�뺯�� 
	FILE *fp;
	char fp_m[300];
	strcpy(fp_m,fpm.c_str());
	fp = fopen(data_name, "a");
	fprintf(fp, "%s\n", fp_m);
	fclose(fp);
}
