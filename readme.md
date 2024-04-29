# 语法分析器

这个简单的语法分析器是用 C++ 编写的，用于分析给定的输入句子是否符合指定的文法规则。它使用了预测分析法，并提供了一些用于构建预测分析表和分析输入的函数。

## 文件结构

- `code`：存放各个模块编写测试的代码（有一部分是合并前未修改的，但分块便于查看）
- `outfile`：存放程序的文件
  - `out.txt`：存放输入文法，格式`S -> E `，用空格+->分割的输入文法信息（不要|，这样输入后处理简便）。
  - `out1.txt`：单纯是将测试过的内容复制过来（一次一次重新输入太麻烦了）。
  - `productions.txt`：存放初步处理后的文法，包括提取左因子，消除间接直接递归。
  - `analysis_process.dot` 和 `analysis_process.png`：分析过程的可视化结果，其中 `.dot` 文件是 Graphviz 格式的源文件，`.png` 文件是根据 `.dot` 文件生成的图片文件。
  - `analysis_set.dot` 和 `analysis_set.png`：分析集合的可视化结果，包括first，follow和select，格式与上面类似。
  - `dot_table.dot` 和 `dot_table.png`：预测分析表的可视化结果，格式与上面类似。
  - `predictive_analysis_table.csv`：预测分析表的 CSV 格式文件，包含了预测分析表的内容。
- `main.cpp`：包含语法分析器的主要实现代码。
- `main.exe`：编译后生成的可执行文件。
- `readme.md`：项目的说明文档，提供了项目的介绍、用法、示例等信息。

## 功能实现

- 提取左因子（`extractLeftFactor`函数）

  - 遍历文法，消除形如S->ab|ac的文法，根本思路是检查产生式，将相同的第一个字母提取出来作为左因子

  - eg. S->a|adb|adc，先提取a，解决a|aab|aac的左因子a，再提取db|dc的左因子d，若遇到aac|aa，则两次提取a

