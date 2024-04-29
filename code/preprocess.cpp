#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <string>

using namespace std;

// 定义一个类来表示产生式
class Production {
public:
    set<string> productions;
};


int main() {
    map<string, Production> grammar;
    set<string> nonTerminals;
    set<string> terminals;

    ifstream inFile("productions.txt");
    if (!inFile) {
        cerr << "Failed to open grammar file." << endl;
        return 1;
    }

    string line;
    while (getline(inFile, line)) {
        istringstream iss(line);
        string left, arrow, right;
        if (iss >> left >> arrow >> right && arrow == "->") {
            nonTerminals.insert(left);
            grammar[left]; // 确保左部存在于 grammar 中
        } else {
            cerr << "Invalid grammar syntax: " << line << endl;
        }
    }

    inFile.close();

    // 第二次循环处理右部
    inFile.open("productions.txt"); // 重新打开文件
    while (getline(inFile, line)) {
        istringstream iss(line);
        string left, arrow, right;
        if (iss >> left >> arrow >> right && arrow == "->") {
            // 跳过大写字母以及对应的'，只处理非终结符和其他符号作为终结符
            istringstream iss_right(right);
            string part;
            while (iss_right >> part) {
                if (part != "ε") {
                    for (char c : part) {
                        if ((!isalpha(c) && c != '\'') || (isalpha(c) && islower(c))) {
                            // 如果字符不是大写字母，且不是对应的 ' 符号，作为终结符处理
                            terminals.insert(string(1, c));
                        }
                    }
                }
            }
            grammar[left].productions.insert(right);

        }
    }
    // terminals.insert(string("$"));
    inFile.close();

    // 输出终结符
    cout << "Terminals are ";
    for (const string& t : terminals) {
        cout << t << " ";
    }
    cout << endl;

    // 输出非终结符
    cout << "Non-terminals are  ";
    for (const string& nt : nonTerminals) {
        cout << nt << " ";
    }
    cout << endl;

    // 输出文法规则
    cout << endl << "After eliminating left recursion:" << endl;
    for (const auto& entry : grammar) {
        const string& nonTerminal = entry.first;
        const Production& prod = entry.second;
        cout << "Productions for " << nonTerminal << " : ";
        for (const auto& production : prod.productions) {
            cout << production << " | ";
        }
        cout << endl;
    }


    return 0;
}
