#include<stdio.h>
#include<string.h>
#include<malloc.h>

int mem = 0;
int cmpnum = 0;

void *bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	return malloc(size);
}

int byte_cmp(char a, char b)
{
	cmpnum++;
	return a - b;
}

struct linklist {
	unsigned int val;
	struct linklist *next;
};

unsigned int BKDRHash(const char* str, int length) //网上找的BKDR哈希函数
{
	unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < length; ++str, ++i)
	{
		hash = (hash * seed) + (*str);
	}

	return hash;
}

void creatchain(struct linklist *chain, int len) //初始化拉链
{
	for (int i = 0; i < len; i++) {
		chain[i].val = i;
		chain[i].next = NULL;
		bupt_malloc(sizeof(chain[i]));
	}
}

void addchain(struct linklist *chain, int loc, unsigned int hashnum) //往拉链中插入元素
{
	struct linklist *p = chain+loc;
	while (p->next != NULL) { //遍历到链表尾
		p = p->next;
	}
	struct linklist *q= (struct linklist*)malloc(sizeof(struct linklist)); //必须开个内存空间，防止函数结束后收回内存空间
	q->val = hashnum;
	q->next = NULL;
	bupt_malloc(sizeof(*q));
	p->next = q;
}

int findchain(struct linklist *chain, int loc, unsigned int hashnum) //查找hashnum是否在拉链中
{
	struct linklist *p = (chain + loc)->next; //p赋为第一个元素
	int flag = 0;//默认匹配失败
	char a = 'a', b = 'b';
	while (p != NULL) {
		int c = byte_cmp(a, b);
		if (p->val == hashnum) {
			flag = 1;
			break;
		}
		p = p->next;
	}
	return flag;
}

int main()
{
	//bkdr
	unsigned int N = 1270688; //血与泪的教训
	unsigned int M = 986004;
	FILE *fp = fopen("patterns-127w.txt", "r");
	int primenum = 122777;
	//采用模一个大质数的方式使用拉链法
	struct linklist *ChainHash = (struct linklist*)malloc(primenum * sizeof(struct linklist));
	//初始化拉链，默认的struct内.next不是NULL
	creatchain(ChainHash, primenum);

	
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		//读取字符串，先使用bkdr哈希函数将其转化成一个整数
		unsigned int bkdrhash = BKDRHash(tem, len);
		int loc = bkdrhash%primenum; //loc范围为：0-primenum-1
		//使用拉链法
		addchain(ChainHash, loc, bkdrhash);
		//if (i % 10000==0)
			//printf("checking on num %d\n", i);
	}
	fclose(fp);
	int memsize = (mem * 8) / 1024;//sizeof()以字节为单位：1byte=8bit
	
	//开始匹配
	fp = fopen("words-98w.txt", "r");
	FILE *fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';;
		int len = strlen(tem);
		//读取字符串，先使用bkdr哈希函数将其转化成一个整数
		unsigned int bkdrhash = BKDRHash(tem, len);
		int loc = bkdrhash%primenum; //loc范围为：0-primenum-1
		//使用拉链法来进行匹配
		int flag = findchain(ChainHash, loc, bkdrhash); //1为匹配成功，0表示匹配失败
		if (flag == 1) { //匹配成功
			fprintf(fpwrite, "%s %s", tem, "yes\n");
			yescount++;
		}
		else {
			fprintf(fpwrite, "%s %s", tem, "no\n");
		}
	}

	//写入最后一行数据

	fprintf(fpwrite, "%d %d %d %d", memsize, cmpnum, M, yescount);
	fclose(fpwrite);

	fclose(fp);
	fclose(fpwrite);

	
	//释放内存（该例内其实不free也行，程序结束后会自动把分配的内存回收。
	free(ChainHash);
	return 0;
}