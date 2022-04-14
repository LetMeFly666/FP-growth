/*
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:43:22
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-14 20:48:15
 */
#include <windows.h>  // Sleep
#include <algorithm>
#include <iostream>
#include <fstream>
#include <istream>  // getline
#include <cstring>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
using namespace std;
#define dbg(x) cout << #x << " = " << x << endl

#define SlowExit(toSay, errCode) {cerr << toSay << endl; Sleep(725); exit(errCode);}
#define EXIT1IFNOANOTHERPARAMETR if (i + 1 >= argc) SlowExit("[0]: Parameter not enough", 1)  // 如果参数个数不足就exit(1)


class Node;
class FP_Tree;

using Item = int;
using Items = vector<Item>;
using ItemWithTime = pair<Items, int>;
using Database = vector<ItemWithTime>;
using AppendTime = map<Item, int>;
using FrequentItemsets = vector<ItemWithTime>;
using HeadTable = map<Item, pair<Node*, Node*>>;  // [<Item, <FirstNode, LastNode>>]


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
    Node* root = new Node(0);
    HeadTable headTable;
};


void init(int argc, char** argv, Database& database);
void input(Database& database);
void debug_input();
void analyMinSupportNum(string minSupportInput, Database& database);
void debug_analyMinSupportNum();
void get1Itemset(Database& database);
void showResult();
void buildTree(Database& database, FP_Tree& fpTree);
void debug_buildTree_headTable(Database& database);
void digData(FP_Tree& fpTree, vector<Item> prefix);
template <class T>
void debug_vector(vector<T> v);
void debug_buildTree_Tree(FP_Tree& fpTree);

