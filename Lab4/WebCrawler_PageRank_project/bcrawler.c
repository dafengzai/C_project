#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool
#include<time.h>
//���߳̿�
#include<process.h>>
/*  ��������ͷ�ļ� */
//Windows �µ� socket �������� Winsock.dll �� ws2_32.dll
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
//����ƥ���
#define PCRE2_CODE_UNIT_WIDTH 8
#include<pcre2.h>

#define branch 37 
#define MAXBYTE 500000

int urlNumber = 0; //���ڱ����ҳ�ı��

long int mem = 0;
void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	//test
	//printf("size are %d\n", size);
	return malloc(size);
	
}

/*
��������
l waitL=firsturl��fetchedL=null
2 While waitL��Ϊnull��
	* ȡ��waitL�еĶ���ͷurl
	* send()�������󵽷�����
	* Page=recv��url���ȴ�������ҳ����
	* fetchedL.add(url)
	* Ulist = parseurl(page)
	* Ulist=Ulist-fetchedL)//ȥ��
	* waitL=waitL.add(Ulist)

�������ҳ��ȡʹ��Socket�ӿ�
*/

//ά��һ��waitList
typedef struct waitNode
{
	char* url;
	struct waitNode* next;
}waitNode;
typedef struct waitList
{
	waitNode* head, * tail;
	int count;
}waitList;
//waitL�Ĳ�����ɾ������
void waitListInserUrl(char* url, waitList* waitL);
void waitListDeleteUrl(waitList* waitL);

/*
//fetchList: ���ڴ洢������ȡ������ҳ
typedef struct fetchNode
{
	char* url;
	struct fetchNode* next;
}fetchNode;
typedef struct fetchList
{
	fetchNode* head, * tail;
	int count;
}fetchList;
//fetchL�Ĳ���
void fetchListAdd(char* url, fetchList* fetchL);
*/

/*
ʹ��trie����������ȡ��url�洢��ȥ�أ�ͬʱ��ĩβ�ڵ�洢���ӹ�ϵ
*/
typedef struct trienode {
	struct trienode* chidren[branch]; //����Ϊ37����,26��ĸ+10����+1����
	char end; //�Ƿ����ս�
	int number; //����ҳ�ı��
	//int* links; //��ҳ���ӹ�ϵ������ҳ�ܵ�����ⲿ��ҳ
	//int linksNum;
}trienode, * trietree;
trienode* CreatTrie();
void InsertNode(trietree root, char* str); //�÷���һ��ָ��ĩβ��ָ�룬����������������ҳ
int FindNode(trietree root, char* str); //����-1��ʾ����ʧ��
trienode* GetNode(trietree root, char* str); //���ظ���ҳ��trie���е�β�ڵ�
//void PrintTrieUrls(trietree root,  fetchList* fetchL, FILE* fpwrite);
//void PrintTrieLinks(trietree root, fetchList* fetchL, FILE* fpwrite);

//���̺߳���
unsigned int _stdcall ThreadProc(LPVOID lpParameter)
{
	printf("hehe\n");
	return 0;
}

