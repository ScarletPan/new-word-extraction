[TOC]

## 1. 通用新词发现模块


## 2. 

```text
 - New words Discovery Module options
Usage:
  ./cpp [OPTION...]

  -m, --map_type arg       Mapping type, 'hash' or 'trie' (default: hash)
  -g, --max_gram arg       Max gram length (default: 4)
  -c, --min_count arg      Minimum count (default: 5)
  -s, --base_solidity arg  Minimum solidity of unigram (default: 5.0)
  -e, --min_entropy arg    Minimum entropy (default: 2.0)
      --sort               Sort the results
      --help               Print help
```





## 3. 安装

将项目下载到本地，进入项目目录，并输入

```bash
$ python setup.py install
```



## 4. 使用样例


## 5. 教程和文档


## 6. 新词发现工具


## 7. Yard分布式部署

## 8. 测试结果

1. **运行时间：**

|           | 《人民的名义》(778K) | 《天龙八部》(3.5M) | 《冰与火之歌全集》(8.9M) | 2.8w公众号文章(114M) |
| --------- | -------------------- | ------------------ | ------------------------ | -------------------- |
| python版  | 7.83s                | 20.00s             | 49.36s                   | 814.77s (13.6 min)   |
| C++复现版 | 3.20s                | 12.03 s            | 31.40s                   | 437.62s (7.29 min)   |

2. **运行时最大占用内存：**

|           | 《人民的名义》(778K) | 《天龙八部》(3.5M) | 《冰与火之歌全集》(8.9M) | 2.8w公众号文章(114M) |
| --------- | -------------------- | ------------------ | ------------------------ | -------------------- |
| python版  | 353 MB               | 1115 MB            | 2602 MB                  | 22344 MB (21.8GB)    |
| C++复现版 | 85 MB                | 312 MB             | 730 MB                   | 7267 MB (7.1 GB)     |

