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
string startSymbol ="S";

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

// 计算文法的 Follow 集合
// 计算文法的 Follow 集合
map<string, set<string>> calculateFollowSets(map<string, Production>& grammar, map<string, set<string>>& firstSets) {
    map<string, set<string>> followSets;

    // 初始化开始符号的 Follow 集合
    followSets[startSymbol].insert("$");

    // 标记 Follow 集合是否发生变化
    bool changed = true;

    // 开始迭代计算 Follow 集合
    while (changed) {
        changed = false;

        // 遍历所有产生式
        for (const auto& entry : grammar) {
            const string& nonTerminal = entry.first;
            const Production& productions = entry.second;

            // 遍历当前非终结符的所有产生式
            for (const string& production : productions.productions) {
                // 查找当前产生式右部中非终结符的位置
                for (size_t i = 0; i < production.size(); ++i) {
                    string symbol = string(1, production[i]);
                    // cout<<"start"<<symbol<<endl;
                    // 如果当前符号是非终结符
                    if (nonTerminals.count(symbol)) {
                        // 查找当前非终结符之后的符号，包括可能的 T' 情况
                        size_t j = i + 1;
                        string nextSymbol = string(1, production[j]);
                        while (j < production.size() && (production[j] == '\'' || isalpha(production[j]))) {
                            if(isalpha(production[j])&&production[j+1] == '\''){
                                j++;

                            }else if(isalpha(production[j])&&production[j+1] != '\''){
                                break;
                            }else if(j!=i+1){

                                nextSymbol += production[j];
                                break;
                            }else{
                                symbol += production[j];
                                nextSymbol =string(1, production[j+1]);
                                j++;
                                break;
                            }
                        }
                        // 处理 T 和 T' 之类的情况，确保我们正确处理终结符和非终结符之间的关系
                        if (j< production.size()) {
                            // cout<<nextSymbol<<endl;
                            // 如果跟随非终结符的符号是终结符，则将其加入到 Follow 集合中
                            if (terminals.count(nextSymbol)) {
                                changed |= followSets[symbol].insert(nextSymbol).second;
                            }else{
                                // 否则，将跟随非终结符的 First 集合中的符号加入到 Follow 集合中
                            for (const string& nextFirstSymbol : firstSets[nextSymbol]) {
                                if (nextFirstSymbol != "ε") {
                                    // cout<<"first:"<<symbol<<" add "<<nextFirstSymbol<<endl;
                                    changed |= followSets[symbol].insert(nextFirstSymbol).second;
                                }
                            }
                            }

                            

                            // 如果跟随非终结符的 First 集合中包含 ε，则将当前产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            if (firstSets[nextSymbol].count("ε")) {
                                for (const string& followSymbol : followSets[nonTerminal]) {
                                    // cout<<"follow:"<<symbol<<" add "<<followSymbol<<endl;
                                    changed |= followSets[symbol].insert(followSymbol).second;
                                }
                            }
                        } else {
                            // 如果非终结符是产生式的最后一个符号，则将产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            for (const string& followSymbol : followSets[nonTerminal]) {
                                // cout<<"F-F"<<symbol<<" "<<followSymbol<<endl;
                                changed |= followSets[symbol].insert(followSymbol).second;
                            }
                        }
                    }
                }
            }
        }
    }

    return followSets;
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
            if (startSymbol.empty()) {
            startSymbol = left;
        }
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
    terminals.insert(string("$"));
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

    // 计算 First 集合
    map<string, set<string>> firstSets = calculateFirstSets(grammar);

    // 计算 Follow 集合
    map<string, set<string>> followSets = calculateFollowSets(grammar, firstSets);

    // 输出 Follow 集合
    cout << "Follow sets are:" << endl;
    for (const auto& entry : followSets) {
       
        cout << entry.first << ": ";
        for (const string& followSymbol : entry.second) {
            cout << followSymbol << " ";
        }
        cout << endl;
    }

    return 0;
}