int main()
{
	//��ʱ
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*
	��������
	l waitL=firsturl��fetchedL=null
	*/
	waitNode* firstu = (waitNode*)bupt_malloc(sizeof(waitNode));
	firstu->url = "/"; //��ʼurl
	firstu->next = NULL;
	waitList* waitL = (waitList*)bupt_malloc(sizeof(waitList));
	waitL->head = firstu;
	waitL->tail = firstu;
	waitL->count = 1;

	trietree root = CreatTrie();
	//Ϊ��ֹ�ظ���waitL���Ԫ��ʱ�͵ðѸ�Ԫ��Ҳ�Ž�trie������ȥ��
	InsertNode(root, waitL->head->url);
	
	//���д��url.txt
	FILE* fpwrite = fopen("url.txt", "w");

	//���߳�
	/*
	uintptr_t _beginthreadex( // NATIVE CODE
	   void *security,  //�̰߳�ȫ����
	   unsigned stack_size,  //�̵߳�ջ��С
	   unsigned ( *start_address )( void * ),//�̺߳���
	   void *arglist,  //���ݵ��̺߳����еĲ���
	   unsigned initflag,  //�̳߳�ʼ�����
	   unsigned *thrdaddr   //�߳�ID
	);
	*/
	//_beginthreadex(NULL, 1000, ThreadProc, 0, 0, NULL);

	//��ʼ��DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//�����׽���
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//���������������
	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
	sockAddr.sin_family = AF_INET; //PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("10.108.106.165");
	sockAddr.sin_port = htons(80); //TCP�˿ں�
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	

	/*
	��������
	2 While waitL��Ϊnull��
		* ȡ��waitL�еĶ���ͷurl
		* send()�������󵽷�����
		* Page=recv��url���ȴ�������ҳ����
		* fetchedL.add(url)
		* Ulist = parseurl(page)
		* Ulist=Ulist-fetchedL)//ȥ��
		* waitL=waitL.add(Ulist)
	*/
	int k = 0;
	/*
	�޷����ҳ���ԭ����404����֮��ġ�
	��receiver�������ղ�����Ϣ��szBuffer�ǿյģ���TCP����ֱ�ӾͶϵ���
	*/
	//test
	int flag = 0;
	while (waitL->count != 0)
	{
		if (flag % 1000 == 0) {
			printf("Have got %d pages\n", flag);
			finish_t = clock();
			total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
			printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);
		}
		//if (flag > 2000)
		//	break;
		flag++;

		if (k % 50 == 2) {
			//�����׽���
			sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

			//���������������
			struct sockaddr_in sockAddr;
			memset(&sockAddr, 0, sizeof(sockAddr));  //ÿ���ֽڶ���0���
			sockAddr.sin_family = PF_INET;
			sockAddr.sin_addr.s_addr = inet_addr("10.108.106.165");
			sockAddr.sin_port = htons(80); 
			connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
		}

		//�������������Ϣ
		//char* SendBuffer = (char*)bupt_malloc(sizeof(char) * 200); //Ҫ����������͵�����
		char SendBuffer[200] = { 0 };
		/*
		char* SendBuffer = "GET /news.sohu.com/20151105/n425340125.shtml HTTP/1.1\r\n" //��http1.1��Ĭ����ʹ�ó����ӵģ���Connection��ֵΪKeep-alive
			"Host:10.108.106.165\r\n"
			"Connection: Keep-Alive\r\n\r\n";
							//"Connection: Close\r\n\r\n";
		*/
		//������������news.sohu.com/20151105/n425340125.shtml����ʽ

		//test
		//if (strlen(waitL->head->url) < 20) { //"/news.sohu.com/"
		//	waitListDeleteUrl(waitL);
		//	continue;
		//}

		sprintf(SendBuffer, "GET %s HTTP/1.1\r\n"
			"Host:10.108.106.165\r\n"
			"Connection: Keep-Alive\r\n\r\n", waitL->head->url);
		trienode* currNode = GetNode(root, waitL->head->url);
		if (currNode == NULL) {
			waitListDeleteUrl(waitL);
			continue;
		}
		//д���waitLȡ����url��Ϣ
		fprintf(fpwrite, "%s %d\n", waitL->head->url, currNode->number);
		fprintf(fpwrite, "\n");

		//printf("Receiving page %s with number %d\n", waitL->head->url, currNode->number);
		//����waitL 
		waitListDeleteUrl(waitL);

		int ret = send(sock, SendBuffer, strlen(SendBuffer), 0);
		//free(SendBuffer);

		//���շ��������ص�����
		char szBuffer[MAXBYTE] = { 0 };
		recv(sock, szBuffer, MAXBYTE, NULL);
		//rintf("��Ӧ��200/404��server: %c\n", szBuffer[9]); //szBuffer[9]='2'��ʾ200������ֻ�账����200�ĳɹ���Ӧҳ��
		//printf("Message form server: %s\n", szBuffer);
		//ֻ������ֵΪ200�ĳɹ�ҳ��
		
		//if (szBuffer[9] != '2') { //����200�ĳɹ���Ӧ
			//printf("test: %d\n", szBuffer[9] - '0');
			//printf("Can't get page with %c%c%c\n", szBuffer[9], szBuffer[10], szBuffer[11]);
			//k = 1; //���������ؿ�socket
			//continue;
		//}
		//printf("Success receive a new page\n");
		//�Է��ص�ҳ������szBufferʹ��pcre2���������ƥ��URL��ȡ
		pcre2_code* re;
		PCRE2_SPTR pattern;     /* PCRE2_SPTR is a pointer to unsigned code units of */
		PCRE2_SPTR subject;     /* the appropriate width (8, 16, or 32 bits). */
		PCRE2_SIZE erroroffset;
		int errornumber;
		pattern = (PCRE2_SPTR)"\"http://news.sohu.com..*?\"";  //trick:���ݿ���ֻ��news.sphu.com/
		subject = (PCRE2_SPTR)szBuffer;
		int subject_length = strlen((char*)subject);
		/*************************************************************************
		* Now we are going to compile the regular expression pattern, and handle *
		* any errors that are detected.                                          *
		*************************************************************************/

		re = pcre2_compile(
			pattern,               /* the pattern */
			PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
			0,                     /* default options */
			&errornumber,          /* for error number */
			&erroroffset,          /* for error offset */
			NULL);                 /* use default compile context */

		  /* Compilation failed: print the error message and exit. */

		if (re == NULL)
		{
			//PCRE2_UCHAR buffer[256];
			//pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
			//printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
			//	buffer);
			//return 1;
			printf("PCRE2 compilation failed");
		}

		/*************************************************************************
		* If the compilation succeeded, we call PCRE again, in order to do a     *
		* pattern match against the subject string. This does just ONE match. If *
		* further matching is needed, it will be done below. Before running the  *
		* match we must set up a match_data block for holding the result.        *
		*************************************************************************/

		/* Using this function ensures that the block is exactly the right size for
		the number of capturing parentheses in the pattern. */
		pcre2_match_data* match_data;
		/*
		 match_data is an opaque structure that is accessed by function calls. In particular,
		 the match data block contains a vector of offsets into the subject string
		 that define the matched part of the subject and any substrings that
		 were captured. This is known as the ovector.
		*/
		match_data = pcre2_match_data_create_from_pattern(re, NULL);
		PCRE2_SIZE* ovector;
		/*************************************************************************
		int rc = pcre2_match(
			re,                   // the compiled pattern
			subject,              // the subject string
			subject_length,       // the length of the subject
			0,                    // start at offset 0 in the subject
			0,                    // default options
			match_data,           // block for storing the result
			NULL);                // use default match context
		*************************************************************************/
		/* Matching failed: handle error cases */

		int i = 0, rc = 0;
		int start_offset = 0;
		//char** matchurls = (char**)bupt_malloc(sizeof(char*) * 200);
		//int matchurlsLen = 0;
		while ((rc = pcre2_match(re, subject, \
			subject_length, start_offset, 0, match_data, NULL)) > 0)
		{
			/* Match succeded. Get a pointer to the output vector, where string offsets are
		stored. */

		//PCRE2_SIZE* ovector;
			ovector = pcre2_get_ovector_pointer(match_data);
			//printf("heihei %c\n", match_data->startchar);
			//���ƥ����
			for (i = 0; i < rc; i++)
			{
				char* substring_start = subject + ovector[2 * i] + 7; //��ǰ��http://����
				int substring_length = ovector[2 * i + 1] - ovector[2 * i] - 7 - 1; //ĩβ��"ȥ��
				char* matched = (char*)bupt_malloc(sizeof(char) * 100); //������char matched[100],û���з���Ļ�������ʧ����
				//matchurls[matchurlsLen] = (char*)bupt_malloc(sizeof(char) * 100); //����������
				memset(matched, 0, 100);
				strncpy(matched, substring_start, substring_length);
				int tempNum = -1;
				if ((tempNum = FindNode(root, matched)) != -1) { //����ҳ�ѱ�ƥ�����
					fprintf(fpwrite, "%d %d\n", currNode->number, tempNum);
					//printf("�ظ�urlnum: is %d\n",tempNum);
					continue;
				}
				fprintf(fpwrite, "%d %d\n", currNode->number, urlNumber);
				//printf("ȫ��ƥ������: %s with number %d\n", matched, urlNumber);
				waitListInserUrl(matched, waitL);
				InsertNode(root, matched);
				/*
				*/
			}
			start_offset = ovector[2 * (i - 1) + 1];
		}
		fprintf(fpwrite, "\n");
		//������ƥ��õ���urls���к�������
		//free(szBuffer);

		//printf("There are %d elements in waitL\n", waitL->count);
		//�ⲽҪ�漰�����̵߳Ĵ���
		if (k % 50 == 1) {
			//�ر��׽���
			closesocket(sock);  
			//printf("Restarting socket\n");
			//Sleep(1000); //�侲һ��
		}
		k++;
	}

	//�ر��׽���
	closesocket(sock);

	//��ֹʹ�� DLL
	WSACleanup();

	/*
	�������
	*/
	//д�����һ��
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
	fprintf(fpwrite, "%ld %f\n", mem  / 1024, total_t); //���һ��������������ڴ濪����KB��,����ʱ��

	fclose(fpwrite);

	return 0;

}

