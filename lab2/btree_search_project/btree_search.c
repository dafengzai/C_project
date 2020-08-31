#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<math.h>
#include <stdbool.h> //c99没bool

//分别定义平衡因子数
#define LH +1
#define EH  0
#define RH -1

int mem = 0;
int nodenum = 0;
int cmpnum = 0;

void* bupt_malloc(size_t size) {  //size_t:used to represent the size of an object
	if (size <= 0)
		return NULL;
	mem += size;
	nodenum++;
	return malloc(size);
}
int byte_cmp(char a, char b)
{
	cmpnum++;
	return a - b;
}
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

//平衡二叉树的结构定义需要：数据成员，平衡因子，以及左右分支
typedef struct btnode {
	unsigned int val; //将字符串转发为数字后再比较
	int bf; //平衡因子balance flag
	struct btnode* left, * right;
}btnode, *btree;
int InsertAVL(btree* root, unsigned int value, bool* taller);
bool FindNode(btree root, unsigned int value);

void LeftBalance(btree* root);
void RightBalance(btree* root);
//平衡二叉树的失衡调整主要是通过旋转最小失衡子树来实现的。根据旋转的方向有两种处理方式:左旋与右旋 。[http://data.biancheng.net/view/59.html]
void L_Rotate(btree* root);//1.节点的右孩子替代此节点位置 2.右孩子的左子树变为该节点的右子树,3.节点本身变为右孩子的左子树
void R_Rotate(btree* root);//1.节点的左孩子代表此节点,2.节点的左孩子的右子树变为节点的左子树,3.将此节点作为左孩子节点的右子树。

int main()
{
	unsigned int N = 1270688; //血与泪的教训
	unsigned int M = 986004;
	
	FILE* fp = fopen("patterns-127w.txt", "r");

	//struct btnode* root = NULL;
	btree root = NULL; 
	//开始读取数据
	for (int i = 0; i < N; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//将string转化为数字，使用c99自带函数strtol没吊用，还是得硬来
		int len = strlen(tem);
		int result = BKDRHash(tem, len); //先转数字，但由于位数较大120+个汉字，下面的招没法用，还是用hash吧
		//for (int j = 0; j < len; j++) {
		//	result = result * 10 + (tem[j] - '0');
		//}
		//建立二叉平衡树
		bool taller;
		InsertAVL(&root, result, &taller); //传入地址的地址，以此让地址内的节点元素改变（地址在函数内不会变，但地址内元素可以改变赋值成节点）
		/*
		说明：在插入新的结点的时候，我们使用一个taller的变量来记录树的高度是否变化。默认认为树的高度是有增加的。
		我们在插入新的结点后，首先判断树的高度是否增加了，假如树的高度没有变化，不必进行如何操作。
		当树的高度增加时，我们就考虑是否需要对树的进行平衡调整。
		*/
		//if (i == 1000)
		//	break;
	}
	fclose(fp);

	//开始匹配
	fp = fopen("words-98w.txt", "r");
	FILE* fpwrite = fopen("result.txt", "w");
	int yescount = 0;
	for (int i = 0; i < M; i++) {
		char tem[150];
		fgets(tem, 150, fp);
		tem[strlen(tem) - 1] = '\0';
		//将string转化为数字，使用c99自带函数strtol没吊用，还是得硬来
		int len = strlen(tem);
		unsigned int result = BKDRHash(tem, len);
		//for (int j = 0; j < len; j++) {
		//	result = result * 10 + (tem[j] - '0');
		//}

		//查找平衡二叉树来进行匹配
		if(FindNode(root, result)){ //匹配成功
			fprintf(fpwrite, "%s %s", tem, "yes\n");
			yescount++;
		}
		else {
			fprintf(fpwrite, "%s %s", tem, "no\n");
		}
	}

	//写入最后一行数据
	int memsize = mem / 1024; //sizeof()以字节为单位，本题单位为KB
	cmpnum = cmpnum / 1000;
	fprintf(fpwrite, "%d %d %d %d %d", nodenum, memsize, cmpnum, M, yescount);
	fclose(fpwrite);

	fclose(fp);
	fclose(fpwrite);
	
	return 0;

}

