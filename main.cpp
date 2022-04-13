/*
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:43:22
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-13 16:28:04
 */
#include <windows.h>  // Sleep
#include <algorithm>
#include <iostream>
#include <fstream>
#include <istream>  // getline
#include <cstring>
#include <vector>
#include <map>
using namespace std;
#define dbg(x) cout << #x << " = " << x << endl

#define SlowExit(toSay, errCode) {cerr << toSay << endl; Sleep(725); exit(errCode);}
#define EXIT1IFNOANOTHERPARAMETR if (i + 1 >= argc) SlowExit("[0]: Parameter not enough", 1)  // 如果参数个数不足就exit(1)


using Item = int;
using Items = vector<Item>;
using ItemWithTime = pair<Items, int>;
using Database = vector<ItemWithTime>;
using AppendTime = map<Item, int>;
using FrequentItemsets = vector<ItemWithTime>;
#define HeadTable map<Item, pair<Node*, Node*>>  // [<Item, <FirstNode, LastNode>>]


string dataName;  // 输入文件名
int minSupportNum = 0;  // 最小支持度
// Database database;  // 数据库(内存版本) [<[itemNum, ...], appendTime>, ...]
FrequentItemsets frequentItemsets;  // 频繁项集 [<[item, ...], appendTime>, ...]
bool ifPauseBeforeExit = false;  // 程序执行完是否退出


struct Node {
    Item item;
    int appendTime;
    map<Item, Node*> childs;
    Node* father = nullptr;
    Node* next = nullptr;
    Node(Item item, int appendTime = 1) : item(item), appendTime(appendTime) {};
    Node* addChild(Item item, HeadTable& headTable, int appendTime = 1);
    ~Node() {delete this;};
};

struct FP_Tree {
    Node* root;
    HeadTable headTable;
};


Database &init(int argc, char** argv);
Database &input();
void debug_input();
void analyMinSupportNum(string minSupportInput, Database& database);
void debug_analyMinSupportNum();
void get1Itemset(Database& database);
void showResult();
void buildTree();


Node* Node::addChild(Item item, HeadTable& headTable, int appendTime) {
    Node* newNode;
    if (!childs.count(item)) {
        newNode = new Node(item, appendTime);
        next = newNode;
        if (!headTable.count(item)) {  // 还没有过
            headTable[item] = {newNode, newNode};
        }
        else {
            pair<Node*, Node*>* itemInHeadtable = &headTable[item];
            itemInHeadtable->second->next = newNode;
            itemInHeadtable->second = newNode;
        }
    }
    else {
        newNode = childs[item];
        newNode->appendTime += appendTime;
    }
    return newNode;
}

/**
 * 1. 分析命令行参数，未传递参数提示输入
 * 2. 初始化读入数据
 * 3. 确定最小支持度
 * 4. 返回database
 */
Database &init(int argc, char** argv) {
    string minSupportInput;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-i")) {
            EXIT1IFNOANOTHERPARAMETR;
            dataName = argv[++i];
        }
        else if (!strcmp(argv[i], "-s")) {
            EXIT1IFNOANOTHERPARAMETR;
            minSupportInput = argv[++i];
        }
        else if (!strcmp(argv[i], "-o")) {
            EXIT1IFNOANOTHERPARAMETR;
            freopen(argv[++i], "w", stdout);  // 输出到文件
        }
        else if (!strcmp(argv[i], "-h")) {
            system("start https://fp-growth.letmefly.xyz");
        }
        else if (!strcmp(argv[i], "-p")) {
            ifPauseBeforeExit = true;
        }
    }
    if (dataName.empty()) {  // 输入文件
        cerr << "Please input the file name of the database: ";
        cin >> dataName;
    }
    if (minSupportInput.empty()) {  // 最小支持度
        cerr << "Please input the min support: ";
        cin >> minSupportInput;
    }
    Database database = input();
    analyMinSupportNum(minSupportInput, database);
    return database;
}

/* 将dataName中的数据读入到database中 */
Database &input() {  // 读入数据
    Database database;
    ifstream istr(dataName.c_str(), ios::in);
    if (istr.fail()) {
        SlowExit("[2]: Open input file failed.", 2);
    }
    string line;
    int cnt = 0;
    while (getline(istr, line)) {
        Items thisLog;
        bool lastIsNum = false;
        Item num = 0;
        for (char& c : line) {
            if (c >= '0' && c <= '9') {
                num *= 10;
                num += c - '0';
                lastIsNum = true;
            }
            else {
                if (lastIsNum) {
                    thisLog.push_back(num);
                    num = 0;
                }
                lastIsNum = false;
            }
        }
        if (lastIsNum)
            thisLog.push_back(num);
        database.push_back({thisLog, 1});
    }
    istr.close();
    return database;
}

