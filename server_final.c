#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>


#define BUF_SIZE 500
#define MAX_CLNT 256


// 로그인 아이디 구조체
typedef struct User{
    char id[20];
    char password[20];
    struct User *next;
} User;


// 의원 정보 구조체
typedef struct Clinic {
    char major[50];
    char name[500];
    char address[500];
    char number[50];
    struct Clinic *next;
} Clinic;


// 약국 정보 구조체
typedef struct Pharmacy {
    char major[50];
    char name[509];
    char address[500];
    char number[50];
    struct Pharmacy *next;
}Pharmacy;


typedef struct Socket_Id{
    SOCKET clientSock;
    char id[20];
}Socket_Id;

// 함수 및 쓰레드
void ErrorHandling(char* msg);
unsigned WINAPI MAIN (void* arg);
unsigned WINAPI User_Login(void* arg); // 로그인 쓰레드
unsigned WINAPI User_Add(void* arg); // 회원가입 쓰레드
unsigned WINAPI Program_Exit(void* arg); // 프로그램 종료 쓰레드
unsigned WINAPI Clinic_Search(void* arg); // 의원 검색
unsigned WINAPI Clinic_Advice (void* arg); // 의원 1대1 채팅 기능
void Pharmacy_Serach(SOCKET clientSock); // 약국 검색
User *User_list();
User *UserAdd (User *list, char *id, char *password);
Clinic *Clinic_list();
Pharmacy *Pharmacy_list();


int clientCount=0;
char clientid[BUF_SIZE] = {0};
Socket_Id clientSocks[MAX_CLNT];
int login_true = 0;//로그인 확인 변수
HANDLE hMutex;
WSADATA wsaData;

SOCKET serverSock,clientSock;
SOCKADDR_IN serverAddr,clientAddr;
HANDLE UserLoginThread, UserAddThread, ClinicSearchThread, PharmacySerachThread, ClinicAdviceThread, ProgramExitThread, hThread;
int clientAddrSize;

