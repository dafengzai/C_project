#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>

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

unsigned int MurmurHash2(const void * key, int len, unsigned int seed) //能用，没毛病
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//进行比特操作用的函数
void SetArrayBit(char *bf, unsigned int loc) {
	char tem = 1 << (7 - loc % 8);
	bf[loc / 8] = bf[loc / 8] | tem; //或操作
}
int GetArrayBit(char *bf, unsigned int loc) {  //返回1为匹配成功
	char tem = bf[loc / 8];
	int a= (tem & 1 << (7 - loc % 8)) >> (7 - loc % 8); //先用1左移再与bf内元素进行与操作（1左移后其他位为0与操作不影响，再右移回去末尾为1就返回1，末尾为0返回0
	//printf("%d\n", a);
	return a;
}

int main()
{
	unsigned int N = 1270688; //血与泪的教训
	unsigned int M = 986004;
	int k = 0; //hash函数的个数
	long int m = 0;//需开辟的存储空间个数
	//关注的是false positive rate f:一个不在集合N中的字符经k次hash后对应位皆为1的概率
	//经推算后当q:数组中某一位为0的概率为0.5，即数组m中一半元素为空时f最小
	//具体公式为：给定f后,其余皆可计算出
	
	double f = 0.000001; //设定错误率
	m = N*1.44*(log(1/f) / log(2));  //m=3.6kw
	k = 0.693*(m / N); //k=19
	//printf("%d\n", m);
	//printf("%d\n", k);

	//建立bf用的数组，用char来表示一个8bit来判断
	int size = m / 8;
	//int size = m; //其实m没超过INT_MAX
	char *bfarray = (char *)malloc((size) * sizeof(char)); //char占一个字节，int占4个
	mem = (size) * sizeof(char);
	//初始化bfarray
	for (int i = 0; i < size; i++) {
		bfarray[i] = '0';
	}
	
	//建立k个murmur hash函数用的seed
	int *seed = (int *)malloc(k * sizeof(int));
	for (int i = 0; i < k; i++)
	{
		//seed[i] = 66 * (i + 1);
		//seed[i] = pow(13, i + 1) % 10000;
		seed[i] = (13 ^ (i + 1)) % 10000;
	}

	FILE *fp = fopen("patterns-127w.txt", "r");
	//将文件内字符串经hash后翻入bfarray中


	
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		int len = strlen(tem);
		for (int j = 0; j < k; j++) {
			//读取字符串，先使用murmur哈希函数将其转化成一个整数
			unsigned int murmurindex = MurmurHash2(tem, len, seed[j]);
			murmurindex = murmurindex%m;
			SetArrayBit(bfarray, murmurindex);
			//bfarray[murmurindex] = '1';
		}
	}
	fclose(fp);
	unsigned int memsize = (mem * 8) / 1024;//sizeof()以字节为单位：1byte=8bit
	
	//开始匹配
	fp = fopen("words-98w.txt", "r");
	FILE *fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	char a = 'a', b = 'b';
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';;
		int len = strlen(tem);
		//开始进行bloom filter
		int flag = 1;//默认成功
		for (int j = 0; j < k; j++) {
			//读取字符串，先使用murmur哈希函数将其转化成一个整数
			unsigned int murmurindex = MurmurHash2(tem, len, seed[j]);
			murmurindex = murmurindex%m;
			//判断
			int c = byte_cmp(a, b); //计算比较次数用，懒得改了
			//printf("%d\n", cmpnum);
			int result = GetArrayBit(bfarray, murmurindex); //返回1表示匹配成功
			//char result = bfarray[murmurindex];
			if (result != 1) { //匹配失败
				flag = 0;
				break;
			}
		}
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
	//free(bfarray); 有bug，我也不知道为啥
	//free(seed);
	return 0;

}