#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool
#include<time.h>
//多线程库
#include<process.h>>
/*  函数所需头文件 */
//Windows 下的 socket 程序依赖 Winsock.dll 或 ws2_32.dll
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
//正则匹配库
#define PCRE2_CODE_UNIT_WIDTH 8
#include<pcre2.h>

#define branch 37 
#define MAXBYTE 500000

int urlNumber = 0; //用于标记网页的编号

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
爬虫流程
l waitL=firsturl，fetchedL=null
2 While waitL不为null：
	* 取出waitL中的队列头url
	* send()发送请求到服务器
	* Page=recv（url）等待返回网页内容
	* fetchedL.add(url)
	* Ulist = parseurl(page)
	* Ulist=Ulist-fetchedL)//去重
	* waitL=waitL.add(Ulist)

具体的网页获取使用Socket接口
*/

//维护一个waitList
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
//waitL的插入与删除函数
void waitListInserUrl(char* url, waitList* waitL);
void waitListDeleteUrl(waitList* waitL);

/*
//fetchList: 用于存储所有爬取过了网页
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
//fetchL的插入
void fetchListAdd(char* url, fetchList* fetchL);
*/

/*
使用trie树来进行爬取的url存储与去重，同时在末尾节点存储链接关系
*/
typedef struct trienode {
	struct trienode* chidren[branch]; //本例为37叉树,26字母+10数字+1其它
	char end; //是否能终结
	int number; //本网页的编号
	//int* links; //网页链接关系：该网页能到达的外部网页
	//int linksNum;
}trienode, * trietree;
trienode* CreatTrie();
void InsertNode(trietree root, char* str); //得返回一个指向末尾的指针，方便后续添加链接网页
int FindNode(trietree root, char* str); //返回-1表示查找失败
trienode* GetNode(trietree root, char* str); //返回该网页在trie树中的尾节点
//void PrintTrieUrls(trietree root,  fetchList* fetchL, FILE* fpwrite);
//void PrintTrieLinks(trietree root, fetchList* fetchL, FILE* fpwrite);

//多线程函数
unsigned int _stdcall ThreadProc(LPVOID lpParameter)
{
	printf("hehe\n");
	return 0;
}

