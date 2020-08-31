该项目为2019年北京邮电大学秋季学期研究生《计算机应用编程实验》课程的项目代码，全部使用C语言实现，仅作交流使用。

课程总共分为4个部分：

1. 海量字符串检索。分为：数组、hash表、布隆过滤器三个实现版本。词典串文件pattern.txt：127万个字符串，待匹配的字符串文件words.txt：98万个字符串。
2. 树结构海量字符串检索：分别使用二叉搜索树、B+树、Radix Trie（基数树）实现。在实现过程中要注意的是模式串的规模较大，需要构造一个高效数据结构来处理。
3. 多模式字符串匹配：给定测试大规模中文文本串，数据量约800M，待查找的key大约220万个词作为模式串（文件过大不放在本项目中了）， 需要从大文本串中查找出所有模式串出现的位置和次数。共分为：朴素查找实现（暴力解决，程序运行时间数小时）、Multikmp实现、ac自动机匹配实现（结合了Trie树与KMP算法，可以在一次遍历内完成所有模式串的匹配，效率比前两个高很多）。
4. 网站页面分析器：爬取校园网内服务器上十几万个页面的网站，构建页面链接网络，计算pagerank最高的前10个页面。页面爬取使用winsock创建socket获取网页，对返回的页面内容使用pcre2库进行正则怕匹配提取URL，能力有限本代码仅使用单线程。PageRank部分要注意矩阵的阶很大，无法直接计算其特征值和特征向量，需要使用Power Iteration幂迭代方法。

**注意**

各lab内根目录下存有程序运行所需的数据集，将其拷贝到各C代码同一目录下以便调试与运行。