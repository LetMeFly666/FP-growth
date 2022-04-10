/*
 * @Author: LetMeFly
 * @Date: 2022-04-10 09:43:22
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-04-10 15:51:21
 */
#include <windows.h>  // Sleep
#include <iostream>
#include <fstream>
#include <istream>  // getline
#include <cstring>
#include <vector>
using namespace std;
#define dbg(x) cout << #x << " = " << x << endl

#define EXIT1IFNOANOTHERPARAMETR if (i + 1 >= argc) exit(1)  // 如果参数个数不足就exit(1)
#define SlowExit(toSay, errCode) {cerr << toSay << endl; Sleep(500); exit(errCode);}


string dataName;  // 输入文件名
int minSupportNum = 0;  // 最小支持度
vector<vector<int>> database;  // 数据库(内存版本)


void init(int argc, char** argv);
void input();
void debug_input();
void analyMinSupportNum(string minSupportInput);


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
        SlowExit("Open input file failed.", 2);
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
        database.push_back(thisLog);
    }
    istr.close();
}

/* 将输入的最小支持度(string)转为最小支持数(int) */
void analyMinSupportNum(string minSupportInput) {

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
    debug_input();
    return 0;
}