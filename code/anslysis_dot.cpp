#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <string>
#include <iomanip>
#include <vector>
#include <stack>
using namespace std;

// 定义一个类来表示产生式
class Production {
public:
    set<string> productions;
};

set<string> nonTerminals;
set<string> terminals;
const int Table_width = 10;
const char start_char = 'S';

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
map<string, set<string>> calculateFollowSets(map<string, Production>& grammar, map<string, set<string>>& firstSets) {
    map<string, set<string>> followSets;

    // 初始化开始符号的 Follow 集合
    followSets[*nonTerminals.begin()].insert("$");

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
                    if (nonTerminals.count(symbol)) {
                        // 查找当前非终结符之后的符号，包括可能的 T' 情况
                        size_t j = i + 1;
                        string nextSymbol = string(1, production[j]);
                        while (j < production.size() && (production[j] == '\'' || isalpha(production[j]))) {
                            if (isalpha(production[j]) && production[j + 1] == '\'') {
                                j++;
                            } else if (isalpha(production[j]) && production[j + 1] != '\'') {
                                break;
                            } else if (j != i + 1) {
                                nextSymbol += production[j];
                                break;
                            } else {
                                symbol += production[j];
                                nextSymbol = string(1, production[j + 1]);
                                j++;
                                break;
                            }
                        }
                        // 处理 T 和 T' 之类的情况，确保我们正确处理终结符和非终结符之间的关系
                        if (j < production.size()) {
                            // 如果跟随非终结符的符号是终结符，则将其加入到 Follow 集合中
                            if (terminals.count(nextSymbol)) {
                                changed |= followSets[symbol].insert(nextSymbol).second;
                            } else {
                                // 否则，将跟随非终结符的 First 集合中的符号加入到 Follow 集合中
                                for (const string& nextFirstSymbol : firstSets[nextSymbol]) {
                                    if (nextFirstSymbol != "ε") {
                                        changed |= followSets[symbol].insert(nextFirstSymbol).second;
                                    }
                                }
                            }

                            // 如果跟随非终结符的 First 集合中包含 ε，则将当前产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            if (firstSets[nextSymbol].count("ε")) {
                                for (const string& followSymbol : followSets[nonTerminal]) {
                                    changed |= followSets[symbol].insert(followSymbol).second;
                                }
                            }
                        } else {
                            // 如果非终结符是产生式的最后一个符号，则将产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            for (const string& followSymbol : followSets[nonTerminal]) {
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

map<string, set<string>> calculateSelectSets(map<string, Production>& grammar, map<string, set<string>>& firstSets, map<string, set<string>>& followSets) {
    map<string, set<string>> selectSets;

    for (const auto& entry : grammar) {
        const string& nonTerminal = entry.first;
        const Production& productions = entry.second;

        for (const string& production : productions.productions) {
            set<string> selectSet;
            bool canDeriveEpsilon = true;

            // 遍历产生式右部
            for (size_t i = 0; i < production.size(); ++i) {
                string symbol = string(1, production[i]);

                // 如果是终结符，直接添加到 SELECT 集合中
                if (terminals.count(symbol)) {
                    selectSet.insert(symbol);
                    canDeriveEpsilon = false; // 不再继续检查 epsilon
                    break;
                }

                // 如果是非终结符
                if (nonTerminals.count(symbol)) {
                    // 将该非终结符的 FIRST 集合加入到 SELECT 集合中
                    selectSet.insert(firstSets[symbol].begin(), firstSets[symbol].end());

                    // 如果该非终结符的 FIRST 集合中不包含 epsilon，则不再继续检查 epsilon
                    if (!firstSets[symbol].count("ε")) {
                        canDeriveEpsilon = false;
                        break;
                    }
                }
            }

            // 如果产生式右部能够推导出 epsilon，则将 FOLLOW 集合加入到 SELECT 集合中
            if (canDeriveEpsilon) {
                selectSet.insert(followSets[nonTerminal].begin(), followSets[nonTerminal].end());
            }

            // 构造产生式的键值
            string productionKey = nonTerminal + " -> " + production;

            // 将 SELECT 集合添加到 selectSets 中
            selectSets[productionKey] = selectSet;
        }
    }

    return selectSets;
}

// 定义预测分析表的类型
typedef map<pair<string, string>, string> PredictiveTable;

// 函数用于构建预测分析表
PredictiveTable buildPredictiveTable(map<string, Production>& grammar, map<string, set<string>>& selectSets) {
    PredictiveTable predictiveTable;

    // 遍历文法中的每个非终结符及其对应的产生式
    for (const auto& entry : grammar) {
        const string& nonTerminal = entry.first;
        const Production& productions = entry.second;

        // 遍历当前非终结符的每个产生式
        for (const string& production : productions.productions) {
            // 获取当前产生式的 SELECT 集合
            const set<string>& selectSet = selectSets[nonTerminal + " -> " + production];

            // 将当前产生式与每个终结符的组合添加到预测分析表中
            for (const string& terminal : selectSet) {
                predictiveTable[{nonTerminal, terminal}] = production;
            }
        }
    }

    return predictiveTable;
}

// 辅助函数：从预测分析表中获取产生式
string getProductionFromTable(PredictiveTable& predictiveTable, string nonTerminal, string terminal) {
    if (predictiveTable.count({nonTerminal, terminal}) != 0) {
        return predictiveTable[{nonTerminal, terminal}];
    } else {
        return ""; // 表中不存在对应的产生式
    }
}

string stackToString(const stack<char>& stk) {
    string str;
    stack<char> tempStack = stk; // 复制栈以避免修改原始栈
    while (!tempStack.empty()) {
        str = tempStack.top() + str;
        tempStack.pop();
    }
    return str;
}

// 打印当前状态
void printCurrentStatus(std::stack<char>& stk, const char* input, const std::string& action) {
    std::string stackStr;
    while (!stk.empty()) {
        stackStr = stk.top() + stackStr;
        stk.pop();
    }

    std::cout << std::setw(Table_width) << stackStr << std::setw(Table_width) << input << std::setw(Table_width) << action << std::endl;
}

void writeDOT(const std::string& sentence, PredictiveTable& predictiveTable, map<string, set<string>>& firstSets) {
    std::ofstream dotFile("analysis_process.dot");
    dotFile << "digraph AnalysisProcess {" << std::endl;
    dotFile << "  rankdir=LR;" << std::endl;

    // Output table header
    dotFile << "  node [shape=plaintext]" << std::endl;
    dotFile << "  graph [fontsize=10 fontname=\"Verdana\"]" << std::endl;
    dotFile << "  table [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">" << std::endl;
    dotFile << "    <tr><td align=\"center\">栈</td><td align=\"center\">输入</td><td align=\"center\">动作</td></tr>" << std::endl;

    // Process input string
    std::stack<char> stk;
    stk.push('$');
    stk.push(start_char);
    size_t i = 0;
    bool accepted = false; // Flag to check if the sentence is accepted
    while (i < sentence.length()) {
        char input = sentence[i];
        std::string stackStr = stackToString(stk);

        while (stk.top() != input) {
            char temp = stk.top();
            std::string top(1, temp);
            std::string production;
            if (terminals.count(top)) {
                cerr << "Error: Top of stack is a terminal, but input symbol is '" << input << "'" << endl;
                dotFile << "    <tr><td align=\"center\">" << stackStr << "</td><td align=\"center\">" << sentence.substr(i) << "</td><td align=\"center\">Fail</td></tr>" << std::endl;
                dotFile.close();
                return;
            }
            if (nonTerminals.count(top)) {
                stk.pop();
                if (stk.top() == '\'') {
                    top += '\'';
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                } else {
                    stk.push(temp);
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                }
            }
            if (production.empty()) {
                cerr << "Error: Production for table[" << top << "][" << input << "] is empty." << endl;
                dotFile << "    <tr><td align=\"center\">" << stackStr << "</td><td align=\"center\">" << sentence.substr(i) << "</td><td align=\"center\">Fail</td></tr>" << std::endl;
                dotFile << "  </table>>];" << std::endl;
                dotFile << "\"Error: Production for table[" << top << "][" << input << "] is empty.\"" << endl;
                dotFile << "\"Fail\"" << std::endl;
                dotFile << "}" << std::endl;
                dotFile.close();
                return;
            }
            if (production != "ε") {
                stk.pop();
                for (int j = static_cast<int>(production.size()) - 1; j >= 0; --j) {
                    stk.push(production[j]);
                }
            } else {
                stk.pop();
            }
            dotFile << "    <tr><td align=\"center\">" << stackStr << "</td><td align=\"center\">" << sentence.substr(i) << "</td><td align=\"center\">" << production << "</td></tr>" << std::endl;
        }
        stk.pop();
        ++i;
    }

    // Check if any remaining non-terminals can derive epsilon
     while (!stk.empty() && nonTerminals.count(string(1, stk.top()))) {
        bool canDeriveEpsilon = false;
        string top(1, stk.top());

        // Check if the top symbol can derive epsilon
        for (const string& terminal : firstSets[top]) {
            if (terminal == "ε") {
                canDeriveEpsilon = true;
                break;
            }
        }

        if (canDeriveEpsilon) {
            dotFile << "    <tr><td align=\"center\">" << stackToString(stk) << "</td><td align=\"center\">$</td><td align=\"center\">ε</td></tr>" << std::endl;
            stk.pop();
        } else {
            break;
        }
    }

    // Output table footer and final edge
    dotFile << "  </table>>];" << std::endl;
    if (stk.empty()) {
        if (i==sentence.length()) {
            // 如果栈和输入都为空，则接受
            dotFile << "\"Accept\";"  << std::endl;
            accepted = true;
        } else {
            // 如果栈为空但输入不为空，说明无法接受，不是合法句子
            dotFile << "\"Fail\";" << std::endl;
            accepted = false;
        }
    } else {
        // 如果栈非空，则继续判断
        if (sentence.empty()) {
            // 如果输入为空，说明无法接受，不是合法句子
            dotFile << "\"Fail\";" << std::endl;
            accepted = false;
        } else if(stk.top() == '$') {
            dotFile << "\"Accept\";" << std::endl;
            accepted = true;
        } else {
            // 如果栈和输入都不为空，说明无法接受，不是合法句子
            dotFile << "\"Fail\";" << std::endl;
            accepted = false;
        }
    }
    dotFile << "}" << std::endl;
    dotFile.close();

    if (accepted) {
        cout << "分析成功！" << endl;
    } else {
        cerr << "分析失败！" << endl;
    }
}


// 辅助函数：打印当前栈、输入和动作
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
    terminals.insert(string("$"));
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

    // 计算 First 集合
    map<string, set<string>> firstSets = calculateFirstSets(grammar);

    // 计算 Follow 集合
    map<string, set<string>> followSets = calculateFollowSets(grammar, firstSets);

    // 计算 Select 集合
    map<string, set<string>> selectSets = calculateSelectSets(grammar, firstSets, followSets);

    // 构建预测分析表
    PredictiveTable predictiveTable = buildPredictiveTable(grammar, selectSets);
    string sentence;
    cout << "Enter the input string:";
    cin >> sentence;
    writeDOT(sentence, predictiveTable,firstSets);
    // 等待键盘输入以停止程序
    cout << "Press any key to exit..." << endl;
    cin.get(); // 等待键盘输入
    return 0;
}