int InsertAVL(btree* root,unsigned int value, bool* taller) {
	if (*root == NULL) {  //第一个节点赋值了，所以要先定义好
		//printf("hehe\n");
		*root = (btree)malloc(sizeof(btnode));
		bupt_malloc(sizeof(btnode));
		(*root)->left = NULL;
		(*root)->right = NULL;
		(*root)->val = value;
		(*root)->bf = EH;
		*taller = true;
	}
	//printf("heihei\n");
	else if (value == (*root)->val) { //本例字符串都不一样，用不到这个的
		*taller = false;
		return 0;
	}
	//如果value小于root内元素，插入左子树中
	else if (value < (*root)->val) {
		int flag = InsertAVL(&(*root)->left, value, taller);
		if (flag == 0) //插入不会影响树的平衡
			return 0;
		//判断插入过程是否会导致树深度+1
		if (*taller) {
			/*
			树高度增加后来判断根节点的平衡因子，由于是在其左子树添加新结点的过程中导致失去平衡，
			所以当根结点的原平衡因子本身为 1 时（加入新节点后+1=2了），需要进行左子树的平衡处理，否则更新树中各结点的平衡因子数
			*/
			switch ((*root)->bf)
			{
			case LH:
				LeftBalance(root);
				*taller = false; //左平衡后父节点无需改动了
				break;
			case EH:
				(*root)->bf = LH;
				*taller = true; //原本为EH，对该节点无影响，保持为true让父节点继续判断
				break;
			case RH:
				(*root)->bf = EH;
				*taller = false; //添加后节点平衡了，无需让父节点继续判断
				break;
			}
		}
	}
	else if (value > (*root)->val) { //同理
		int flag = InsertAVL(&(*root)->right, value, taller);
		if (flag == 0) //节点插入为改变树的平衡
			return 0;
		if (*taller) {
			switch ((*root)->bf)
			{
			case LH:
				(*root)->bf = EH;
				*taller = false; //左平衡后树高会恢复
				break;
			case EH:
				(*root)->bf = RH;
				*taller = true;
				break;
			case RH:
				RightBalance(root);
				*taller = false; //加入的节点不激活平衡函数，taller赋false让其继续判断
				break;
			}
		}
	}
	return 1; //插入节点默认是会对平衡产生影响的，交由父节点判断
}
void LeftBalance(btree* root) {
	btree lc, lrc;
	lc = (*root)->left;
	//查看以 T 的左子树为根结点的子树，失去平衡的原因，如果 bf 值为 1 ，则说明添加在左子树为根结点的左子树中，需要对其进行右旋处理；
	//反之，如果 bf 值为 -1，说明添加在以左子树为根结点的右子树中，需要进行双向先左旋后右旋的处理
	switch (lc->bf)
	{
	//既然root不平衡了（+2），其左子树肯定bf便不可能为0
	case LH:
		(*root)->bf = lc->bf = EH;
		R_Rotate(root);
		break;
	case RH: //root的左孩子的右子树插入节点，使用先左后右旋转
		lrc = lc->right;
		switch (lrc->bf) //更新节点bf,看下[http://data.biancheng.net/view/59.html]的图7
		{
		case LH:
			(*root)->bf = RH; lc->bf = EH; break;
		case RH:
			(*root)->bf = EH; lc->bf = LH; break;
		case EH:
			(*root)->bf = EH; lc->bf = EH; break;
		}
		lrc->bf = EH;
		L_Rotate(&(*root)->left);
		R_Rotate(root);
		break;
	}
}
void RightBalance(btree* root) {
	btree rc, rlc;
	rc = (*root)->right;
	//查看以 T 的右子树为根结点的子树，失去平衡的原因，如果 bf 值为 -1 ，需要对其进行左旋处理；
	//反之，如果 bf 值为 1（右左），说明添加在以右子树为根结点的左子树中，需要进行双向先右旋后左旋的处理
	switch (rc->bf)
	{
		//既然root不平衡了（+2），其左子树肯定bf便不可能为0
	case RH:
		(*root)->bf = rc->bf = EH;
		L_Rotate(root);
		break;
	case LH: //root的左孩子的右子树插入节点，使用先左后右旋转
		rlc = rc->left;
		switch (rlc->bf) //更新节点bf,看下[http://data.biancheng.net/view/59.html]的图8
		{
		case LH:
			(*root)->bf = EH; rc->bf = RH; break;
		case RH:
			(*root)->bf = LH; rc->bf = EH; break;
		case EH:
			(*root)->bf = EH; rc->bf = EH; break;
		}
		rlc->bf = EH;
		R_Rotate(&(*root)->right); //传地址的地址来更改内容
		
		L_Rotate(root);
		break;
	}
}
void L_Rotate(btree* root) {
	//1.节点的右孩子替代此节点位置 2.右孩子的左子树变为该节点的右子树,3.节点本身变为右孩子的左子树
	btree rc = (*root)->right;
	(*root)->right = rc->left;
	rc->left = *root;
	*root = rc;
}
void R_Rotate(btree* root) {
	//1.节点的左孩子代表此节点,2.节点的左孩子的右子树变为节点的左子树,2.将此节点作为左孩子节点的右子树。
	btree lc = (*root)->left;
	(*root)->left = lc->right;
	lc->right = *root;
	*root = lc;
}
bool FindNode(btree root, unsigned int value) {
	//本质上就是二叉树的查找
	btree tem = root;
	char a = 'a', b = 'b';
	while (tem) {
		int c = byte_cmp(a, b); //计算比较次数用，懒得改了
		if (value == tem->val) { //查找成功
			return true;
		}
		else if (value < tem->val) {
			tem = tem->left;
		}
		else {
			tem = tem->right;
		}
	}
	return false; //二叉树查找的循环内没找到
}