Node* Node::addChild(Item item, HeadTable& headTable, int appendTime) {
    Node* newNode;
    if (!childs.count(item)) {
        newNode = new Node(item, appendTime);
        childs[item] = newNode;
        newNode->father = this;
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
 * 4. 数据存入database
 */
void init(int argc, char** argv, Database& database) {
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
    input(database);
    analyMinSupportNum(minSupportInput, database);
}

/* 将dataName中的数据读入到database中 */
void input(Database& database) {  // 读入数据
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

/* 通过database建树到fpTree中 */
void buildTree(Database& database, FP_Tree& fpTree) {
    static int buildTime = 0;
    printf("the %d-th time building a tree\n", ++buildTime);
    // 统计出现次数
    AppendTime appendTime;
    for (ItemWithTime& transaction : database) {
        for (Item& item : transaction.first) {
            appendTime[item] += transaction.second;
        }
    }
    // 排序函数
    auto cmp = [&appendTime](Item& item1, Item& item2) {
        int diff = appendTime[item1] - appendTime[item2];
        if (diff)  // 出现次数不同，出现次数大的优先
            return diff > 0;
        return item1 > item2;  // 出现次数相同，编号大的优先（其实这个顺序无所谓，这样是为了和PPT保持一致）
    };
    for (ItemWithTime& transaction : database) {
        sort(transaction.first.begin(), transaction.first.end(), cmp);
        Node* root = fpTree.root;
        for (Item& item : transaction.first) {
            if (appendTime[item] < minSupportNum) {
                break;
            }
            root = root->addChild(item, fpTree.headTable, transaction.second);
        }
    }
}

/* 通过[树&前缀]进行数据挖掘 */
void digData(FP_Tree& fpTree, vector<Item> prefix) {
    auto ifIsSinglePath = [&fpTree]() {
        Node* root = fpTree.root;
        while (root) {
            if (root->childs.size() > 1)  // 非单子
                return false;
            if (root->childs.size() == 1)  // 单子
                root = root->childs.begin()->second;
            else  // 无子
                break;
        }
        return true;
    };
    if (!fpTree.root->childs.size())
        return;
    if (ifIsSinglePath()) {
        vector<Item> itemsInTree;
        Node* root = fpTree.root;
        int minAppendTime = INT_MAX;
        while (root) {
            if (root != fpTree.root) {
                itemsInTree.push_back(root->item);
                minAppendTime = min(minAppendTime, root->appendTime);
            }
            if (root->childs.size())  // 有子(必为1)
                root = root->childs.begin()->second;
            else  // 无子
                break;
        }

        printf("When the prefix is: ");  //******
        debug_vector(prefix);  //********
        printf("The single path items: ");//**********
        debug_vector(itemsInTree);  //*******

        for (int i = 1; i < (1 << (itemsInTree.size())); i++) {
            vector<Item> thisItems = prefix;
            for (int j = 0; j < itemsInTree.size(); j++) {
                if (i & (1 << j)) {
                    thisItems.push_back(itemsInTree[j]);
                }
            }
            printf("add 2^n-1 (%d times): ", minAppendTime);  //*******
            debug_vector(thisItems);  //******

            frequentItemsets.push_back({thisItems, minAppendTime});
        }
    }
    else {
        for (auto& [item, nodes] : fpTree.headTable) {  // 头表中的每个元素
            Database database;
            vector<Item> thisPrefix = prefix;
            thisPrefix.push_back(item);
            for (Node* node = nodes.first; node; node = node->next) {  // 这个元素链
                vector<Item> thisVector;
                for (Node* p = node->father; p != fpTree.root; p = p->father) {  // 往上遍历
                    thisVector.push_back(p->item);
                }
                database.push_back({thisVector, node->appendTime});
            }
            FP_Tree newFPTree;
            buildTree(database, newFPTree);
            digData(newFPTree, thisPrefix);
        }
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

/* debug buildTree 的 headTable */
void debug_buildTree_headTable(Database& database) {
    FP_Tree fpTree;
    buildTree(database, fpTree);
    HeadTable headTable = fpTree.headTable;
    for (auto[item, nodes] : headTable) {
        Node* node = nodes.first;
        cout << "item[" << item << "]:";
        bool first = true;
        while (node) {
            if (first)
                first = false;
            else
                cout << " -> ";
            cout << "(" << node << ", " << node->appendTime << ")";
            node = node->next;
        }
        puts("");
    }
}

/* 打印一个vector */
template <class T>
void debug_vector(vector<T> v) {
    printf("[");
    bool first = true;
    for (T& t : v) {
        if (first)
            first = false;
        else
            printf(", ");
        cout << (char)(t + 'a');
    }
    printf("]\n");
}

/* 打印FP-Tree */
void debug_buildTree_Tree(FP_Tree& fpTree) {
    string head = "<html><head></head><body><div class=\"mermaid\">\ngraph TD\nRoot((Root))\n";
    string tail = "\n</div></div><script src=\"./mermaid.min.js\"></script><script>mermaid.initialize({theme: 'forest',logLevel: 3,securityLevel: 'loose',flowchart: { curve: 'basis' },});</script></body></html>";
    string middle;
    queue<Node*> q;
    map<Node*, string> ma;
    ma[fpTree.root] = "Root";
    q.push(fpTree.root);
    int id = 0;
    while (q.size()) {
        Node* node = q.front();
        q.pop();
        for (auto [Item, nextNode] : node->childs) {
            q.push(nextNode);
            string thisId = to_string(id++);
            ma[nextNode] = thisId;
            middle += thisId + "((";
            middle += (char)(nextNode->item + 'a');
            middle += " : " + to_string(nextNode->appendTime);
            middle += "))\n";
            middle += ma[node] + " --> " + ma[nextNode] + "\n";
        }
    }
    ofstream ostr("source/Tree.html", ios::out);
    ostr << head + middle + tail;
    ostr.close();
}

int main(int argc, char** argv) {
    Database database;
    init(argc, argv, database);
    clock_t start = clock();
    get1Itemset(database);
    FP_Tree fpTree;
    buildTree(database, fpTree);
    debug_buildTree_Tree(fpTree);
    digData(fpTree, {});
    
    clock_t end = clock();
    cerr << "Time consumed: " << (double)(end - start) / CLK_TCK << "s" << endl;
    showResult();

    if (ifPauseBeforeExit) {
        system("pause");
    }
    return 0;
}