int main(int argc, char const *argv[])
{
    system("chcp 65001 >nul");

    //서버 포트 번호 고정
    char port[100] = "55555";


    //윈도우 소켓 사용을 알림
    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
		ErrorHandling("WSAStartup() error!");
    
    //하나의 뮤텍스를 생성
    hMutex=CreateMutex(NULL,FALSE,NULL);

    //소켓 생성 함수
    serverSock=socket(PF_INET,SOCK_STREAM,0);


    //소켓 생성 코드 4개
    memset(&serverAddr,0,sizeof(serverAddr)); 
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serverAddr.sin_port=htons(atoi(port));


    //생성한 소켓을 배치
    if(bind(serverSock,(SOCKADDR*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR)
        ErrorHandling("bind() error");


    //소켓을 받는 코드
	if(listen(serverSock,5)==SOCKET_ERROR)
		ErrorHandling("listen() error");
    

    printf("listening...\n");

    
	while(1){// 무한루프 

        //클라이언트 소켓 관리
		clientAddrSize=sizeof(clientAddr);
		clientSock=accept(serverSock,(SOCKADDR*)&clientAddr,&clientAddrSize);
        

        printf("Connected Client IP : %s\n",inet_ntoa(clientAddr.sin_addr));
        SOCKET* newClientSock = malloc(sizeof(SOCKET));
        *newClientSock = clientSock;
        hThread = (HANDLE)_beginthreadex(NULL,0,MAIN,(void*)newClientSock,0,NULL);
        CloseHandle(hThread);
        
    }
    
    closesocket(serverSock);//생성한 소켓을 끈다.
	WSACleanup();
    return 0;
}

//유저정보 연결리스트 생성
User* User_list() {
    FILE *ul;
    User *temp = NULL, *list = NULL;
    char ch[256];

    if ((ul = fopen ("user_information.txt", "r")) == NULL) 
    {
        printf("file open error");
        exit(-1);
    }

    while (fgets(ch, sizeof(ch), ul))
    {
        if (strlen(ch) > 0 && ch[strlen(ch) - 1] == '\n') 
        {
            ch[strlen(ch) - 1] = '\0';
        }

        User *temp = (User*)malloc(sizeof(User));

        strcpy(temp -> id, strtok(ch,"\t"));
        strcpy(temp -> password, strtok(NULL,"\t"));
        temp -> next = list;
        list = temp;
    }

    fclose(ul);
    return list;
}

//의원 정보 연결리스트
Clinic* Clinic_list() {
    FILE *cl;
    Clinic *temp = NULL, *list = NULL;
    char ch[10000];
    //char *ch = (char*)malloc(sizeof(char));

    printf("Clinic file reading starting\n");
    if ((cl = fopen("Asan_clinic.txt", "r")) == NULL)
    {
        printf("file open error");
        exit(-1);
    }

    //sprintf("file open starting\n");
    while (fgets(ch, sizeof(ch), cl))
    {
        ch[strlen(ch) - 1] = '\0';
        temp = (Clinic*)malloc(sizeof(Clinic));

        char *token = strtok(ch, "\t");
        if (!token) continue;
        strcpy(temp->major, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->name, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->address, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->number, token);

        //printf("%s, %s, %s, %s\n", temp->major,temp->name,temp->address,temp->number);

        temp -> next = list;
        list = temp;
    }
    
    fclose(cl);
    return list;
}

// 약국 정보 연결리스트
Pharmacy* Pharmacy_list() {
    FILE *pl;
    Pharmacy *temp = NULL, *list = NULL;
    char ch[256];

    printf("Pharmacy file reading starting\n");
    if ((pl = fopen("Asan_pharmacy.txt", "r")) == NULL)
    {
        printf("file open error");
        exit(-1);
    }

    while (fgets(ch, sizeof(ch), pl))
    {
        ch[strlen(ch) - 1] = '\0';
        Pharmacy* temp = (Pharmacy*)malloc(sizeof(Pharmacy));

        char *token = strtok(ch, "\t");
        if (!token) continue;
        strcpy(temp->major, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->name, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->address, token);

        token = strtok(NULL, "\t");
        if (!token) continue;
        strcpy(temp->number, token);
        
        //printf("%s, %s, %s, %s\n", temp->major,temp->name,temp->address,temp->number);

        temp -> next = list;
        list = temp;
    }
    
    fclose(pl);
    return list;
}


unsigned WINAPI MAIN (void* arg) {
    SOCKET clientSock=*((SOCKET*)arg);
    
    //로그인 반복문
    while(1) 
    {
        printf("login create loop\n");
        char msg[BUF_SIZE] = {0};

        recv(clientSock, msg, BUF_SIZE, 0);
        char *Login_tag = msg;
        //printf("%s\n", Login_tag);

            
        if(strcmp(Login_tag, "LO") == 0)
        {
            printf("login\n");
            
            UserLoginThread = (HANDLE)_beginthreadex(NULL,0,User_Login,(void*)&clientSock,0,NULL);
            WaitForSingleObject(UserLoginThread,INFINITE);

            int login;

            WaitForSingleObject(hMutex,INFINITE);
            login =login_true;
            login_true = 0;
            ReleaseMutex(hMutex);

            if(login == 1)
            {
                printf("login success\n");
                WaitForSingleObject(hMutex,INFINITE);
                strcpy(clientSocks[clientCount].id ,clientid);
                //printf("%s\n", clientSocks[clientCount].id);
                clientSocks[clientCount].clientSock=clientSock;
                //printf("%d\n", clientSock);
                clientCount++;
                ReleaseMutex(hMutex);
                
                break;
            }
        }

        if(strcmp(Login_tag, "CR") == 0)
        {
            printf("create\n");
            
            UserAddThread = (HANDLE)_beginthreadex(NULL,0,User_Add,(void*)&clientSock,0,NULL);
            WaitForSingleObject(UserAddThread,INFINITE);
        }

    }

    printf("loop excape\n");

    while (1)
    {
        char msg[BUF_SIZE] = {0};

        printf("menu loop\n");

        recv(clientSock, msg, BUF_SIZE, 0);
        char *menu_tag = msg;

        printf("%s\n", menu_tag);

        if (strcmp(menu_tag, "CN") == 0)
        {
            printf("clinic search\n");
            ClinicSearchThread=(HANDLE)_beginthreadex(NULL,0,Clinic_Search,(void*)&clientSock,0,NULL);
            WaitForSingleObject(ClinicSearchThread,INFINITE);
            printf("clinic search Exit\n");
        }
        if (strcmp(menu_tag, "AD") == 0)
        {
            printf("Clinic advice\n");
            ClinicAdviceThread=(HANDLE)_beginthreadex(NULL,0,Clinic_Advice,(void*)&clientSock,0,NULL);
            WaitForSingleObject(ClinicAdviceThread,INFINITE);
            printf("Clinic advice Exit\n");
        }
        if (strcmp(menu_tag, "EX") == 0)
        {
            printf("Program Exit\n");
            ProgramExitThread=(HANDLE)_beginthreadex(NULL,0,Program_Exit,(void*)&clientSock,0,NULL);
            WaitForSingleObject(ProgramExitThread,INFINITE);
            printf("Program Exit Exit\n");
            break;
        }
        
    }

    
    return 0;
}

//로그인 함수 
unsigned WINAPI User_Login(void* arg) {
    SOCKET clientSock=*((SOCKET*)arg);
    User *head = User_list();
    User *temp = head;
    char recvmsg[BUF_SIZE] = {0};

    recv(clientSock, recvmsg, BUF_SIZE, 0);
    char *user_information = recvmsg;

    char *id = strtok(user_information, "/");
    char *password = strtok(NULL, "/");

    printf("%s\n", id);
    printf("%s\n", password);

    while (temp != NULL)
    {
        if (strcmp(id, temp->id) == 0 && strcmp(password, temp->password) == 0)
        {
            printf("connet > %s\n", id);
            send(clientSock, "1", 1, 0);
            

            WaitForSingleObject(hMutex,INFINITE);
            login_true = 1;
            strcpy(clientid, id);
            ReleaseMutex(hMutex);

            return 0;
        }

        temp = temp->next;
    }
    
    send(clientSock, "0", 1, 0);
    
    WaitForSingleObject(hMutex,INFINITE);
    login_true = 0;
    ReleaseMutex(hMutex);

    return 0;
}

// 회원가입 함수
unsigned WINAPI User_Add(void* arg) {
    SOCKET clientSock=*((SOCKET*)arg);
    User *head = User_list();
    User *temp = head;
    FILE *AD;
    char msg[BUF_SIZE] = {0};

    recv(clientSock, msg, BUF_SIZE, 0);
    char *user_information = msg;

    char *id = strtok(user_information, "/");
    char *password = strtok(NULL, "/");

    head = UserAdd(temp, id, password);

    if ((AD = fopen ("C:\\Cprogram\\Online_national_medical_center\\user_information.txt", "a")) == NULL) {
        printf ("file open error!");
        exit (-1);
    }
    else
    {
        sprintf(msg,"%s\t%s", id, password);
        fprintf(AD, "%s\n", msg);
        fclose(AD);
    }

    //printf("화윈가입 완료");

    send(clientSock, "0", 1, 0);

    return 0;
}

// 유저추가 함수
User *UserAdd (User *list, char *id, char *password) {
    User *current = NULL, *follow = NULL, *newnode = NULL;
    current = follow = list;
    
    if ((newnode = (User *) malloc (sizeof (User))) == NULL) 
    {
        printf ("No memory allocated..\n");
        return NULL;
    }
    
    strcpy(newnode->id, id);
    strcpy(newnode->password, password);

    printf("id: %s\tpassword: %s\n", id, password);
    
    while (current != NULL) {
    follow = current;
    current = current->next;
    }
    
    newnode->next = current;
    if (current == list) list = newnode;
    else follow->next = newnode;

    
    return list;
}


unsigned WINAPI Clinic_Search(void* arg) {
    printf("unsigned WINAPI Clinic_Search(void* arg) starting\n");
    SOCKET clientSock=*((SOCKET*)arg);
    Clinic *head = Clinic_list();
    Clinic *temp = head;
    Clinic cliniclist[500] = {0};

    int count = 0;
    char num_list[10] = {0};
    char address_asan[BUF_SIZE] = {0};
    char recvmsg[BUF_SIZE] = {0};
    char sendmsg[BUF_SIZE] = {0};
    memset(sendmsg, 0, BUF_SIZE);


    recv(clientSock, recvmsg, BUF_SIZE, 0);
    printf("%s\n", recvmsg);

    char *switch_num = strtok(recvmsg, "/");

    if (strcmp(switch_num, "1") == 0)
    {
        printf("1 strat\n");
        char *major = strtok(NULL, "/");
        printf("%s\n", major);
        while (temp != NULL)
        {

            if (temp->major != NULL && strcmp(major, temp->major) == 0)
            {
                printf("%d\n", count);
                strcpy(cliniclist[count].major, temp->major);
                strcpy(cliniclist[count].name, temp->name);
                strcpy(cliniclist[count].address, temp->address);
                strcpy(cliniclist[count].number, temp->number);

                printf("%s, %s, %s, %s\n",temp->major,temp->name,temp->address,temp->number);

                count++;
            }

            temp = temp -> next;
        }
        
    }


    if (strcmp(switch_num, "2") == 0)
    {
        printf("2 strat\n");
        char *address = strtok(NULL, "/");

        while (temp != NULL)
        {

            if (temp->address != NULL && strstr(temp->address, address) != NULL)
            {
                printf("%d\n", count);
                strcpy(cliniclist[count].major, temp->major);
                strcpy(cliniclist[count].name, temp->name);
                strcpy(cliniclist[count].address, temp->address);
                strcpy(cliniclist[count].number, temp->number);

                printf("%s, %s, %s, %s\n",temp->major,temp->name,temp->address,temp->number);

                count++;
            }

            temp = temp -> next;
        }
        
    }

    
    if (strcmp(switch_num, "3") == 0)
    {
        printf("3 strat\n");
        char *major = strtok(NULL, "/");
        char *address = strtok(NULL, "/");
        // strcat(address_asan, "충청남도 아산시 ");
        // strcat(address_asan, address);
        // printf("%s\n", address_asan);
        // int num = strlen(address);
        // printf("%d\n", num);

        while (temp != NULL)
        {

            if (temp->address != NULL && strstr(temp->address, address) != NULL && strstr(temp->major, major) != NULL)
            {
                printf("%d\n", count);
                strcpy(cliniclist[count].major, temp->major);
                strcpy(cliniclist[count].name, temp->name);
                strcpy(cliniclist[count].address, temp->address);
                strcpy(cliniclist[count].number, temp->number);

                printf("%s, %s, %s, %s\n",temp->major,temp->name,temp->address,temp->number);

                count++;
            }

            temp = temp -> next;
        }
        
    }

    sprintf(num_list,"%d", count);
    printf("%s\n", num_list);
    send(clientSock, num_list, strlen(num_list), 0);

    for (int i = 0; i < count; i++)
    {
        printf("%d\n", i);
        memset(sendmsg, 0, 500);
        sprintf(sendmsg,"major: %s | name: %s | address: %s | number: %s\n", cliniclist[i].major, cliniclist[i].name, cliniclist[i].address, cliniclist[i].number);
        //printf("%s, %s, %s, %s\n", cliniclist[i].major, cliniclist[i].name, cliniclist[i].address, cliniclist[i].number);
        send(clientSock, sendmsg, strlen(sendmsg), 0);
        //printf("%s", sendmsg);

        Sleep(100);
    }

    memset(recvmsg, 0, BUF_SIZE);
    // 여기에 서브 시스템 구현
    recv(clientSock, recvmsg, BUF_SIZE, 0);
    printf("%s\n", recvmsg);

    if (strcmp(recvmsg, "PH") == 0)
    {
        Pharmacy_Serach(clientSock);
    }
    else
    {
        return 0;
    }
    

    return 0;
}


void Pharmacy_Serach(SOCKET clientSock) {
    printf("Pharmacy_Serach(SOCKET clientSock) starting\n");
    Pharmacy *head1 = Pharmacy_list();
    Pharmacy *temp1 = head1;
    Pharmacy pharmacylist[500] = {0};

    Clinic *head2 = Clinic_list();
    Clinic *temp2 = head2;

    int count = 0;
    char num_list[10] = {0};
    char recvmsg[BUF_SIZE] = {0};
    char sendmsg[BUF_SIZE] = {0};
    char name[100] = {0};
    char address[43] = {0};


    recv(clientSock, recvmsg, BUF_SIZE, 0);
    printf("%s\n", recvmsg);

    strcpy(name, recvmsg);

    while (temp2 != NULL)
    {
        if (temp2->name != NULL && strcmp(name, temp2->name) == 0)
        {
            strncpy(address, temp2 -> address, sizeof(address) - 1);
            address[sizeof(address)] = '\0';
            printf("%s\n", address);
            break;
        }
        temp2 = temp2->next;
    }

    while (temp1 != NULL)
    {
        if (strstr(temp1->address, address) != NULL)
        {
            strcpy(pharmacylist[count].name, temp1->name);
            strcpy(pharmacylist[count].address, temp1->address);
            strcpy(pharmacylist[count].number, temp1->number);
            count++;
        }
        temp1 = temp1->next;
    }

    sprintf(num_list,"%d", count);
    printf("%s\n", num_list);

    if (count == 0)
    {
        send(clientSock, "근처에 약국이 없습니다.", BUF_SIZE, 0);
    }
    else
    {
        send(clientSock, num_list, strlen(num_list), 0);
    }

    for (int i = 0; i < count; i++)
    {
        printf("%d\n", i);
        memset(sendmsg, 0, 500);
        sprintf(sendmsg,"name: %s | address: %s | number: %s\n", pharmacylist[i].name, pharmacylist[i].address, pharmacylist[i].number);
        //printf("%s, %s, %s, %s\n", cliniclist[i].major, cliniclist[i].name, cliniclist[i].address, cliniclist[i].number);
        send(clientSock, sendmsg, strlen(sendmsg), 0);
        //printf("%s", sendmsg);

        Sleep(100);
    }
    
    return;
}


unsigned WINAPI Clinic_Advice (void* arg) {
    printf("unsigned WINAPI Clinic_Advice (void* arg) starting\n");
    SOCKET clientSock = *((SOCKET*)arg);
    char recvmsg[BUF_SIZE];
    int recvLen;

    while (1)
    {
        memset(recvmsg, 0, BUF_SIZE);

        recv(clientSock, recvmsg, BUF_SIZE, 0);

        char name_msg[BUF_SIZE];
        strcpy(name_msg, recvmsg);

        char *name = strtok(name_msg, "/");
        char *msg = strtok(NULL, "/");
        char *id = NULL;

        // 받을 사람 찾기
        SOCKET clientsocket1 = 0, clientsocket2 = 0;
        WaitForSingleObject(hMutex, INFINITE);
        for (int i = 0; i < clientCount; i++)
        {
            if (strcmp(name, clientSocks[i].id) == 0)
            {
                clientsocket1 = clientSocks[i].clientSock;
                break;
            }
        }
        ReleaseMutex(hMutex);

        // 보낸 사람 찾기
        WaitForSingleObject(hMutex, INFINITE);
        for (int i = 0; i < clientCount; i++)
        {
            if (clientSock == clientSocks[i].clientSock)
            {
                id = clientSocks[i].id;
                clientsocket2 = clientSocks[i].clientSock;
                break;
            }
        }
        ReleaseMutex(hMutex);


        if (clientsocket1 == 0) 
        {
            printf("보낼 사람 ID를 찾을 수 없음\n");
            send(clientsocket2, "r", 1, 0);
            return 0;
            //break;
        }

        // 종료 메시지 처리
        // 서버에서 종료 메시지 처리 시
        
        if (msg && strcmp(msg, "q") == 0)
        {
            if (clientsocket1 != 0) 
            {
                send(clientsocket1, "q", 1, 0);
                send(clientsocket2, "q", 1, 0);
                return 0;

            }

            return 0;
            //break;
        }
        

        // 메시지 전송
        char sendmsg[BUF_SIZE];
        memset(sendmsg, 0, BUF_SIZE);
        sprintf(sendmsg, "%s: %s", id, msg);
        
        printf("메시지 전송: %s\n", sendmsg);
        
        send(clientsocket1, sendmsg, strlen(sendmsg) + 1, 0);
    }

    printf("Clinic_Advice 스레드 종료\n");
    return 0;
}


unsigned WINAPI Program_Exit(void* arg) {
    SOCKET clientSock=*((SOCKET*)arg);

    printf("유저와 연결이 끊겼습니다.\n");
    send(clientSock,"0",1,0);
    
    WaitForSingleObject(hMutex,INFINITE);

    for( int i=0; i < clientCount; i++) 
    {
        if(clientSock == clientSocks[i].clientSock)
        {
            while(i++ < clientCount-1)
            {
                strcpy(clientSocks[i].id, clientSocks[i+1].id);
                clientSocks[i].clientSock = clientSocks[i+1].clientSock;
            }
                
            break;
        }
    }

    clientCount--;
	ReleaseMutex(hMutex);
    closesocket(clientSock);

	return 0;
}

// 오류 메시지 출력함수
void ErrorHandling(char* msg){
	fputs(msg,stderr);
	fputc('\n',stderr);
	exit(1);
}

