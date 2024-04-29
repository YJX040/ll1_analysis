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

set<string> nonTerminals;
set<string> terminals;

// 递归计算符号的 First 集合
void calculateFirst(map<string, Production>& grammar, string symbol, set<string>& firstSet) {
    // 如果当前符号是终结符，则直接加入到 First 集合中
    if (terminals.count(symbol)) {
        firstSet.insert(symbol);
        return;
    }

    // 如果当前符号是非终结符，则遍历其所有产生式
    for (const string& production : grammar[symbol].productions) {
        // 如果产生式右部为空，则加入 ε 到 First 集合中
        if (production == "ε") {
            firstSet.insert("ε");
            continue;
        }

        // 否则，递归计算右部第一个符号的 First 集合
        string firstSymbol = production.substr(0, 1);
        calculateFirst(grammar, firstSymbol, firstSet);
    }
}

// 计算文法的 First 集合
map<string, set<string>> calculateFirstSets(map<string, Production>& grammar) {
    map<string, set<string>> firstSets;
    for (const auto& entry : grammar) {
        const string& nonTerminal = entry.first;
        set<string> firstSet;
        calculateFirst(grammar, nonTerminal, firstSet);
        firstSets[nonTerminal] = firstSet;
    }
    return firstSets;
}

int main() {
    map<string, Production> grammar;
    
    set<string> firstSet;
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

    // 计算文法的 First 集合
    map<string, set<string>> firstSets = calculateFirstSets(grammar);

    // 输出文法的 First 集合
    cout << endl << "First sets:" << endl;
    for (const auto& entry : firstSets) {
        const string& nonTerminal = entry.first;
        const set<string>& firstSet = entry.second;
        cout << "First(" << nonTerminal << ") : ";
        for (const string& terminal : firstSet) {
            cout << terminal << " ";
        }
        cout << endl;
    }

    return 0;
}
