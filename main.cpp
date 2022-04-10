/*
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:43:22
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-10 22:19:07
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


string dataName;  // 输入文件名
int minSupportNum = 0;  // 最小支持度
vector<pair<vector<int>, int>> database;  // 数据库(内存版本) [<[itemNum, ...], appendTime>, ...]
map<int, int> appendTime;  // 某个数据库中，每个item出现过的总次数
// map<int, vector<vector<int>>> results;  // [<size, [[item, ...], ...]>]


class Node {
public:
    int thisItem;
    int count;
    map<int, Node*> childs;
    Node* next = nullptr;
    Node* father = nullptr;
    Node() {};
    Node(int num, int thisAppendTime = 1) : thisItem(num), count(thisAppendTime) {};
    ~Node() {delete this;};
    Node* addChild(int num, int thisAppendTime = 1);
};


map<int, Node*> tableHead;  // 头表


void init(int argc, char** argv);
void input();
void debug_input();
void analyMinSupportNum(string minSupportInput);
void debug_analyMinSupportNum();
void analyze(vector<int> prefix = {});
void countAppendTime();
Node* buildTree();
bool cmp(int itemA, int itemB);
bool singlePath(Node* root) {
    while (root) {
        if (root->childs.size() > 1)
            return false;
        root = root->childs.begin()->second;
    }
    return true;
}


Node* Node::addChild(int num, int thisAppendTime) {
    if (childs.count(num)) {
        Node* child = childs[num];
        child->count += thisAppendTime;
        return child;
    }
    else {
        Node* child = new Node(num, thisAppendTime);
        child->father = this;
        if (tableHead.count(num)) {
            tableHead[num]->next = child;
        }
        tableHead[num] = child;
        childs[num] = child;
        return child;
    }
}


/**
 * 1. 分析命令行参数，未传递参数提示输入
 * 2. 初始化读入数据
 * 3. 确定最小支持度
 */
void init(int argc, char** argv) {
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
    }
    if (dataName.empty()) {  // 输入文件
        cerr << "Please input the file name of the database: ";
        cin >> dataName;
    }
    if (minSupportInput.empty()) {  // 最小支持度
        cerr << "Please input the min support: ";
        cin >> minSupportInput;
    }
    input();
    analyMinSupportNum(minSupportInput);
}

/* 将dataName中的数据读入到database中 */
void input() {  // 读入数据
    ifstream istr(dataName.c_str(), ios::in);
    if (istr.fail()) {
        SlowExit("[2]: Open input file failed.", 2);
    }
    string line;
    int cnt = 0;
    while (getline(istr, line)) {
        vector<int> thisLog;
        bool lastIsNum = false;
        int num = 0;
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
}

/* 将输入的最小支持度(string)转为最小支持数(int) */
void analyMinSupportNum(string minSupportInput) {
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

/* 进行挖掘 */
void analyze(vector<int> prefix) {
    countAppendTime();
    static bool firstTime = true;
    if (firstTime) {  // 如果是第一次调用这个函数的话，先把一项集全部生成
        for (auto [item, thisAppendTime] : appendTime) {
            if (thisAppendTime >= minSupportNum) {
                printf("{%d}\n", item);
            }
        }
        firstTime = false;
    }
    Node* root = buildTree();
    if (singlePath(root)) {
        // 2^n + prefix
        root = root->next;
        printf("{");
        bool firstTime = true;
        while (root) {
            if (firstTime)
                firstTime = false;
            else
                printf(", ");
            printf("%d", root->thisItem);
        }
        for (int& t : prefix) {
            if (firstTime)
                firstTime = false;
            else
                printf(", ");
            printf("%d", t);
        }
        printf("}'s 2^n\n");
        return;
    }
    map<int, Node*> tableHeadTemp = tableHead;
    for (auto [num, head] : tableHeadTemp) {
        vector<pair<vector<int>, int>> databaseBackup;
        map<int, int> appendTimeBackup;
        swap(database, databaseBackup);
        swap(appendTime, appendTimeBackup);
        
        while (head) {
            int thisAppendTime = head->father->count;
            vector<int> thisLog;
            Node* node = head;
            while (node->father) {  // 根节点无father
                thisLog.push_back(node->thisItem);
                node = node->father;
            }
            databaseBackup.push_back({thisLog, thisAppendTime});
            head = head->next;
        }
        vector<int> thisPrefix = prefix;
        thisPrefix.push_back(num);
        analyze(thisPrefix);

        swap(database, databaseBackup);
        swap(appendTime, appendTimeBackup);
    }
}

/* (vector<vector<int>>) database -> (map<int, int>) appendTime */
void countAppendTime() {
    appendTime.clear();
    for (auto [thisLog, thisAppendTime] : database)  {
        for (int& t : thisLog) {
            appendTime[t] += thisAppendTime;
        }
    }
}

/* 建树并返回root */
Node* buildTree() {
    Node* root = new Node();
    for (auto [thisLog, thisAppendTime] : database) {
        sort(thisLog.begin(), thisLog.end(), cmp);
        Node* node = root;
        for (int& thisItem : thisLog) {
            if (appendTime[thisItem] < minSupportNum)
                break;
            node = node->addChild(thisItem, thisAppendTime);
        }
    }
    return root;
}

/* 根据商品出现次数排序 */
bool cmp(int itemA, int itemB) {
    int appendTimeDiff = appendTime[itemA] - appendTime[itemB];
    if (!appendTimeDiff) {  // 二者出现次数相同
        return itemA < itemB;  // 小的itemId优先
    }
    return appendTimeDiff > 0;  // 出现次数大的在前
}

/* Debug: 输出database(vector<vector<int>>) */
void debug_input() {
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
    database = {{{1, 2}, 1}, {{1}, 1}, {{2}, 1}, {{1}, 1}};
    while (cin >> s) {
        analyMinSupportNum(s);
        printf("minSupportNum = %d, minSupport = %lf%%\n", minSupportNum, double(minSupportNum) * 100 / ((int)database.size()));
        printf("minSupportNum - 1 = %d, minSupport_1 = %lf%%\n", minSupportNum - 1, double(minSupportNum - 1) * 100 / ((int)database.size()));
        minSupportNum = 0;
    }
    exit(0);
}

int main(int argc, char** argv) {
    init(argc, argv);
    analyze();
    return 0;
}