- 消除左递归（`eliminateLeftRecursion`函数）

  - 支持直接和间接递归，简介思路就是依次替换可以被替换的非终结符，然后进行直接递归处理，但有些问题，导致间接递归的处理有很大的重复项，需要进一步完善

  - 直接递归问题不大，是将类似T->Ta|b，这样的处理成T->T’b，T’->aT'，就是课上的思路（个人觉得其实用其他字母替换最好，不然后面计算各个集合总要考虑文法非终结符包不包含`'`这个东西，很麻烦）

- 计算First集合（`calculateFirst`函数）

  - 就是遍历寻找能推出来的第一个非终结符以及`ε`

- 计算Follow集合（`calculateFollowSets`函数）

  - 遍历文法，找到非终结符后，根据所在位置判断具体操作
    - 后方是终结符，放入follow
    - 后方是非终结符，其first放入该终结符follow
    - 后方能推出`ε`或者该非终结符已经是最后一位，则将文法产生式左侧follow集合放入该终结符follow集合

- 计算SELECT集合（`calculateSelectSets`函数）

  - 实际上就是填写预测分析表时的终结符与非终结符交点填写什么产生式比较合适
  - 把文法产生式与终结符联系起来，实现就是找各个非终结符对应的first集合，若存在`ε`，就把follow也放进去。
  - 结构就是 非终结符 -> 产生式 ：对应终结符

- `buildPredictiveTable` 函数

  - 该函数用于构建预测分析表。它遍历文法中的每个非终结符及其对应的产生式，然后根据文法的SELECT集合来填充预测分析表。

  - 如果发现文法有歧义，会输出错误信息并返回空表。

- `printPredictiveTable` 函数

  - 该函数用于打印预测分析表到一个CSV文件中。它将非终结符作为行标签，将终结符作为列标签，然后将对应的产生式填入表格中。（单纯是终端输出容易乱）

- `writePredictiveTableDot` 函数

  - 该函数用于将预测分析表以DOT格式写入文件，以便可视化显示。它使用Graphviz的DOT语言表示预测分析表，将非终结符和终结符作为节点，将产生式填入表格中。

- `writeDotSet` 函数

  - 该函数用于将FIRST集、FOLLOW集和SELECT集以DOT格式写入文件，以便可视化显示。它将这些集合作为节点，以及一些附加信息如终结符和非终结符的数量。

  - 很神奇的问题是，这个dot生成的图片是横着的，只能randisk让右转90度，所以输出的结果和dot结果正好是反着的（文字没反，只是应该first在最上面变成了select在最上面，所以调整了dot写入顺序让他好看一点）

- `analyze` 函数

  - 该函数用于对输入的句子进行语法分析，并输出分析过程。它使用预测分析表来进行分析，同时结合了栈的操作和输入的处理，逐步进行推导，并输出每一步的动作和栈的状态。
  - 就是分析栈顶和当前input
    - 栈顶是非终结符
      - 查找栈顶与字符串当前位置的终结符交点文法是什么
        - 空，输出交点为空，分析失败
        - 有文法，pop栈顶，push产生式进去
    - 栈顶是终结符
      - 字符串和栈顶一致，输出match，然后pop栈顶，字符串位置往后一位
      - 否则直接退出，fail匹配失败
    - 字符串结束，栈不空
      - 查看是不是能推出`ε`，然后依次替换
      - 最后判断栈顶是不是`$`，是accept，否fail

- `analyzedot` 函数

  该函数与 `analyze` 函数类似，但它将语法分析过程以DOT格式写入文件，以便可视化显示。

- `main`函数

  1. 从文件中读取文法规则，并对其进行处理，先提取左因后消除左递归，这样处理间接左递归的时候方便点。
  2. 将处理后的文法规则输出到文件中，并重新读取以获取右部信息。
  3. 计算文法规则的FIRST集、FOLLOW集和SELECT集。
  4. 构建预测分析表。
  5. 打印文法规则、FIRST集、FOLLOW集、SELECT集和预测分析表。
  6. 从用户输入中获取待分析的句子，并进行语法分析。
  7. 将语法分析过程以及相关集合和表格输出为DOT文件，并将DOT文件转换为PNG图片进行可视化显示。

  8. 通过调用系统命令使用Graphviz的dot命令将生成的DOT文件转换为PNG图片，以便用户更直观地查看分析结果。

## 用法

1. 编译源代码并生成可执行文件 `main.exe`。 
2. 准备输入文法规则，格式为`S -> E`，保存在 `outfile/out.txt` 文件中。
3. 运行 `main.exe`，程序会读取文法规则并进行处理，然后计算集合和构建预测分析表。
4. 输入待分析的句子，程序会进行语法分析并输出分析过程和结果。 
5. 查看生成的可视化结果，包括分析过程、分析集合和预测分析表。

## 实例

`out.txt`

```
S -> a
S -> b
S -> (T)
T -> S
T -> T:S
```

预期结果

`productions.txt`

```S -> (T)
S -> a
S -> b
T -> ST'
T' -> :ST'
T' -> ε
```

```
Original Grammar:
S -> (T) | a | b | 
T -> S | T:S | 

After extracting left factors:
S -> (T) | a | b | 
T -> S | T:S | 

After eliminating left recursion:
S -> (T) | a | b |
T -> ST' |
T' -> :ST' | ε |
Productions have been written to productions.txt
Terminals are $ ( ) : a b
Non-terminals are  S T T'

First sets:
First(S) : ( a b
First(T) : ( a b
First(T') : : ε

Follow sets are:
Follow(S): $ ) :
Follow(T): )
Follow(T'): )

Select sets are:
S -> (T): (
S -> a: a
S -> b: b
T -> ST': ( a b
T' -> :ST': :
T' -> ε: )
```

![image-20240429231133904](https://mine-picgo.oss-cn-beijing.aliyuncs.com/imgtest/image-20240429231133904.png)

![image-20240429231149960](https://mine-picgo.oss-cn-beijing.aliyuncs.com/imgtest/image-20240429231149960.png)



输入字符串

```
(a:(b))
```

![image-20240429231313549](https://mine-picgo.oss-cn-beijing.aliyuncs.com/imgtest/image-20240429231313549.png)



## 注意事项

- 确保输入的文法规则符合指定的格式，并且没有语法错误。
- 对于复杂的文法，可能需要花费一些时间来计算集合和构建预测分析表。

- 间接左递归存在bug，能运行且结果无误，但是不是最简便，导致分析过程有些冗余，建议能直接处理就按照直接递归走