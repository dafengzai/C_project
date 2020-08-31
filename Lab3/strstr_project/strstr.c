#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool
#include<time.h>

int mem = 0;
int nodenum = 0;
long long int cmpnum = 0;
int cmpcount = 0;

void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	nodenum++;
	return malloc(size);
}

typedef struct node {
	char *data;
	int priority; //priotity:
	struct node* next;
}Node;
Node* newNode(char* str, int len, int p); // Function to Create A New Node 
Node* peek(Node** head); // Return the value at head 
void pop(Node** head);
void push(Node** head, char* str, int len, int p);
int isEmpty(Node** head);

int stringCountNum(char* str, int len, char* s);
int main()
{
	//计时
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*大致流程分两部分：
	一是读取pattern.txt。做一个大循环，在每个字符串的循环内进行查找（硬来版），并先写入output.txt先
	二：把output文件读取进来并排序，之后导出到result.txt
	*/

	/*
	读取pattern.txt文件，做个大循环
	*/
	FILE* fp = fopen("../../pattern_bf_kmp.txt", "r");
	FILE* fpwrite = fopen("output.txt", "w");

	/*
	为加快IO速度，将string.txt全读进内存
	*/
	char* s = (char*)bupt_malloc(sizeof(char) * 1024 * 1024 *880); //一个char就占一个字节B，1024B=1KB,string.txt有877MB
	FILE* fpstr = fopen("../../string.txt", "r");
	char *tempchar = s;
	int test=0;
	while ((*tempchar = (char)fgetc(fpstr)) != EOF) {
		if (*tempchar == '\n') continue;
		tempchar++;
		//test++;
		//if (test>10000000)
		//	break;
	}
	*tempchar = '\0'; //结尾判断
	fclose(fpstr);
	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
	start_t = clock();
	printf("将string.txt读进内存占用的总时间：%f\n", total_t);

	unsigned int M = 1500;
	//测试
	int check = 0;
	//unsigned int M = 10;
	for (int i = 0; i < M; i++) {
		char str[150];
		fgets(str, 150, fp);
		str[strlen(str) - 1] = '\0';
		int len = strlen(str);

		/*
		开始查找
		*/

		int count = 0; //模式串出现次数
		count = stringCountNum(str, len, s);



		//写入结果output
		fprintf(fpwrite, "%s %d\n", str, count);

		
		
		//计时 3.1秒一次
		//finish_t = clock();
		//total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒
		//start_t = finish_t;
		//printf("CPU 占用的总时间：%f\n", total_t);
		check++;
		if (check % 100 == 0)
			printf("%d patterns are searched.\n", check);
	}

	fclose(fp);
	fclose(fpwrite);
	
	//把output文件读取进来并排序，之后导出到result.txt
	fp = fopen("output.txt", "r");
	fpwrite = fopen("result.txt", "w");

	//使用优先队列实现（通过链表实现，别用堆为难自己了）
	
	//读取第一个元素
	char tem[50];
	int temval = 0;
	fscanf(fp, "%s", tem);
	fscanf(fp, "%d", &temval);
	int len = strlen(tem); //求len没问题
	Node* top = newNode(tem, len, temval);
	
	for (int i = 1; i < M; i++) {
		fscanf(fp, "%s", tem);
		fscanf(fp, "%d", &temval);
		//tem[strlen(tem) - 1] = '\0';
		len = strlen(tem); //求len没问题
		
		push(&top, tem, len, temval);

	}

	//结果写入result.txt
	while (!isEmpty(&top)) {
		//Node* tem = peek(&top);
		printf("%s %d\n", top->data, top->priority);

		fprintf(fpwrite, "%s %d\n", top->data, top->priority);
		pop(&top);
	}

	//写入最后一行

	fprintf(fpwrite, "%lld %d\n", cmpnum/1000, mem/1024); //最后一行输出两个数：字节比较次数K 内存开销（KB）

	fclose(fp);
	fclose(fpwrite);

	//计时
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//将时间转换为秒

	printf("CPU 占用的总时间：%f\n", total_t);

	return 0;

}
int stringCountNum(char* str, int len, char* s) {
	//逐字符串读取并比较
	char *c = s;
	int count = 0; //模式串出现次数
	//因为判断不正确后需要回溯到当时比较char的下一个，所以需要一个char数组暂存。
	//char tem[150];
	//int temlen = 0;
	//int temindex = 1; //待匹配字符串需后移一位
	int index = 0; //当前查找到str中的哪一个元素
	char* tempchar = c + 1; //用于不匹配时的回溯
	//while (*c != '\0') {
	while (*c != '\0') {
		cmpnum++;
		if (*c != str[index]) { //查找失败
			if (index == 0) {//直接找下一个即可
				c++;
				continue;
			}
			else {//index != 0 还需额外的处理
				index = 0; //匹配失败了，重新来比较
				//改
				c = tempchar;
				/*
				while (1) {
					cmpnum++;
					if (tem[temindex] == str[index]) {
						index++;
					}
					temindex++;
					if (temindex >= temlen) { //tem字符串已经匹配完了，更新下
						int pretemlen = temlen;
						temlen = index;
						for (int j = 0; j < temlen; j++) {
							tem[j] = tem[pretemlen - temlen + j];
						}
						temindex = 1; //
						break;
					}
				}
				*/
			}//经过while处理后匹配了tem[]内元素，并把其更新
		}
		else { //查找成功:c == str[index]
			if (index == 0) { //首个元素匹配
				tempchar = c + 1;
			}
			index++;
			if (index >= len - 1) { //模式串匹配，最后一个是\0，只需len-1
				count++;
				//temlen = 0;
				//temindex = 1;
				index = 0;
			}
		}
		c++;
	}
	return count;
}
//优先队列实现
Node* newNode(char* str, int len, int p)
{
	Node* temp = (Node*)malloc(sizeof(Node));
	//memset(temp, 0, sizeof(Node)); //初始化：不行，会把data指针变NULL
	temp->data = (char*)malloc(sizeof(char)* len + 1); //指针变量记得分配内存
	//*(temp->data) = *str; 这赋值的是头文件
	for (int i = 0; i < len; i++) {
		*(temp->data + i) = *(str + i);
	}
	*(temp->data + len) = '\0'; //strlen不会计入'\0'，得自己加
	temp->priority = p;
	temp->next = NULL;

	return temp;
}
// Return the value at head 

Node* peek(Node** head)
{
	return *head;
}

// Removes the element with the 
// highest priority form the list 
void pop(Node** head)
{
	Node* temp = *head;
	(*head) = (*head)->next;
	free(temp);
}

// Function to push according to priority 
void push(Node** head, char* str, int len, int p)
{
	Node* start = (*head);

	// Create new Node 
	Node* temp = newNode(str, len,p);

	// Special Case: The head of list has lesser 
	// priority than new node. So insert new 
	// node before head node and change head node. 
	if ((*head)->priority < p) { //本例为priority大的在头

		// Insert New Node before head 
		temp->next = *head;
		(*head) = temp;
	}
	else {

		// Traverse the list and find a 
		// position to insert new node 
		while (start->next != NULL &&
			start->next->priority > p) {
			start = start->next;
		}

		// Either at the ends of the list 
		// or at required position 
		temp->next = start->next;
		start->next = temp;
	}
}

// Function to check is list is empty 
int isEmpty(Node** head)
{
	return (*head) == NULL;
}