/* 将输入的最小支持度(string)转为最小支持数(int) */
void analyMinSupportNum(string minSupportInput, Database& database) {
    /* “%”是否在minSupportInput中 */
    auto isPercentInStr = [&minSupportInput]() {
        for (const char& c : minSupportInput) {
            if (c == '%')
                return true;
        }
        return false;
    };
    if (!isPercentInStr()) {  // 只能出现纯数字[0-9]
        if (minSupportInput.empty()) {
            SlowExit("[4]: Please input the min support instand of nothin.", 4);
        }
        for (char& c : minSupportInput) {
            if (c >= '0' && c <= '9') {
                minSupportNum *= 10;
                minSupportNum += c - '0';
            }
            else {
                SlowExit("[3]: Please input a positive num if unusing '%-method'.", 3);
            }
        }
    }
    else {  // 以%结尾，前面由数字和至多一个'.'组成。若包含'.'，则'.'后可以无数字(1. 等价于 1.0)，但'.'前面不可以有数字
        for (int i = 0; i < minSupportInput.size(); i++) {
            if (minSupportInput[i] == '%' && i != minSupportInput.size() - 1)
                SlowExit("[5]: Illegal % position", 5);
            // if (i == minSupportInput.size() - 1 && minSupportInput[i] != '%')
            //     SlowExit("[5]: Illegal % position", 5);
        }
        auto ifOkDot = [&minSupportInput]() {
            int cntDots = 0;
            for (char& c : minSupportInput) {
                cntDots += c == '.';
            }
            if (!cntDots)
                return true;
            if (cntDots > 1)
                return false;
            if (minSupportInput[0] == '.')
                return false;
            return true;
        };
        if (!ifOkDot())
            SlowExit("[6]: Illegal % position|num.", 6);
        double percent = 0;
        bool alreadyDot = false;
        double k = 1;  // 1/10, 1/100, ...
        for (char& c : minSupportInput) {
            if (c == '.') {
                alreadyDot = true;
            }
            else if (c >= '0' && c <= '9') {
                if (alreadyDot) {
                    k /= 10;
                    percent += k * (c - '0');
                }
                else {
                    percent *= 10;
                    percent += c - '0';
                }
            }
            else if (c != '%') {
                SlowExit("[7]: Illegal character in '%-method'", 7);
            }
        }
        minSupportNum = percent * ((int)database.size()) / 100;
        if ((double)minSupportNum * 100 /  ((int)database.size()) < percent)
            minSupportNum++;
    }
}

/* 根据初始数据库获得频繁一项集 */
void get1Itemset(Database& database) {
    AppendTime appendTime;
    for (ItemWithTime& transaction : database) {
        for(Item& item : transaction.first) {
            appendTime[item] += transaction.second;
        }
    }
    for (AppendTime::iterator it = appendTime.begin(); it != appendTime.end(); it++) {
        if (it->second >= minSupportNum) {
            frequentItemsets.push_back({{it->first}, it->second});
        }
    }
}

/* 展示结果 */
void showResult() {
    printf("frequent item sets 's size is: %d\n", frequentItemsets.size());
    for (ItemWithTime& transaction : frequentItemsets) {
        printf("{");
        bool firstPrint = true;
        for (Item& item : transaction.first) {
            if (firstPrint)
                firstPrint = false;
            else
                printf(", ");
            printf("%d", item);
        }
        printf("} --> %d\n", transaction.second);
    }
}

/* Debug: 输出database(vector<vector<int>>) */
void debug_input(Database& database) {
    puts("database:");
    puts("------------------");
    for (auto [thisLog, thisAppendTime] : database) {
        while (thisAppendTime--) {
            bool first = true;
            for (int& t : thisLog) {
                if (first)
                    first = false;
                else
                    putchar(' ');
                cout << t;
            }
            puts("");
        }
    }
    puts("------------------");
}

/* Debug: (请将此函数放在main开头)输入一个代表最小支持度的参数并输出转换结果(并结束程序) */
void debug_analyMinSupportNum() {
    cerr << "Debug: Please continue input minsupport and I will show you the result" << endl;
    string s;
    Database database = {{{1, 2}, 1}, {{1}, 1}, {{2}, 1}, {{1}, 1}};
    while (cin >> s) {
        analyMinSupportNum(s, database);
        printf("minSupportNum = %d, minSupport = %lf%%\n", minSupportNum, double(minSupportNum) * 100 / ((int)database.size()));
        printf("minSupportNum - 1 = %d, minSupport_1 = %lf%%\n", minSupportNum - 1, double(minSupportNum - 1) * 100 / ((int)database.size()));
        minSupportNum = 0;
    }
    exit(0);
}

int main(int argc, char** argv) {
    Database database = init(argc, argv);
    get1Itemset(database);
    showResult();
    
    if (ifPauseBeforeExit) {
        system("pause");
    }
    return 0;
}