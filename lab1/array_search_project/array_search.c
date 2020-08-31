#include<stdio.h>
#include<string.h>
#include<malloc.h>

int mem = 0;
int cmpnum = 0;
//格式转成 GB2312
//内存分配可以看出一个二维的情形，每一个string使用malloc(size_of(s.length))，而添加新元素时使用relloc分配

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

int main()
{
	unsigned int N = 1270688; //血与泪的教训
	FILE *fp = fopen("patterns-127w.txt", "r");

	//注意，使用二维指针char **，指针指向的是一个指针，再给指针的指针开辟空间放字符串
	char **p=NULL; //地址的地址
	p = (char **)malloc(N * sizeof(char*)); //开二维指针，这样p+1直接就都下一个字符串  
	//printf("%s\n", p[0]); //所以用printf（）输出时候，只要你打印格式是%s，系统知道我们要输出字符串，而且系统也可以确定字符串的长度，所以当你用%s打印字符指针p的时候就会输出字符串

	//开始读取数据
	int *strslen1 = (int *)malloc(N * sizeof(int));
	for (int i = 0; i < N;i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		strslen1[i] = len; //用于记录各字符串长度
		//追加内存空间
		p[i] = (char *)malloc(len * sizeof(char));
		//printf("%d\n", sizeof(tem));
		strcpy(p[i], tem);
		bupt_malloc(sizeof(p[i]));
		

		//if (i == 1000)
		//	break;
	}

	fclose(fp);

	//读取比对用数据
	unsigned int M = 986004;
	//int strslen[M];
	int *strslen2 = (int *)malloc(M * sizeof(int));
	fp = fopen("words-98w.txt", "r");
	char **q = NULL;
	q = (char **)malloc(M * sizeof(char*));
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		strslen2[i] = len; //用于记录各字符串长度
		//追加内存空间
		q[i] = (char *)malloc(len * sizeof(char));
		strcpy(q[i], tem);
		bupt_malloc(sizeof(q[i]));

		//char *p1 = (char *)malloc(sizeof(tem));
		//if (i == 1000)
		//	break;
	}
	fclose(fp);
	//for (int i = 0; i < 1000; i++)
	//	printf("%s\n", q[i]);

	//printf("%c\n", q[1][1]); //是个问号，中文编码不一样
	//
	FILE *fpwrite = fopen("result.txt", "w");

	int memsize = (mem * 8) / 1024;//sizeof()以字节为单位：1byte=8bit

	

	//开始比较
	//维护一个string数组来存放结果
	//char *results = (char *)malloc(M * sizeof(char));
	int yescount = 0;
	for (int i = 0; i < M; i++) {

		int flag = 1;
		int border = strslen2[i];
		for (int j = 0; j < N; j++) {
			//开始比对两个字符串
			int judge = 0;
			int k = 0;
			//printf("%d\n", strslen1[j]);
			int plen = strslen1[j];
			if (plen !=  border) //不一样长，肯定错
				continue;
			for (int k = 0; k < border; k++) {
				int a = byte_cmp(q[i][k], p[j][k]);
				if (a != 0) { //比对失败
					judge = 1;
					break;
				}
			}
			if (judge == 0) { //比对成功
				flag = 0;
				break;
			}
		}
		if (flag == 0) { //比对成功
			//fprintf(fpwrite, "%s", "yes\n");
			fprintf(fpwrite, "%s %s", q[i], "yes\n");
			yescount++;
		}
		else {
			//fprintf(fpwrite, "%s", "no\n");
			fprintf(fpwrite, "%s %s", q[i], "no\n");
		}

		if (i % 10000 == 0)
			printf("checking on num %d\n", i);
	}
	//写入数据
	fprintf(fpwrite, "%d %d %d %d", memsize, cmpnum, M, yescount);
	fclose(fpwrite);
	//fclose(fp);
	//释放内存
	free(p);
	free(q);
	free(strslen1);
	free(strslen2);
	return 0;
}