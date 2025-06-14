#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 500

unsigned WINAPI User_Login_Interface(void* arg);
unsigned WINAPI User_Create_Interface(void* arg);
unsigned WINAPI ClinicSearch_Interface(void* arg);
unsigned WINAPI ClinicAdviceSend_Interface(void* arg);
unsigned WINAPI ClinicAdviceRecv_Interface(void* arg);
unsigned WINAPI Program_Exit(void* arg);
void Pharmacy_Serach_Interface(SOCKET sock);
int User_login_Create_Interface(void);
int menu_Interface(void);
void ErrorHandling(char* msg);

int recvmsging = 1;
int login_true = 0;//로그인 확인용
char restart[10] = {0};
char adviceexit[10] = {0};
char id[20];

int main(int argc, char const *argv[])
{
	system("chcp 65001 >nul");
	//소켓 활성화 구조체
    WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	HANDLE LoginInterfaceThread, CreateInterfaceThread, ClinicSearchInterface, ClinicAdviceSendThread, ClinicAdviceRecvThread, ProgramExitThread;


	//서버IP, 서버포트번호 고정
	char severIp[100] = "127.0.0.1";
	char port[100] = "55555";


	//윈도우 소켓 사용을 알림
    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
		ErrorHandling("WSAStartup() error!");


	//소켓 생성 함수
    sock=socket(PF_INET,SOCK_STREAM,0);


    //소켓 생성 코드 4개
    memset(&serverAddr,0,sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=inet_addr(severIp);
	serverAddr.sin_port=htons(atoi(port));


	//소켓연결 함수 
    if(connect(sock,(SOCKADDR*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR)
		ErrorHandling("connect() error");


    int choose_num = 0;

	while (login_true != 1)
	{
		choose_num = User_login_Create_Interface();
		
        if (choose_num == 1)
        {
			//printf("login\n");
			send(sock, "LO", 2, 0);
			LoginInterfaceThread = (HANDLE)_beginthreadex(NULL,0,User_Login_Interface,(void*)&sock,0,NULL);
			WaitForSingleObject(LoginInterfaceThread,INFINITE);
		}
        else if (choose_num == 2)
        {
			//printf("create\n");
            send(sock, "CR", 2, 0);
			CreateInterfaceThread = (HANDLE)_beginthreadex(NULL,0,User_Create_Interface,(void*)&sock,0,NULL);
			WaitForSingleObject(CreateInterfaceThread,INFINITE);
        }
		else
		{
			continue;
		}
		
	}
	
	
	//printf("탈출 성공\n");
	
	
	while (1)
	{
		choose_num = 0;
		choose_num = menu_Interface();

		if (choose_num == 1)
		{
			//printf("Clinic Search\n");
            send(sock, "CN", 2, 0);
			ClinicSearchInterface = (HANDLE)_beginthreadex(NULL,0,ClinicSearch_Interface,(void*)&sock,0,NULL);
			WaitForSingleObject(ClinicSearchInterface,INFINITE);
		}
		else if (choose_num == 2)
		{
			//printf("Clinic Advice\n");
			send(sock, "AD", 2, 0);
			ClinicAdviceSendThread = (HANDLE)_beginthreadex(NULL,0,ClinicAdviceSend_Interface,(void*)&sock,0,NULL);
			ClinicAdviceRecvThread = (HANDLE)_beginthreadex(NULL,0,ClinicAdviceRecv_Interface,(void*)&sock,0,NULL);
			
			WaitForSingleObject(ClinicAdviceSendThread,INFINITE);
			WaitForSingleObject(ClinicAdviceRecvThread,INFINITE);
		}
		else if (choose_num == 3)
		{
			send(sock, "EX", 2, 0);
			ProgramExitThread=(HANDLE)_beginthreadex(NULL,0,Program_Exit,(void*)&sock,0,NULL);
			WaitForSingleObject(ProgramExitThread,INFINITE);
			break;
		}
		else
		{
			continue;
		}
		
	}
	

	closesocket(sock);
	WSACleanup();
    return 0;
}


int User_login_Create_Interface() {
    char msg[BUF_SIZE] = {0};
    int choose_num;

    printf("=====================================================================\n");
	printf("====================  LOGIN & CREATE INTERFACE  ====================\n");
	printf("=====================================================================\n");
    printf("\n");
    printf("1. Login\n\n");
    printf("2. create account\n");
	printf("\n");
    printf("=====================================================================\n");
    printf("\n\n");
    printf("choose number > ");
    
    fgets(msg, sizeof(msg), stdin);
	msg[strlen(msg)-1] = '\0';
    choose_num = atoi(msg);

    return choose_num;
}


unsigned WINAPI User_Login_Interface(void* arg) {
    SOCKET sock=*((SOCKET*)arg);
	
	char password[20];
	char sendmsg[BUF_SIZE] = {0};
	char recvmsg[BUF_SIZE] = {0};
	printf("=====================================================================\n");
	printf("========================= LOGIN INTERFACE =========================\n");
	printf("=====================================================================\n");
	printf("ID > ");
	fgets(id, sizeof(id), stdin);
	id[strlen(id) - 1] = '\0';

	printf("PASSWORD > ");
	fgets(password, sizeof(password), stdin);
	password[strlen(password) - 1] = '\0';

	printf("\n");
	printf("=====================================================================\n");
	
	strcat(sendmsg, id);
	strcat(sendmsg, "/");
	strcat(sendmsg, password);

	send(sock, sendmsg, BUF_SIZE, 0);

	//printf("%s\n",sendmsg);

	recv(sock, recvmsg, BUF_SIZE, 0);
	
	//printf("%s\n",recvmsg);
	
	login_true = atoi(recvmsg);
}


unsigned WINAPI User_Create_Interface(void* arg) {
	SOCKET sock=*((SOCKET*)arg);
	char id[20];
	char password[20];
	char sendmsg[BUF_SIZE] = {0};
	char recvmsg[BUF_SIZE] = {0};

	printf("=====================================================================\n");
	printf("========================= CREATE INTERFACE =========================\n");
	printf("======== 병원 EX) ID > 병원명  ||  환자 EX) ID > 환자명 =============\n");
	printf("=====================================================================\n");
	printf("ID > ");
	fgets(id, sizeof(id), stdin);
	id[strlen(id) - 1] = '\0';

	printf("PASSWORD > ");
	fgets(password, sizeof(password), stdin);
	password[strlen(password) - 1] = '\0';

	printf("\n");
	printf("=====================================================================\n");
	
	strcat(sendmsg, id);
	strcat(sendmsg, "/");
	strcat(sendmsg, password);

	send(sock, sendmsg, BUF_SIZE, 0);
	//printf("%s\n",sendmsg);

	recv(sock, recvmsg, BUF_SIZE, 0);
	//printf("%s\n",recvmsg);

	login_true = atoi(recvmsg);
}


int menu_Interface() {
	char msg[BUF_SIZE] = {0};
    int choose_num;

	printf("=====================================================================\n");
	printf("==============================  Menu  ===============================\n");
	printf("=====================================================================\n");
    printf("\n");
    printf("1. Clinic Search\n\n");
	printf("2. Advice Clinic\n\n");
	printf("3. Exit Program\n");
	printf("\n");
    printf("=====================================================================\n");
    printf("\n\n");
    printf("choose number > ");

	fgets(msg, sizeof(msg), stdin);
	msg[strlen(msg)-1] = '\0';
    choose_num = atoi(msg);

    return choose_num;
}


unsigned WINAPI ClinicSearch_Interface(void* arg) {
	SOCKET sock=*((SOCKET*)arg);
	char num[5];
	char major[50];
	char address[50];
	char clinic_count[BUF_SIZE] = {0};
	char msg[BUF_SIZE] = {0};
	char sendmsg[BUF_SIZE] = {0};
	char recvmsg[BUF_SIZE] = {0};

start:

	memset(recvmsg, 0, BUF_SIZE);

	printf("====================================================================\n");
	printf("========================== Clinic Search ==========================\n");
	printf("====================================================================\n");
	printf("\n");
    printf("1. 전문 과목 검색\n\n");
    printf("2. 주소로 검색\n\n");
	printf("3. 고급 검색\n");
	printf("\n");
    printf("====================================================================\n");
	printf("\n\nchoose number > ");
	fgets(num, sizeof(num), stdin);
	num[strlen(num) - 1] = '\0';

	int choose = atoi(num);

	if (choose > 3 || choose < 1)
	{
		goto start;
	}
	
	if (strcmp(num, "1") == 0)
	{
		printf("==========================================================================\n");
		printf("============================= Clinic Search ==============================\n");
		printf("==========================================================================\n");
		printf("| 가정의학과   내과    정형외과    비뇨의학과   산부인과   소아청소년과 |\n");
		printf("| 신경외과  심장혈관흉부외과    안과    영상의학과    외과   이비인후과 |\n");
		printf("| 재활의학과   정신건강의학과   마취통증의학과   치과   피부과   한의원 |\n");
		printf("==========================================================================\n");
		printf("\n\nsearch major > ");
		fgets(major, sizeof(major), stdin);
		major[strlen(major) - 1] = '\0';
		
		strcat(sendmsg, num);
		strcat(sendmsg,"/");
		strcat(sendmsg, major);

		send(sock, sendmsg, BUF_SIZE, 0);
	}
	if (strcmp(num, "2") == 0)
	{
		printf("====================================================================\n");
		printf("=========================== Clinic Search ==========================\n");
		printf("====================================================================\n");
		printf("== Search ex1) 충청남도 아산시 배방읍 배방로 ...  \n");
		printf("== Search ex2) 도로명                           \n");
		printf("====================================================================\n");
		printf("\n\nsearch address > ");
		fgets(address, sizeof(address), stdin);
		address[strlen(address) - 1] = '\0';

		strcat(sendmsg, num);
		strcat(sendmsg,"/");
		strcat(sendmsg, address);

		send(sock, sendmsg, BUF_SIZE, 0);
	}
	if (strcmp(num, "3") == 0)
	{
		printf("==========================================================================\n");
		printf("============================== Clinic Search =============================\n");
		printf("==========================================================================\n");
		printf("==========================================================================\n");
		printf("| 가정의학과   내과    정형외과    비뇨의학과   산부인과   소아청소년과 |\n");
		printf("| 신경외과  심장혈관흉부외과    안과    영상의학과    외과   이비인후과 |\n");
		printf("| 재활의학과   정신건강의학과   마취통증의학과   치과   피부과   한의원 |\n");
		printf("==========================================================================\n");
		printf("== Search ex1)   major > 전문 과목 \n");
		printf("==========================================================================\n");
		printf("== Search ex2) address > 도로명 or 충청남도 아산시 배방읍 배방로 ...   \n");
		printf("==========================================================================\n");
		printf("\n\nsearch major > ");
		fgets(major, sizeof(major), stdin);
		major[strlen(major) - 1] = '\0';

		printf("\n\nsearch address > ");
		fgets(address, sizeof(address), stdin);
		address[strlen(address) - 1] = '\0';

		strcat(sendmsg, num);
		strcat(sendmsg,"/");
		strcat(sendmsg, major);
		strcat(sendmsg,"/");
		strcat(sendmsg, address);

		send(sock, sendmsg, BUF_SIZE, 0);
	}

	recv(sock, clinic_count, BUF_SIZE, 0);
	int count = atoi(clinic_count);
	printf("%d\n", count);

	if (count == 0)
	{
		printf("검색 결과가 없습니다.");
	}
	else
	{
		printf("========================= Clinic Search Result ==========================\n");
		for ( int i = 0; i < count; i++)
		{
			memset(recvmsg, 0, 500);
			int recvlen = recv(sock, recvmsg, 500 -1, 0);
			recvmsg[recvlen] = '\0';

			printf("%d | %s", i+1, recvmsg);
			printf("\n");
			
			if (recvmsg[strlen(recvmsg) - 1] != '\n') {
				printf("\n");
			}
		}
	}


	printf("\n의원 인근 약국 정보가 필요하십니까? ( Y / N ) > ");
	fgets(msg, sizeof(msg), stdin);
	msg[strlen(msg) - 1] = '\0';

	if (strcmp(msg, "Y") == 0)
	{
		send(sock, "PH", 2, 0);
		// 약국 검색 함수
		Pharmacy_Serach_Interface(sock);
	}
	else;
	{
		send(sock, "NO", 2, 0);
		return 0;
	}

	return 0;
}


void Pharmacy_Serach_Interface(SOCKET sock) {
	char name[50];
	char clinic_count[BUF_SIZE] = {0};
	char sendmsg[BUF_SIZE] = {0};
	char recvmsg[BUF_SIZE] = {0};

	printf("====================================================================\n");
	printf("========================= Pharmacy Search ==========================\n");
	printf("====================================================================\n");
	printf("병원명을 입력하십시오.\n");
	printf("====================================================================\n");
	printf("\n\nsearch name > ");

	fgets(name, sizeof(name), stdin);
	name[strlen(name) - 1] = '\0';

	strcpy(sendmsg, name);
	send(sock, sendmsg, BUF_SIZE, 0);

	recv(sock, clinic_count, BUF_SIZE, 0);
	int count = atoi(clinic_count);
	printf("%d\n", count);

	if (count == 0)
	{
		printf("검색 결과가 없습니다.\n");
	}
	
	if (count != 0)
	{
		printf("========================= Clinic Search Result ==========================\n");
		for ( int i = 0; i < count; i++)
		{
			memset(recvmsg, 0, 500);
			int recvlen = recv(sock, recvmsg, 500 -1, 0);
			recvmsg[recvlen] = '\0';

			printf("%d | %s", i+1, recvmsg);
			printf("\n");
			
			if (recvmsg[strlen(recvmsg) - 1] != '\n') {
				printf("\n");
			}
		}
	}

}


unsigned WINAPI ClinicAdviceSend_Interface(void* arg) {
	SOCKET sock=*((SOCKET*)arg);
	char name[BUF_SIZE] = {0};
	recvmsging = 1;

	printf("====================================================================\n");
	printf("========================== Clinic Advice ==========================\n");
	printf("====================================================================\n");
	printf("대화하고 싶은 사람을 입력 후 문자를 입력하시오.\n");
	printf("종료를 원하시면 q를 입력하십시오.\n");
	printf("====================================================================\n");
	// printf("\n\n상담하고 싶은 병원명을 입력하십시오 > ");
	printf("\n > ");
	fgets(name, sizeof(name), stdin);
	name[strlen(name) - 1] = '\0';

	while(1)
	{
		char sendmsg[BUF_SIZE] = {0};
		char msg[BUF_SIZE] = {0};

		printf("\n > ");
		fgets(msg, sizeof(msg), stdin);
		msg[strlen(msg) - 1] = '\0';

		if (strcmp(msg, "q") == 0 && recvmsging == 1)
		{
			//printf("종료\n");
			strcat(sendmsg, name);
			strcat(sendmsg, "/");
			strcat(sendmsg, "q");
			send(sock, sendmsg, strlen(sendmsg), 0);
			
			return 0;
		}
		if (strcmp(msg, "q") == 0 && recvmsging == 0)
		{
			//printf("종료\n");
			
			return 0;
		}
		if (strcmp(restart, "r") == 0)
		{
			return 0;
		}

		strcat(sendmsg, name);
		//strcat(sendmsg, "/");
		//strcat(sendmsg, id);
		strcat(sendmsg, "/");
		strcat(sendmsg, msg);

		send(sock, sendmsg, strlen(sendmsg), 0);
	}

	return 0;
}


unsigned WINAPI ClinicAdviceRecv_Interface(void* arg) {
	SOCKET sock=*((SOCKET*)arg);
	char recvmsg[BUF_SIZE] = {0};

	while (1)
	{

		int recvlen = recv(sock, recvmsg, BUF_SIZE, 0);
		if (recvlen <= 0) {
			printf("상대방이 연결을 종료했습니다.\n");
			printf("\n > ");
			return 0;
		}
		recvmsg[recvlen] = '\0';

		if (strcmp(recvmsg, "q") == 0)
		{
			printf("상대방이 채팅을 종료하였습니다.\n");
			printf("\n > ");
			
			recvmsging = 0;
			
			return 0;
		}

		if (strcmp(recvmsg, "r") == 0)
		{
			printf("상대방을 찾을 수 없습니다.\n");
			printf("\n > ");
			return 0;
		}


		printf("%s\n", recvmsg);
		printf("\n > ");
	}

	return 0;
}


unsigned WINAPI Program_Exit(void* arg) {
	SOCKET sock=*((SOCKET*)arg);
	char recvmsg[BUF_SIZE] = {0};
	char sendmsg[BUF_SIZE] = {0};

	recv(sock, recvmsg, BUF_SIZE, 0);
	//printf("%s\n", recvmsg);

	if(!strcmp(recvmsg, "0")){
		printf("> 프로그램을 종료합니다.\n");
		closesocket(sock);
		exit(0);
	}

	return 0;
}

//오류 메시지 출력 함수
void ErrorHandling(char* msg){
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}