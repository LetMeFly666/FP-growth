/*
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:43:22
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-10 16:36:09
 */
#include <windows.h>  // Sleep
#include <iostream>
#include <fstream>
#include <istream>  // getline
#include <cstring>
#include <vector>
using namespace std;
#define dbg(x) cout << #x << " = " << x << endl

#define SlowExit(toSay, errCode) {cerr << toSay << endl; Sleep(725); exit(errCode);}
#define EXIT1IFNOANOTHERPARAMETR if (i + 1 >= argc) SlowExit("[0]: Parameter not enough", 1)  // 如果参数个数不足就exit(1)


string dataName;  // 输入文件名
int minSupportNum = 0;  // 最小支持度
vector<vector<int>> database;  // 数据库(内存版本)


void init(int argc, char** argv);
void input();
void debug_input();
void analyMinSupportNum(string minSupportInput);
void debug_analyMinSupportNum();


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
        database.push_back(thisLog);
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
    }
}

/* Debug: 输出database(vector<vector<int>>) */
void debug_input() {
    puts("database:");
    puts("------------------");
    for (vector<int>& thisLog : database) {
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
    puts("------------------");
}

int main(int argc, char** argv) {
    init(argc, argv);
    return 0;
}