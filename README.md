<!--
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:40:57
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-10 15:03:21
-->
# FP-growth

FP-growth算法实现关联规则的数据挖掘

+ 在线文档 [https://fp-growth.letmefly.xyz](https://fp-growth.letmefly.xyz)

+ 项目地址 [https://github.com/LetMeFly666/FP-growth](https://github.com/LetMeFly666/FP-growth)

+ See [Apriori(https://github.com/LetMeFly666/Apriori)](https://github.com/LetMeFly666/Apriori) For More

本次决定先实现代码再进行优化，否则直接开始想具体哪种最优的话太慢了。

## 使用方法

```bash
FP-growth[.exe] [-i DATANAME] [-s SUPPORT] [-o RESULT] [-h]
```

其中```[]```中的内容表示可选。

### 参数列表

|参数| 是否必选 | 类型 | 描述 |
| :--: |:--: |:--: |:--: |
| ```-i``` | ✖ | 输入文件名 | 如果参数中有```-i```，那么程序将会以此参数的下一个参数作为输入文件。如```FP-growth -i source/retail.dat``` |
| ```-s``` | ✖ | 最小支持度 | 如果参数中有```-s```，那么程序将会以此参数的下一个参数作为[最小支持度(百分比)](#minSupport)。如```FP-growth -s 1.25%``` |
| ```-o``` | ✖ | 重定向输出 | 如果参数中有```-o```，那么程序将会重定向输出到下一个参数所代表的文件中。如```FP-growth -o output.txt```。相当于```FP-growth > output.txt```。提示性信息如"请输入数据文件名"不会重定向到该指定文件中。 |
| ```-h``` | ✖ | 显示帮助 | 如果参数中有```-h```，那么程序将会打开在线帮助文档[https://fp-growth.letmefly.xyz](https://fp-growth.letmefly.xyz)。 |

#### 最小支持度 <a id="minSupport"></a>

|方式 | 类型 | 格式 | 描述 |
| :--: |:--: |:--: |:--: |
| 方式一 | 百分比 | 百分数 | 可以输入一个```＞0```的百分数来表示 **最小支持度为交易条数的百分之几** 。 例如```FP-growth -s 1.25%``` 。百分数中允许```小数```的出现。|
| 方式二 | 正整数 | 正整数 | 可以输入一个正整数来表示 **最小支持度是几** 。例如```FP-growth -s 100```。 正整数中不允许小数的出现 |

### 错误类型

程序运行过程中可能会产生一些错误，其中一些将会被捕获的错误列表如下：

| 程序返回值 | 错误类型 | 描述 |
| :--: | :--: | :--: |
| ```0``` | 程序无误 | 正确运行 |
| ```1``` | 参数不足 | 如```-i```后需要参数```DATANAME```，但若```-i```后没有此参数将会返回此错误 |
| ```2``` | 无法打开文件 | 无法读入输入文件，请检查输入文件是否存在 |
