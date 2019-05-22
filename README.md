[TOC]

## 1. 新词发现模块


## 2. 安装

### 2.1 C++ 快速版

```bash
$ git clone http://git.code.oa.com/alonsoli/new-word-extraction.git
$ cd new-word-extraction
$ cd new-word-extraction/cpp
$ make
```

### 2.2 Python版

```bash
$ git clone http://git.code.oa.com/alonsoli/new-word-extraction.git
$ cd new-word-extraction/python
$ python setup.py install
```



## 3. 快速入门

### 3.1 C++快速版

1. **检索候选词** `retrieve`

```bash
$ ./cpp/fastnewwords --mode retrieve < ../data/books/renmingdemingyi.txt > tmp.txt
```

输入文件格式为含中文的 `utf8` 文本，输出格式为 [word] [count] [score1] [score2]\n 如下例所示：

```
侯亮平 1358 485.426 2.87977 
李达康 781 797.963 3.31298 
书记 630 833.582 2.44989 
高育良 628 477.54 2.73268 
祁同伟 553 1224.79 3.10702 
```

2. **重排序** `rerank`

```bash
$ ./cpp/fastnewwords --mode rerank --dict dict.txt --stopwords stopwwords.txt < tmp.txt > tmp.txt
```

输入/输出文件格式 与 上一步的输出文件格式相同

### 3.2 Python版

```bash
$ python demo/newwords.py < ../data/books/renmingdemingyi.txt > tmp.txt
```



## 3. 命令行参数

### 3.1 C++快速版

```
$ ./fastnewwords --help

* New words Discovery Module options
 Usage:
   --topk arg          Return only topk after reranking (default: 20)
   --min_solidity arg  Minimum solidity of unigram (default: 5.0)
   --stopwords arg     Stopwords path
   --dict arg          Exsiting dict path
   --min_count arg     Minimum count (default: 5)
   --map_type arg      Mapping type, 'hash' or 'trie' (default: 'hash')
   --max_gram arg      Max gram length (default: 4)
   --noscores arg      output words without scores
   --min_entropy arg   Minimum entropy (default: 2.0)
   --mode arg          program mode, 'retrieve' or 'rerank' (default: 'retrieve')
   -h, --help          Print help
```



### 3.2 Python版

```
$ python demo/newwords.py --help

usage: newwords.py [-h] [-g MAX_GRAM] [-c MIN_COUNT] [-s BASE_SOLIDITY]
                   [-e MIN_ENTROPY]

optional arguments:
  -h, --help            show this help message and exit
  -g MAX_GRAM, --max_gram MAX_GRAM
                        Max Length of gram
  -c MIN_COUNT, --min_count MIN_COUNT
                        Minimum occurrence of the gram
  -s BASE_SOLIDITY, --base_solidity BASE_SOLIDITY
                        Minimum solidity of unigram
  -e MIN_ENTROPY, --min_entropy MIN_ENTROPY
                        Minimum entropy
```




## 4. Yard分布式部署

## 5. 测试结果

### 5.3 功能测试

* 《人民的名义》

```bash
$ ./fastnewwords --mode retrieve < ../data/books/renmingdemingyi.txt > tmp.txt
$ ./fastnewwords --mode rerank --min_solidity 5 --topk 20 --noscores --dict ../data/dict/dict.txt --stopwords ../data/dict/stopwords.txt < tmp.txt > tmp2.txt

TOP 20 Frequent: 
侯亮平 李达康 高育良 祁同伟 蔡成功 沙瑞金 高小琴 陈海 丁义珍 陆亦可 欧阳菁 季昌明 京州 也不 陈岩石 刘新建 郑西坡 赵瑞龙 赵东来 大风厂
```

* 《天龙八部》

```bash
$ ./fastnewwords --mode retrieve < ../data/books/tianlongbabu_jinyong.txt > tmp.txt
$ ./fastnewwords --mode rerank --min_solidity 15 --topk 20 --noscores --dict ../data/dict/dict.txt  --stopwords ../data/dict/stopwords.txt < tmp.txt > tmp2.txt

TOP 20 Frequent: 
段誉 萧峰 阿紫 乔峰 阿朱 童姥 钟灵 保定帝 阿碧 玄难 玄慈 薛神医 群豪 师叔 谭婆 辽国 吓得 青袍客 谭公 师伯
```

* 《冰与火之歌》

```bash
$ ./fastnewwords < ../data/books/bingyuhuo.txt > tmp.txt
$ ./fastnewwords --mode rerank --min_solidity 15 --topk 20 --noscores -D ../data/dict/dict.txt --stopwords ../data/dict/stopwords.txt < tmp.txt > tmp2.txt

TOP 20 Frequent: 
琼恩 丹妮 詹姆 珊莎 瑟曦 凯特琳 劳勃 罗柏 席恩 艾德 诸神 阿多 蓝礼 佛雷 莫尔蒙 托曼 冰原狼 伊蒙 莱莎 弥林 
```



### 5.2 性能测试

1. **运行时间：**

|                 | 《人民的名义》(778K) | 《天龙八部》(3.5M) | 《冰与火之歌全集》(8.9M) | 2.8w公众号文章(114M) |
| --------------- | -------------------- | ------------------ | ------------------------ | -------------------- |
| python版        | 7.83s                | 20.00s             | 49.36s                   | 814.77s (13.6 min)   |
| C++复现版       | 3.20s                | 12.03 s            | 31.40s                   | 437.62s (7.29 min)   |
| C++快速版(hash) | 1.86s                | 7.82s              | 18.46s                   | 214.20s (3.57 min)   |
| C++快速版(trie) | 2.89s                | 11.64s             | 29.09s                   | 375.18s  (6.25 min)  |

2. **运行时最大占用内存：**

|                 | 《人民的名义》(778K) | 《天龙八部》(3.5M) | 《冰与火之歌全集》(8.9M) | 2.8w公众号文章(114M) |
| --------------- | -------------------- | ------------------ | ------------------------ | -------------------- |
| python版        | 353 MB               | 1115 MB            | 2602 MB                  | 22344 MB (21.8GB)    |
| C++复现版       | 85 MB                | 312 MB             | 730 MB                   | 7267 MB (7.1 GB)     |
| C++快速版(hash) | 46 MB                | 162 MB             | 386 MB                   | 3307 MB (3.2 GB)     |
| C++快速版(trie) | 31 MB                | 105 MB             | 261 MB                   | 2299 MB (2.25 GB)    |