//waitL�Ĳ�����ɾ������
void waitListInserUrl(char* url, waitList* waitL) {
	//β������
	waitNode* temp = (waitNode*)bupt_malloc(sizeof(waitNode));
	//�ڴ����������
	//temp->url = (char*)bupt_malloc(sizeof(char) * 100);
	//��ʼurl
	//for (int i = 0; i < strlen(url); i++)
	//	(temp->url)[i] = url[i];
	temp->url = url;
	temp->next = NULL;
	if (waitL->count == 0) {
		waitL->head = waitL->tail = temp;
		waitL->count++;
		return;
	}
	waitL->tail->next = temp;
	waitL->tail = waitL->tail->next;
	waitL->count++;
}
void waitListDeleteUrl(waitList* waitL) {
	//ͷ��ɾ��
	if (waitL->count == 0) {
		printf("There's a error in deleting url\n");
	}
	waitNode* temp = waitL->head;
	waitL->head = waitL->head->next;
	waitL->count--;
	if (waitL->count == 0) waitL->tail = waitL->head = NULL;
	//test
	free(temp);
}


//trinode��غ���
trienode* CreatTrie() {
	trienode* node = (trienode*)bupt_malloc(sizeof(trienode));
	memset(node, 0, sizeof(trienode));
	node->number = -1;
	//��C�����У�NULL��ʵ�ʵײ�����о���0,����Ϊ��Ŀ�ĺ���;������ʶ���ԭ��NULL����ָ��Ͷ���0������ֵ
	return node;
}
void InsertNode(trietree root, char* str)
{
	int len = strlen(str);
	if (len == 0)
		return;
	char* temp = str;
	trienode* node = root;
	for (int i = 0; i < len; i++) {
		//��*temp���д�д��Сд������ĸ���ŵ�26��֧
		int loc = 0;
		if (*temp >= 'A' && *temp <= 'Z')
			loc = *temp - 'A';
		else if (*temp >= 'a' && *temp <= 'z')
			loc = *temp - 'a';
		else if (*temp >= '0' && *temp <= '9')
			loc = *temp - '0' + 26;
		else
			loc = 36;
		if (node->chidren[loc] == NULL) {
			trienode* tem = CreatTrie();
			node->chidren[loc] = tem;
		}
		node = node->chidren[loc];
		temp++;
	}
	node->end = 1;
	node->number = urlNumber;
	urlNumber++;
	//node->data = str;
	//printf("Inser a node: %s\n", node->data);
}
int FindNode(trietree root, char* str)
{
	trietree node = root;
	char* temp = str;
	for (int i = 0; i < strlen(str); i++) {
		int loc = 0;
		if (*temp >= 'A' && *temp <= 'Z')
			loc = *temp - 'A';
		else if (*temp >= 'a' && *temp <= 'z')
			loc = *temp - 'a';
		else if (*temp >= '0' && *temp <= '9')
			loc = *temp - '0' + 26;
		else
			loc = 36;
		if (node->chidren[loc] == NULL) { //����ʧ��
			return -1;
		}
		node = node->chidren[loc];
		temp++;
	}
	if (node->end == 1) //���ڸ�url
		return node->number;
	return -1;
}
trienode* GetNode(trietree root, char* str)
{
	trietree node = root;
	char* temp = str;
	for (int i = 0; i < strlen(str); i++) {
		int loc = 0;
		if (*temp >= 'A' && *temp <= 'Z')
			loc = *temp - 'A';
		else if (*temp >= 'a' && *temp <= 'z')
			loc = *temp - 'a';
		else if (*temp >= '0' && *temp <= '9')
			loc = *temp - '0' + 26;
		else
			loc = 36; 
		if (node->chidren[loc] == NULL) { //����ʧ��
			printf("There's an error in getting url's node\n");
			return NULL;
		}
		node = node->chidren[loc];
		temp++;
	}
	if (node->end == 1) //���ڸ�url
		return node;
	printf("There's an another error in getting url's node\n");
	//printf("current node's url numbers is %d\n", node->number);
	return NULL;
}
