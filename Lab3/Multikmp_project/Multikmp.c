#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99ûbool
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
	char* data;
	int priority; //priotity:
	struct node* next;
}Node;
Node* newNode(char* str, int len, int p); // Function to Create A New Node 
Node* peek(Node** head); // Return the value at head 
void pop(Node** head);
void push(Node** head, char* str, int len, int p);
int isEmpty(Node** head);

void getNext(char* str, int* next);
int kmpStringCountNum(char* str, int len, int* next, char* s);
int main()
{
	//��ʱ
	clock_t start_t, finish_t;
	double total_t = 0;
	start_t = clock();

	/*
	�ⲻ����ÿһ������������kmp�㡣
	�������̷ֶ����֣�
	һ����ȡpattern.txt����һ����ѭ������ÿ���ַ�����ѭ���ڽ��в��ң�kmp�棩������д��output.txt��
	������output�ļ���ȡ����������֮�󵼳���result.txt
	*/

	/*
	Ϊ�ӿ�IO�ٶȣ���string.txtȫ�����ڴ�
	*/
	char* s = (char*)bupt_malloc(sizeof(char) * 1024 * 1024 * 880); //һ��char��ռһ���ֽ�B��1024B=1KB,string.txt��877MB
	FILE* fpstr = fopen("../../string.txt", "r");
	char* tempchar = s;
	int test = 0;
	while ((*tempchar = (char)fgetc(fpstr)) != EOF) {
		if (*tempchar == '\n') continue;
		tempchar++;
		//test++;
		//if (test>10000000)
		//	break;
	}
	*tempchar = '\0'; //��β�ж�
	fclose(fpstr);
	//��ʱ
	finish_t = clock();
	total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
	start_t = clock();
	printf("��string.txt�����ڴ�ռ�õ���ʱ�䣺%f\n", total_t);

	/*
	��ȡpattern.txt�ļ���������ѭ��
	*/
	FILE* fp = fopen("../../pattern_bf_kmp.txt", "r");
	FILE* fpwrite = fopen("output.txt", "w");

	unsigned int M = 1500;
	//����
	int check = 0;
	//unsigned int M = 10;
	for (int i = 0; i < M; i++) {
		char str[150];
		fgets(str, 150, fp);
		str[strlen(str) - 1] = '\0'; //��\n�ĵ�
		int len = strlen(str);  //strlen�������'\0'

		/*
		��ʼ����
		*/
		int* next = (int*)bupt_malloc(sizeof(int) * len);
		getNext(str, next);
		int count = 0; //ģʽ�����ִ���
		count = kmpStringCountNum(str, len, next, s);



		//д����output
		fprintf(fpwrite, "%s %d\n", str, count);


		//test
	
		//��ʱ 3��һ��
		//finish_t = clock();
		//total_t = (double)(finish_t - start_t) / CLOCKS_PER_SEC;//��ʱ��ת��Ϊ��
		//start_t = finish_t;
		//printf("CPU ռ�õ���ʱ�䣺%f\n", total_t);
		check++;
		if (check % 100 == 0)
			printf("%d patterns are searched.\n", check);
	}

	fclose(fp);
	fclose(fpwrite);

	//��output�ļ���ȡ����������֮�󵼳���result.txt
	fp = fopen("output.txt", "r");
	fpwrite = fopen("result.txt", "w");

	//ʹ�����ȶ���ʵ�֣�ͨ������ʵ�֣����ö�Ϊ���Լ��ˣ�

	//��ȡ��һ��Ԫ��
	char tem[50];
	int temval = 0;
	fscanf(fp, "%s", tem);
	fscanf(fp, "%d", &temval);
	int len = strlen(tem); //��lenû����
	Node* top = newNode(tem, len, temval);

	for (int i = 1; i < M; i++) {
		fscanf(fp, "%s", tem);
		fscanf(fp, "%d", &temval);
		//tem[strlen(tem) - 1] = '\0';
		len = strlen(tem); //��lenû����

		push(&top, tem, len, temval);

	}

	//���д��result.txt
	while (!isEmpty(&top)) {
		//Node* tem = peek(&top);
		printf("%s %d\n", top->data, top->priority);

		fprintf(fpwrite, "%s %d\n", top->data, top->priority);
		pop(&top);
	}

	//д�����һ��

	fprintf(fpwrite, "%lld %d\n", cmpnum/1000, mem / 1024); //���һ��������������ֽڱȽϴ���K �ڴ濪����KB��

	fclose(fp);
	fclose(fpwrite);

	return 0;

}
void getNext(char* str, int* next) {
	int i = 0, j = -1;
	next[0] = -1;
	while (i < (int)strlen(str)) {
		if (j == -1 || str[i] == str[j]) {
			i++;
			j++;
			next[i] = j;
		}
		else
			j = next[j];
	}
}
int kmpStringCountNum(char* str, int len, int* next, char* s) {

	//���ַ�����ȡ���Ƚ�
	char *c = s;
	int count = 0; //ģʽ�����ִ���
	
	int index = 0; //��ǰ���ҵ�str�е���һ��Ԫ��
	//test
	int test = 0;
	while (*c != '\0') {
		cmpnum++;

		if (*c != str[index]) { //����ʧ��
			if (index == 0) { //ֱ������һ������
				c++;
				continue;
			}
			//�ƶ�λ��= ��ƥ����ַ���- ��Ӧ��NEXTֵ
			while (*c != str[index]) { //����ʧ��
				//�ƶ�λ��= ��ƥ����ַ���- ��Ӧ��NEXTֵ
				index = next[index];
				cmpnum++;
				if (index == -1) {//��һ���ַ�Ҳ��ƥ�䣬���ư�
					break;
				}

			}
			index++;

		}
		else {
			index++;
			if (index >= len - 1) { //ģʽ��ƥ�䣬���һ����\0��ֻ��len-1, ���ɣ�������˿��ܵø���
				count++;
				index = 0;
			}
		}
		c++;
		
	}
	return count;
}
//���ȶ���ʵ��
Node* newNode(char* str, int len, int p)
{
	Node* temp = (Node*)malloc(sizeof(Node));
	//memset(temp, 0, sizeof(Node)); //��ʼ�������У����dataָ���NULL
	temp->data = (char*)malloc(sizeof(char) * len + 1); //ָ������ǵ÷����ڴ�
	//*(temp->data) = *str; �⸳ֵ����ͷ�ļ�
	for (int i = 0; i < len; i++) {
		*(temp->data + i) = *(str + i);
	}
	*(temp->data + len) = '\0'; //strlen�������'\0'�����Լ���
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
	Node* temp = newNode(str, len, p);

	// Special Case: The head of list has lesser 
	// priority than new node. So insert new 
	// node before head node and change head node. 
	if ((*head)->priority < p) { //����Ϊpriority�����ͷ

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