int main()
{
	//计时
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*
	爬虫流程
	l waitL=firsturl，fetchedL=null
	*/
	waitNode* firstu = (waitNode*)bupt_malloc(sizeof(waitNode));
	firstu->url = "/"; //初始url
	firstu->next = NULL;
	waitList* waitL = (waitList*)bupt_malloc(sizeof(waitList));
	waitL->head = firstu;
	waitL->tail = firstu;
	waitL->count = 1;

	trietree root = CreatTrie();
	//为防止重复，waitL添加元素时就得把该元素也放进trie树内以去重
	InsertNode(root, waitL->head->url);
	
	//结果写入url.txt
	FILE* fpwrite = fopen("url.txt", "w");

	//多线程
	/*
	uintptr_t _beginthreadex( // NATIVE CODE
	   void *security,  //线程安全属性
	   unsigned stack_size,  //线程的栈大小
	   unsigned ( *start_address )( void * ),//线程函数
	   void *arglist,  //传递到线程函数中的参数
	   unsigned initflag,  //线程初始化标记
	   unsigned *thrdaddr   //线程ID
	);
	*/
	//_beginthreadex(NULL, 1000, ThreadProc, 0, 0, NULL);

	//初始化DLL
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	//创建套接字
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//向服务器发起请求
	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
	sockAddr.sin_family = AF_INET; //PF_INET;
	sockAddr.sin_addr.s_addr = inet_addr("10.108.106.165");
	sockAddr.sin_port = htons(80); //TCP端口号
	connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
	

	/*
	爬虫流程
	2 While waitL不为null：
		* 取出waitL中的队列头url
		* send()发送请求到服务器
		* Page=recv（url）等待返回网页内容
		* fetchedL.add(url)
		* Ulist = parseurl(page)
		* Ulist=Ulist-fetchedL)//去重
		* waitL=waitL.add(Ulist)
	*/
	int k = 0;
	/*
	无法获得页面的原因不是404或别的之类的。
	是receiver根本接收不到消息（szBuffer是空的），TCP链接直接就断掉了
	*/
	//test
	int flag = 0;
	while (waitL->count != 0)
	{
		if (flag % 1000 == 0) {
			printf("Have got %d pages\n", flag);
			finish_t = clock();
			total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
			printf("CPU 占用的总时间：%f\n", total_t);
		}
		//if (flag > 2000)
		//	break;
		flag++;

		if (k % 50 == 2) {
			//创建套接字
			sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

			//向服务器发起请求
			struct sockaddr_in sockAddr;
			memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
			sockAddr.sin_family = PF_INET;
			sockAddr.sin_addr.s_addr = inet_addr("10.108.106.165");
			sockAddr.sin_port = htons(80); 
			connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
		}

		//向服务器发送消息
		//char* SendBuffer = (char*)bupt_malloc(sizeof(char) * 200); //要向服务器发送的数据
		char SendBuffer[200] = { 0 };
		/*
		char* SendBuffer = "GET /news.sohu.com/20151105/n425340125.shtml HTTP/1.1\r\n" //在http1.1中默认是使用长连接的，即Connection的值为Keep-alive
			"Host:10.108.106.165\r\n"
			"Connection: Keep-Alive\r\n\r\n";
							//"Connection: Close\r\n\r\n";
		*/
		//后续得用类似news.sohu.com/20151105/n425340125.shtml的形式

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
		//写入从waitL取出的url信息
		fprintf(fpwrite, "%s %d\n", waitL->head->url, currNode->number);
		fprintf(fpwrite, "\n");

		//printf("Receiving page %s with number %d\n", waitL->head->url, currNode->number);
		//处理waitL 
		waitListDeleteUrl(waitL);

		int ret = send(sock, SendBuffer, strlen(SendBuffer), 0);
		//free(SendBuffer);

		//接收服务器传回的数据
		char szBuffer[MAXBYTE] = { 0 };
		recv(sock, szBuffer, MAXBYTE, NULL);
		//rintf("响应（200/404）server: %c\n", szBuffer[9]); //szBuffer[9]='2'表示200，我们只需处理返回200的成功相应页面
		//printf("Message form server: %s\n", szBuffer);
		//只处理返回值为200的成功页面
		
		//if (szBuffer[9] != '2') { //不是200的成功响应
			//printf("test: %d\n", szBuffer[9] - '0');
			//printf("Can't get page with %c%c%c\n", szBuffer[9], szBuffer[10], szBuffer[11]);
			//k = 1; //出问题了重开socket
			//continue;
		//}
		//printf("Success receive a new page\n");
		//对返回的页面内容szBuffer使用pcre2库进行正则匹配URL提取
		pcre2_code* re;
		PCRE2_SPTR pattern;     /* PCRE2_SPTR is a pointer to unsigned code units of */
		PCRE2_SPTR subject;     /* the appropriate width (8, 16, or 32 bits). */
		PCRE2_SIZE erroroffset;
		int errornumber;
		pattern = (PCRE2_SPTR)"\"http://news.sohu.com..*?\"";  //trick:数据库里只有news.sphu.com/
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
			//输出匹配结果
			for (i = 0; i < rc; i++)
			{
				char* substring_start = subject + ovector[2 * i] + 7; //把前面http://忽略
				int substring_length = ovector[2 * i + 1] - ovector[2 * i] - 7 - 1; //末尾的"去掉
				char* matched = (char*)bupt_malloc(sizeof(char) * 100); //不能用char matched[100],没进行分配的话最后会消失掉的
				//matchurls[matchurlsLen] = (char*)bupt_malloc(sizeof(char) * 100); //这里有问题
				memset(matched, 0, 100);
				strncpy(matched, substring_start, substring_length);
				int tempNum = -1;
				if ((tempNum = FindNode(root, matched)) != -1) { //该网页已被匹配过了
					fprintf(fpwrite, "%d %d\n", currNode->number, tempNum);
					//printf("重复urlnum: is %d\n",tempNum);
					continue;
				}
				fprintf(fpwrite, "%d %d\n", currNode->number, urlNumber);
				//printf("全部匹配结果有: %s with number %d\n", matched, urlNumber);
				waitListInserUrl(matched, waitL);
				InsertNode(root, matched);
				/*
				*/
			}
			start_offset = ovector[2 * (i - 1) + 1];
		}
		fprintf(fpwrite, "\n");
		//将正则匹配得到的urls进行后续操作
		//free(szBuffer);

		//printf("There are %d elements in waitL\n", waitL->count);
		//这步要涉及到多线程的处理
		if (k % 50 == 1) {
			//关闭套接字
			closesocket(sock);  
			//printf("Restarting socket\n");
			//Sleep(1000); //冷静一下
		}
		k++;
	}

	//关闭套接字
	closesocket(sock);

	//终止使用 DLL
	WSACleanup();

	/*
	输出数据
	*/
	//写入最后一行
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
	fprintf(fpwrite, "%ld %f\n", mem  / 1024, total_t); //最后一行输出两个数：内存开销（KB）,运行时间

	fclose(fpwrite);

	return 0;

}

//waitL的插入与删除函数
void waitListInserUrl(char* url, waitList* waitL) {
	//尾部插入
	waitNode* temp = (waitNode*)bupt_malloc(sizeof(waitNode));
	//内存分配有问题
	//temp->url = (char*)bupt_malloc(sizeof(char) * 100);
	//初始url
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
	//头部删除
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


//trinode相关函数
trienode* CreatTrie() {
	trienode* node = (trienode*)bupt_malloc(sizeof(trienode));
	memset(node, 0, sizeof(trienode));
	node->number = -1;
	//在C语言中，NULL在实际底层调用中就是0,但是为了目的和用途及容易识别的原因，NULL用于指针和对象，0用于数值
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
		//对*temp进行大写变小写，非字母符号到26分支
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
		if (node->chidren[loc] == NULL) { //查找失败
			return -1;
		}
		node = node->chidren[loc];
		temp++;
	}
	if (node->end == 1) //存在该url
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
		if (node->chidren[loc] == NULL) { //查找失败
			printf("There's an error in getting url's node\n");
			return NULL;
		}
		node = node->chidren[loc];
		temp++;
	}
	if (node->end == 1) //存在该url
		return node;
	printf("There's an another error in getting url's node\n");
	//printf("current node's url numbers is %d\n", node->number);
	return NULL;
}
