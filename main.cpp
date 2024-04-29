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
class Production
{
public:
    set<string> productions;
};
// 非终结符
set<string> nonTerminals;
// 终结符
set<string> terminals;
// 起始标志
string startSymbol;
// 起始标志
char start_char;
// 表格宽度
const int Table_width = 10;
// 文法的第一句就是开始，读取后置为0
int flag = 1;
// 函数用于提取左因子
void extractLeftFactor(map<string, Production> &grammar, const string &nonTerminal, set<string> &nonTerminals)
{
    set<string> commonPrefixes;
    for (const string &production : grammar[nonTerminal].productions)
    {
        commonPrefixes.insert(production.substr(0, 1)); // 假设首字符为左因子
    }
    for (const string &prefix : commonPrefixes)
    {
        set<string> newProductions;
        for (const string &production : grammar[nonTerminal].productions)
        {
            if (production.substr(0, 1) == prefix)
            {
                string newProduction = production.substr(1); // 提取左因子后的部分
                if (newProduction.empty())
                {
                    newProductions.insert("ε");
                }
                else
                {
                    newProductions.insert(newProduction);
                }
            }
        }
        if (newProductions.size() > 1)
        {
            string newNonTerminal;
            char newNonTerminalSymbol = 'Z'; // 从Z开始找一个新的非终结符
            while (grammar.find(string(1, newNonTerminalSymbol)) != grammar.end())
            {
                newNonTerminalSymbol--;
            }
            newNonTerminal = string(1, newNonTerminalSymbol);
            // 构造新的产生式
            for (const string &newProduction : newProductions)
            {
                grammar[newNonTerminal].productions.insert(newProduction);
            }
            // 更新原始非终结符的产生式
            set<string> updatedProductions;
            for (const string &oldProduction : grammar[nonTerminal].productions)
            {
                if (oldProduction.substr(0, 1) != prefix)
                {
                    updatedProductions.insert(oldProduction);
                }
            }
            grammar[nonTerminal].productions = updatedProductions; // 更新产生式
            grammar[nonTerminal].productions.insert(prefix + newNonTerminal);
            // 添加新的非终结符到文法中
            nonTerminals.insert(newNonTerminal);
        }
    }
}
// 函数用于解决左递归
void eliminateLeftRecursion(map<string, Production> &grammar, const string &nonTerminal, set<string> &nonTerminals)
{
    set<string> newProductions;
    set<string> alphaProductions;
    set<string> betaProductions;
    for (const string &production : grammar[nonTerminal].productions)
    {
        if (production[0] == nonTerminal[0])
        {
            alphaProductions.insert(production.substr(1));
        }
        else
        {
            betaProductions.insert(production);
        }
    }
    if (!alphaProductions.empty())
    {
        // 添加新的非终结符及其产生式
        string newNonTerminal = nonTerminal + "'";
        grammar[newNonTerminal].productions.insert("ε");
        for (const string &alphaProd : alphaProductions)
        {
            // 将左递归产生式转移到新的非终结符上
            grammar[newNonTerminal].productions.insert(alphaProd + newNonTerminal);
        }
        // 清空除了 ε 之外的所有产生式
        grammar[nonTerminal].productions.clear();
        for (const string &betaProd : betaProductions)
        {
            grammar[nonTerminal].productions.insert(betaProd + newNonTerminal);
        }
        // 添加新的非终结符到文法中
        nonTerminals.insert(newNonTerminal);
    }
}

// 检查是否存在间接左递归
bool hasIndirectLeftRecursion(const string &nonTerminal, const string &originalNonTerminal, map<string, Production> &grammar, set<string> &visited, set<string> &nonTerminals)
{
    if (nonTerminal == originalNonTerminal)
    {
        return true;
    }
    visited.insert(nonTerminal);
    for (const string &production : grammar[nonTerminal].productions)
    {
        string nextNonTerminal = production.substr(0, 1);
        if (nonTerminals.count(nextNonTerminal) && visited.count(nextNonTerminal) && hasIndirectLeftRecursion(nextNonTerminal, originalNonTerminal, grammar, visited, nonTerminals))
        {
            return true;
        }
    }
    return false;
}

// 消除间接左递归
void eliminateIndirectLeftRecursion(string nonTerminal, map<string, Production> &grammar, set<string> &nonTerminals, set<string> &processed)
{
    // 如果这个非终结符已经处理过，则直接返回
    if (processed.count(nonTerminal))
    {
        return;
    }

    // 将当前非终结符标记为已处理
    processed.insert(nonTerminal);

    for (const string &production : grammar[nonTerminal].productions)
    {
        string nextNonTerminal = production.substr(0, 1);
        if (nonTerminals.count(nextNonTerminal))
        {
            set<string> visited; // 创建一个空集合，用于检查间接左递归
            if (hasIndirectLeftRecursion(nextNonTerminal, nextNonTerminal, grammar, visited, nonTerminals))
            {
                vector<string> temp;
                for (const string &subProduction : grammar[nextNonTerminal].productions)
                {
                    temp.push_back(subProduction + production.substr(1));
                }
                for (const string &subProduction : temp)
                {
                    grammar[nonTerminal].productions.insert(subProduction);
                }
                grammar[nonTerminal].productions.erase(production);
                eliminateIndirectLeftRecursion(nonTerminal, grammar, nonTerminals, processed);
                eliminateLeftRecursion(grammar, nonTerminal, nonTerminals);
                return;
            }
        }
    }
}

// 递归计算符号的 First 集合
void calculateFirst(map<string, Production> &grammar, string symbol, set<string> &firstSet)
{
    // 如果当前符号是终结符，则直接加入到 First 集合中
    if (terminals.count(symbol))
    {
        firstSet.insert(symbol);
        return;
    }
    // 如果当前符号是非终结符，则遍历其所有产生式
    for (const string &production : grammar[symbol].productions)
    {
        // 如果产生式右部为空，则加入 ε 到 First 集合中
        if (production == "ε")
        {
            firstSet.insert("ε");
            continue;
        }
        // 否则，递归计算右部第一个符号的 First 集合
        string firstSymbol = production.substr(0, 1);
        // 判断非终结符是不是X'
        if (production.length() > 1 && production[1] == '\'')
        {
            firstSymbol += '\'';
        }
        calculateFirst(grammar, firstSymbol, firstSet);
    }
}

// 计算文法的 First 集合
map<string, set<string>> calculateFirstSets(map<string, Production> &grammar)
{
    map<string, set<string>> firstSets;
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        set<string> firstSet;
        calculateFirst(grammar, nonTerminal, firstSet);
        firstSets[nonTerminal] = firstSet;
    }
    return firstSets;
}

// 计算文法的 Follow 集合
map<string, set<string>> calculateFollowSets(map<string, Production> &grammar, map<string, set<string>> &firstSets)
{
    map<string, set<string>> followSets;

    // 初始化开始符号的 Follow 集合
    followSets[startSymbol].insert("$");

    // 标记 Follow 集合是否发生变化
    bool changed = true;

    // 开始迭代计算 Follow 集合
    while (changed)
    {
        changed = false;

        // 遍历所有产生式
        for (const auto &entry : grammar)
        {
            const string &nonTerminal = entry.first;
            const Production &productions = entry.second;

            // 遍历当前非终结符的所有产生式
            for (const string &production : productions.productions)
            {
                // 查找当前产生式右部中非终结符的位置
                for (size_t i = 0; i < production.size(); ++i)
                {
                    string symbol = string(1, production[i]);
                    if (nonTerminals.count(symbol))
                    {
                        // 查找当前非终结符之后的符号，包括可能的 T' 情况
                        size_t j = i + 1;
                        string nextSymbol = string(1, production[j]);
                        while (j < production.size() && (production[j] == '\'' || isalpha(production[j])))
                        {
                            if (isalpha(production[j]))
                            {
                                if (j + 1 < production.size() && production[j + 1] == '\'')
                                {
                                    j++;
                                }
                                else if (j + 1 < production.size() && production[j + 1] != '\'')
                                {
                                    break;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            else if (j != i + 1)
                            {
                                nextSymbol += production[j];
                                break;
                            }
                            else
                            {
                                symbol += production[j];
                                nextSymbol = string(1, production[j + 1]);
                                j++;
                                break;
                            }
                        }
                        // 处理 T 和 T' 之类的情况，确保我们正确处理终结符和非终结符之间的关系
                        if (j < production.size())
                        {
                            // 如果跟随非终结符的符号是终结符，则将其加入到 Follow 集合中
                            if (terminals.count(nextSymbol))
                            {
                                changed |= followSets[symbol].insert(nextSymbol).second;
                            }
                            else
                            {
                                // 否则，将跟随非终结符的 First 集合中的符号加入到 Follow 集合中
                                for (const string &nextFirstSymbol : firstSets[nextSymbol])
                                {
                                    if (nextFirstSymbol != "ε")
                                    {
                                        changed |= followSets[symbol].insert(nextFirstSymbol).second;
                                    }
                                }
                            }

                            // 如果跟随非终结符的 First 集合中包含 ε，则将当前产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            if (firstSets[nextSymbol].count("ε"))
                            {
                                for (const string &followSymbol : followSets[nonTerminal])
                                {
                                    changed |= followSets[symbol].insert(followSymbol).second;
                                }
                            }
                        }
                        else
                        {
                            // 如果非终结符是产生式的最后一个符号，则将产生式左部的 Follow 集合加入到当前非终结符的 Follow 集合中
                            for (const string &followSymbol : followSets[nonTerminal])
                            {
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

// 计算 SELECT 集合
map<string, set<string>> calculateSelectSets(map<string, Production> &grammar, map<string, set<string>> &firstSets, map<string, set<string>> &followSets)
{
    map<string, set<string>> selectSets;

    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &productions = entry.second;

        for (const string &production : productions.productions)
        {
            set<string> selectSet;
            bool canDeriveEpsilon = true;

            // 遍历产生式右部
            for (size_t i = 0; i < production.size(); ++i)
            {
                string symbol = string(1, production[i]);

                // 如果是终结符，直接添加到 SELECT 集合中
                if (terminals.count(symbol))
                {
                    selectSet.insert(symbol);
                    canDeriveEpsilon = false; // 不再继续检查 epsilon
                    break;
                }

                // 如果是非终结符
                if (nonTerminals.count(symbol))
                {
                    // 将该非终结符的 FIRST 集合加入到 SELECT 集合中
                    selectSet.insert(firstSets[symbol].begin(), firstSets[symbol].end());

                    // 如果该非终结符的 FIRST 集合中不包含 epsilon，则不再继续检查 epsilon
                    if (!firstSets[symbol].count("ε"))
                    {
                        canDeriveEpsilon = false;
                        break;
                    }
                }
            }

            // 如果产生式右部能够推导出 epsilon，则将 FOLLOW 集合加入到 SELECT 集合中
            if (canDeriveEpsilon)
            {
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
PredictiveTable buildPredictiveTable(map<string, Production> &grammar, map<string, set<string>> &selectSets)
{
    PredictiveTable predictiveTable;

    // 遍历文法中的每个非终结符及其对应的产生式
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &productions = entry.second;

        // 遍历当前非终结符的每个产生式
        for (const string &production : productions.productions)
        {
            // 获取当前产生式的 SELECT 集合
            const set<string> &selectSet = selectSets[nonTerminal + " -> " + production];

            // 将当前产生式与每个终结符的组合添加到预测分析表中
            for (const string &terminal : selectSet)
            {
                auto tableEntry = make_pair(nonTerminal, terminal);
                // 检查是否已经有其他产生式与相同的非终结符和终结符组合对应
                if (predictiveTable.count(tableEntry))
                {
                    cerr << "Error: Ambiguous grammar detected." << endl;
                    return {}; // 返回空的预测分析表表示出错
                }
                predictiveTable[tableEntry] = production;
            }
        }
    }

    return predictiveTable;
}

void printPredictiveTable(const map<string, Production> &grammar, const map<pair<string, string>, string> &predictiveTable, const set<string> &terminals)
{
    ofstream outputFile("./outfile/predictive_analysis_table.csv");
    if (!outputFile.is_open())
    {
        cerr << "Error: Unable to open file for writing." << endl;
        return;
    }

    outputFile << "Non-terminal";
    for (const auto &terminal : terminals)
    {
        outputFile << "," << terminal;
    }
    outputFile << endl;

    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &productions = entry.second;

        outputFile << nonTerminal;

        for (const auto &terminal : terminals)
        {
            auto it = predictiveTable.find({nonTerminal, terminal});
            if (it != predictiveTable.end())
            {
                // 在预测分析表中找到对应的产生式
                outputFile << "," << nonTerminal << "->" << it->second;
            }
            else
            {
                // 在预测分析表中找不到对应的产生式
                outputFile << ",";
            }
        }
        outputFile << endl;
    }
    outputFile.close();
}

void printPredictiveTable(map<string, Production> &grammar, const PredictiveTable &predictiveTable)
{
    cout << "Predictive Analysis Table:" << endl;
    cout << "-----------------------------------" << endl;
    // 打印列标题（终结符）
    cout << setw(20) << "Non-terminal";
    for (const auto &terminal : terminals)
    {
        cout << setw(20) << terminal;
    }
    cout << endl
         << "-----------------------------------" << endl;

    // 打印行（非终结符）及对应的产生式
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &productions = entry.second;

        cout << setw(20) << nonTerminal; // 输出非终结符

        for (const string &terminal : terminals)
        {
            auto it = predictiveTable.find({nonTerminal, terminal});
            if (it != predictiveTable.end())
            {
                if (!it->second.empty())
                {
                    // 找到并且值不为空
                    cout << setw(20) << nonTerminal << " -> " << it->second;
                }
                else
                {
                    // 值为空
                    cout << setw(20) << " "; // 输出一个空白符号，表示值为空
                }
            }
            else
            {
                // 在预测分析表中找不到对应的产生式
                cout << setw(20) << " ";
            }
        }
        cout << endl; // 换行到下一行
    }
    cout << "-----------------------------------" << endl;
}

void writePredictiveTableDot(const PredictiveTable &table)
{
    ofstream outputFile("./outfile/dot_table.dot");
    if (!outputFile.is_open())
    {
        cerr << "Error: Unable to open file for writing." << endl;
        return;
    }

    outputFile << "digraph PredictiveTable {" << endl;

    // 输出表头节点
    outputFile << "node [shape=box];" << endl;
    outputFile << "Table [label=<" << endl;
    outputFile << "<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">" << endl;

    // 输出表头
    outputFile << "<tr><td></td>";
    for (const auto &terminal : terminals)
    {
        outputFile << "<td>" << terminal << "</td>";
    }
    outputFile << "</tr>" << endl;

    // 输出表格内容
    for (const auto &entry : nonTerminals)
    {
        outputFile << "<tr><td>" << entry << "</td>";
        for (const auto &terminal : terminals)
        {
            auto it = table.find({entry, terminal});
            if (it != table.end())
            {
                outputFile << "<td>" << it->second << "</td>";
            }
            else
            {
                outputFile << "<td></td>";
            }
        }
        outputFile << "</tr>" << endl;
    }

    outputFile << "</table>>];" << endl;
    outputFile << "}" << endl;

    outputFile.close();
}

void writeDotSet(const set<string> &nonTerminals, const set<string> &terminals, const map<string, set<string>> &firstSet, const map<string, set<string>> &followSet, const map<string, set<string>> &selectSet)
{

    ofstream outputFile("./outfile/analysis_set.dot");
    if (!outputFile.is_open())
    {
        cerr << "Error: Unable to open file for writing." << endl;
        return;
    }
    outputFile << "digraph AnalysisGraph {" << endl;
    outputFile << "node [shape=box];" << endl; // 设置终结符节点形状
    outputFile << "rankdir=LR;" << endl;
    outputFile << "\"";
    for (const auto &entry : selectSet)
    {

        outputFile << entry.first << " SELECT set = { ";
        for (const auto &symbol : entry.second)
        {
            outputFile << symbol << " ";
        }
        outputFile << "}" << endl;
    }
    outputFile << "\"";
    outputFile << "\"Select set of nonterminal symbols:\"" << endl;

    outputFile << "\"";
    for (const auto &entry : followSet)
    {
        auto it = nonTerminals.find(entry.first);
        if (it != nonTerminals.end())
        {
            outputFile << entry.first << " FOLLOW set = { ";
            for (const auto &symbol : entry.second)
            {
                outputFile << symbol << " ";
            }
            outputFile << "}" << endl;
        }
    }
    outputFile << "\"" << endl;
    outputFile << "\"FOLLOW set of nonterminal symbols:\"" << endl;

    outputFile << "\"";
    for (const auto &entry : firstSet)
    {
        auto it = nonTerminals.find(entry.first);
        if (it != nonTerminals.end())
        {
            outputFile << entry.first << " FIRST set = { ";
            for (const auto &symbol : entry.second)
            {
                outputFile << symbol << " ";
            }
            outputFile << "}" << endl;
        }
    }
    outputFile << "\"";
    outputFile << "\"FIRST set of nonterminal symbols:\"" << endl;

    outputFile << "\"Terminal information:\\nNumber of terminals: " << terminals.size() << " they are: $ ( ) : a b \"" << endl;
    outputFile << "\"nonterminal information:\\nNumber of non-terminal symbols: " << nonTerminals.size() << " they are: S T T' \"" << endl;

    outputFile << "}" << endl;

    outputFile.close();
}

// 辅助函数：从预测分析表中获取产生式
string getProductionFromTable(PredictiveTable &predictiveTable, string nonTerminal, string terminal)
{
    if (predictiveTable.count({nonTerminal, terminal}) != 0)
    {
        return predictiveTable[{nonTerminal, terminal}];
    }
    else
    {
        return ""; // 表中不存在对应的产生式
    }
}

string stackToString(const stack<char> &stk)
{
    string str;
    stack<char> tempStack = stk; // 复制栈以避免修改原始栈
    while (!tempStack.empty())
    {
        str = tempStack.top() + str;
        tempStack.pop();
    }
    return str;
}

// 打印当前状态
// void printCurrentStatus(stack<char> &stk, const char *input, const string &action)
// {
//     string stackStr;
//     while (!stk.empty())
//     {
//         stackStr = stk.top() + stackStr;
//         stk.pop();
//     }

//     cout << setw(Table_width) << stackStr << setw(Table_width) << input << setw(Table_width) << action << endl;
// }

// 辅助函数：打印当前栈、输入和动作
void analyze(const string &sentence, PredictiveTable &predictiveTable, map<string, set<string>> &firstSets)
{
    // 数组模拟栈
    stack<char> stk;
    stk.push('$');        // 初始栈顶为'$'
    stk.push(start_char); // 压入开始符号

    // 输出表头
    cout << setw(Table_width) << "栈" << setw(Table_width) << "输入" << setw(Table_width) << "动作" << endl;

    // 开始分析
    size_t i = 0;
    while (i < sentence.length())
    {
        char input = sentence[i];
        // cout<<input<<endl;
        while (stk.top() != input)
        {
            char temp = stk.top();
            string top(1, temp);
            string production;
            if (terminals.count(top))
            {
                cerr << "分析栈栈顶为终结符 '" << top << "'，但输入符号为 '" << input << "'" << endl;
                return;
            }
            if (nonTerminals.count(top))
            {
                stk.pop();
                // cout<<temp;
                if (stk.top() == '\'')
                {
                    top += '\'';
                    // cout<<top;
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                }
                else
                {
                    stk.push(temp);
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                }
            }
            // cout<<production<<endl;
            if (production.empty())
            {
                cerr << "分析表 table[" << top << "][" << input << "] 项为空。" << endl;
                return;
            }

            if (production != "ε")
            {
                stk.pop(); // 弹出栈顶符号
                for (int j = static_cast<int>(production.size()) - 1; j >= 0; --j)
                {
                    stk.push(production[j]); // 将产生式右部逆序入栈
                }
            }
            else
            {
                stk.pop(); // 弹出栈顶符号
            }

            // 打印当前状态
            cout << setw(Table_width) << stackToString(stk) << setw(Table_width) << sentence.substr(i)+"$" << setw(Table_width) << production << endl;
        }
        cout << setw(Table_width) << stackToString(stk) << setw(Table_width) << sentence.substr(i)+"$" << setw(Table_width) << "匹配" << endl;
        stk.pop();
        // 非终结符不消耗输入，只更新栈中内容
        ++i;
        // cout<<stk.top()<<endl;
        // cout<<i<<endl;
    }
    while (!stk.empty())
    {
        if (nonTerminals.count(string(1, stk.top())))
        {
            char temp = stk.top();
            string top(1, temp);
            bool istemp = false;
            stk.pop();
            if (stk.top() == '\'')
            {
                top += '\'';
                stk.push(temp);
                istemp = true;
            }
            else
            {
                stk.push(temp);
            }
            bool canDeriveEpsilon = false;
            for (const string &terminal : firstSets[top])
            {
                if (terminal == "ε")
                {
                    canDeriveEpsilon = true;
                    break;
                }
            }

            if (canDeriveEpsilon)
            {
                cout << setw(Table_width) << stackToString(stk) << setw(Table_width) << "$" << setw(Table_width) << "ε" << endl;
                if (istemp)
                {
                    stk.pop();
                    istemp = false;
                }
                stk.pop();
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    // 最后检查栈和输入是否已经全部消耗完
    if (stk.empty())
    {
        if (i == sentence.length())
        {
            // 如果栈和输入都为空，则接受
            cout << setw(Table_width) << "$" << setw(Table_width) << "$" << setw(Table_width) << "接受" << endl;
            cout << "分析成功！" << endl;
        }
        else
        {
            // 如果栈为空但输入不为空，说明无法接受，不是合法句子
            cerr << "栈已空，但输入尚未结束。" << endl;
        }
    }
    else
    {
        // 如果栈非空，则继续判断
        if (sentence.empty())
        {
            // 如果输入为空，说明无法接受，不是合法句子
            cerr << "输入已空，但栈尚未清空。" << endl;
        }
        else if (stk.top() == '$')
        {
            cout << setw(Table_width) << "$" << setw(Table_width) << "$" << setw(Table_width) << "接受" << endl;
            cout << "分析成功！" << endl;
        }
        else
        {
            // 如果栈和输入都不为空，说明无法接受，不是合法句子
            cerr << "无法分析完成，栈和输入均未消耗完。" << endl;
        }
    }
}

void analyzedot(const string &sentence, PredictiveTable &predictiveTable, map<string, set<string>> &firstSets)
{
    ofstream dotFile("./outfile/analysis_process.dot");
    dotFile << "digraph AnalysisProcess {" << endl;
    dotFile << "  rankdir=LR;" << endl;

    // Output table header
    dotFile << "  node [shape=plaintext]" << endl;
    dotFile << "  graph [fontsize=10 fontname=\"Verdana\"]" << endl;
    dotFile << "  table [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">" << endl;
    dotFile << "    <tr><td align=\"center\">stack</td><td align=\"center\">input</td><td align=\"center\">action</td></tr>" << endl;

    // Process input string
    stack<char> stk;
    stk.push('$');
    stk.push(start_char);
    size_t i = 0;
    bool accepted = false; // Flag to check if the sentence is accepted
    while (i < sentence.length())
    {
        char input = sentence[i];
        string stackStr = stackToString(stk);

        while (stk.top() != input)
        {
            char temp = stk.top();
            string top(1, temp);
            string production;
            if (terminals.count(top))
            {
                cerr << "Error: Top of stack is a terminal, but input symbol is '" << input << "'" << endl;
                dotFile << "    <tr><td align=\"center\">" <<  stackToString(stk) << "</td><td align=\"center\">" << sentence.substr(i) << "</td><td align=\"center\">Fail</td></tr>" << endl;
                dotFile.close();
                return;
            }
            if (nonTerminals.count(top))
            {
                stk.pop();
                if (stk.top() == '\'')
                {
                    top += '\'';
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                }
                else
                {
                    stk.push(temp);
                    production = getProductionFromTable(predictiveTable, top, string(1, input));
                }
            }
            if (production.empty())
            {
                cerr << "Error: Production for table[" << top << "][" << input << "] is empty." << endl;
                dotFile << "    <tr><td align=\"center\">" << stackToString(stk) << "</td><td align=\"center\">" << sentence.substr(i) << "</td><td align=\"center\">Fail</td></tr>" << endl;
                dotFile << "  </table>>];" << endl;
                dotFile << "\"Error: Production for table[" << top << "][" << input << "] is empty.\"" << endl;
                dotFile << "\"Fail\"" << endl;
                dotFile << "}" << endl;
                dotFile.close();
                return;
            }
            if (production != "ε")
            {
                stk.pop();
                for (int j = static_cast<int>(production.size()) - 1; j >= 0; --j)
                {
                    stk.push(production[j]);
                }
            }
            else
            {
                stk.pop();
            }
            dotFile << "    <tr><td align=\"center\">" <<  stackToString(stk) << "</td><td align=\"center\">" << sentence.substr(i)+"$" << "</td><td align=\"center\">" << production << "</td></tr>" << endl;
        }
        dotFile << "    <tr><td align=\"center\">" <<  stackToString(stk) << "</td><td align=\"center\">" << sentence.substr(i)+"$" << "</td><td align=\"center\">" << "match" << "</td></tr>" << endl;
        stk.pop();
        ++i;
    }

    // Check if any remaining non-terminals can derive epsilon
    while (!stk.empty())
    {
        if (nonTerminals.count(string(1, stk.top())))
        {
            char temp = stk.top();
            string top(1, temp);
            stk.pop();
            bool istemp = false;
            if (stk.top() == '\'')
            {
                top += '\'';
                stk.push(temp);
                istemp = true;
            }
            else
            {
                stk.push(temp);
            }
            bool canDeriveEpsilon = false;
            // Check if the top symbol can derive epsilon
            for (const string &terminal : firstSets[top])
            {
                if (terminal == "ε")
                {
                    canDeriveEpsilon = true;
                    break;
                }
            }
            if (canDeriveEpsilon)
            {
                dotFile << "    <tr><td align=\"center\">" << stackToString(stk) << "</td><td align=\"center\">$</td><td align=\"center\">ε</td></tr>" << endl;
                if (istemp)
                {
                    stk.pop();
                    istemp = false;
                }
                stk.pop();
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    // Output table footer and final edge
    dotFile << "  </table>>];" << endl;
    if (stk.empty())
    {
        if (i == sentence.length())
        {
            // 如果栈和输入都为空，则接受
            dotFile << "\"Accept\t";
            accepted = true;
        }
        else
        {
            // 如果栈为空但输入不为空，说明无法接受，不是合法句子
            dotFile << "\"Refuse\t";
            accepted = false;
        }
    }
    else
    {
        // 如果栈非空，则继续判断
        if (sentence.empty())
        {
            // 如果输入为空，说明无法接受，不是合法句子
            dotFile << "\"Refuse\t";
            accepted = false;
        }
        else if (stk.top() == '$')
        {
            dotFile << "\"Accept\t";
            accepted = true;
        }
        else
        {
            // 如果栈和输入都不为空，说明无法接受，不是合法句子
            dotFile << "\"Refuse\t";
            accepted = false;
        }
    }
    dotFile << sentence << "\"" << endl;
    dotFile << "}" << endl;
    dotFile.close();

    if (accepted)
    {
        cout << "分析成功！" << endl;
    }
    else
    {
        cerr << "分析失败！" << endl;
    }
}

// 将 DOT 文件转换为 PNG 图片
void dotToPng(const string &dotFileName, const string &pngFileName)
{
    // 使用系统命令调用 Graphviz 的 dot 命令将 DOT 文件转换为 PNG 图片
    string command = "D:\\app\\graphviz\\bin\\dot -Tpng \"" + dotFileName + "\" -o \"" + pngFileName + "\"";
    int result = system(command.c_str());

    if (result != 0)
    {
        cerr << "转换失败！" << endl;
    }
    else
    {
        cout << "转换成功！生成的图片为：" << pngFileName << endl;
    }
}

int main()
{
    map<string, Production> grammar;

    // 读取文法文件
    ifstream inFile("./outfile/out.txt");
    if (!inFile)
    {
        cerr << "Failed to open grammar file." << endl;
        return 1;
    }

    string line;
    while (getline(inFile, line))
    {
        istringstream iss(line);
        string left, arrow, right;
        if (iss >> left >> arrow >> right && arrow == "->")
        {
            nonTerminals.insert(left);
            grammar[left]; // 确保左部存在于 grammar 中
            istringstream iss_right(right);
            string part;
            if (flag)
            {
                startSymbol = left; // 在第一次迭代时赋值给 startSymbol
                // cout<< "startSymbol:"<<startSymbol<<endl;
                start_char = startSymbol[0];
                flag = 0;
            }
            while (iss_right >> part)
            {
                grammar[left].productions.insert(part);
            }
        }
        else
        {
            cerr << "Invalid grammar syntax: " << line << endl;
        }
    }

    inFile.close();

    // 输出原始文法
    cout << "Original Grammar:" << endl;
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &prod = entry.second;
        cout << nonTerminal << " -> ";
        for (const auto &production : prod.productions)
        {
            cout << production << " | ";
        }
        cout << endl;
    }
    // 对每个非终结符进行左因子提取
    for (const string &nonTerminal : nonTerminals)
    {
        extractLeftFactor(grammar, nonTerminal, nonTerminals);
    }
    // 输出处理后的文法规则
    cout << endl
         << "After extracting left factors:" << endl;
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &prod = entry.second;
        cout << nonTerminal << " -> ";
        for (const auto &production : prod.productions)
        {
            cout << production << " | ";
        }
        cout << endl;
    }

    // 创建一个空的 processed 集合
    set<string> processed;
    // 消除间接左递归
    for (const string &nonTerminal : nonTerminals)
    {

        // eliminateIndirectLeftRecursion(nonTerminal, grammar, nonTerminals, processed);
        eliminateLeftRecursion(grammar, nonTerminal, nonTerminals);
    }

    // 输出消除左递归后的文法
    cout << endl
         << "After eliminating left recursion:" << endl;
    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &prod = entry.second;
        cout << nonTerminal << " -> ";
        for (const auto &production : prod.productions)
        {
            cout << production << " | ";
        }
        cout << endl;
    }
    ofstream outputFile("./outfile/productions.txt");
    if (!outputFile.is_open())
    {
        cerr << "Error opening file." << endl;
        return 1;
    }

    for (const auto &entry : grammar)
    {
        const string &nonTerminal = entry.first;
        const Production &prod = entry.second;
        for (const auto &production : prod.productions)
        {
            outputFile << nonTerminal << " -> " << production << endl;
        }
    }

    outputFile.close();
    cout << "Productions have been written to productions.txt" << endl;

    ifstream inFile2("./outfile/productions.txt");
    if (!inFile2)
    {
        cerr << "Failed to open grammar file." << endl;
        return 1;
    }
    string line2;
    while (getline(inFile2, line2))
    {
        istringstream iss(line2);
        string left, arrow, right;
        if (iss >> left >> arrow >> right && arrow == "->")
        {
            nonTerminals.insert(left);
            grammar[left]; // 确保左部存在于 grammar 中
        }
        else
        {
            cerr << "Invalid grammar syntax: " << line2 << endl;
        }
    }

    inFile2.close();

    // 第二次循环处理右部
    inFile2.open("./outfile/productions.txt"); // 重新打开文件
    while (getline(inFile2, line2))
    {
        istringstream iss(line2);
        string left, arrow, right;
        if (iss >> left >> arrow >> right && arrow == "->")
        {
            // 跳过大写字母以及对应的'，只处理非终结符和其他符号作为终结符
            istringstream iss_right(right);
            string part;
            while (iss_right >> part)
            {
                if (part != "ε")
                {
                    for (char c : part)
                    {
                        if ((!isalpha(c) && c != '\'') || (isalpha(c) && islower(c)))
                        {
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
    inFile2.close();

    // 计算 First 集合
    map<string, set<string>> firstSets = calculateFirstSets(grammar);

    // 计算 Follow 集合
    map<string, set<string>> followSets = calculateFollowSets(grammar, firstSets);

    // 计算 Select 集合
    map<string, set<string>> selectSets = calculateSelectSets(grammar, firstSets, followSets);
    // 输出 Select 集合
    cout << "Terminals are ";
    for (const string &t : terminals)
    {
        cout << t << " ";
    }
    cout << endl;

    // 输出非终结符
    cout << "Non-terminals are  ";
    for (const string &nt : nonTerminals)
    {
        cout << nt << " ";
    }
    cout << endl;

    // 输出文法的 First 集合
    cout << endl
         << "First sets:" << endl;
    for (const auto &entry : firstSets)
    {
        const string &nonTerminal = entry.first;
        const set<string> &firstSet = entry.second;
        if(nonTerminals.count(nonTerminal)){
            cout << "First(" << nonTerminal << ") : ";
            for (const string &terminal : firstSet)
            {
                cout << terminal << " ";
            }
            cout << endl;
        }
        
    }
    cout << endl;
    cout << "Follow sets are:" << endl;
    for (const auto &entry : followSets)
    {
        if(nonTerminals.count(entry.first)){
            cout << "Follow("<< entry.first << "): ";
            for (const string &followSymbol : entry.second)
            {
                cout << followSymbol << " ";
            }
            cout << endl;
        }
    }
    cout << endl;
    cout << "Select sets are:" << endl;
    for (const auto &entry : selectSets)
    {
        cout << entry.first << ": ";
        for (const string &selectSymbol : entry.second)
        {
            cout << selectSymbol << " ";
        }
        cout << endl;
    }
    cout << endl;
    // 构建预测分析表
    PredictiveTable predictiveTable = buildPredictiveTable(grammar, selectSets);
    // 检查预测分析表是否为空
    if (predictiveTable.empty())
    {
        // 如果为空，说明存在冲突，返回 1
        cerr << "Error: Conflicting grammar detected." << endl;
        return 1;
    }
    // 打印预测分析表
    printPredictiveTable(grammar, predictiveTable);
    printPredictiveTable(grammar, predictiveTable, terminals);
    writeDotSet(nonTerminals, terminals, firstSets, followSets, selectSets);
    writePredictiveTableDot(predictiveTable);
    string sentence;
    cout << "Enter the input string:";
    cin >> sentence;
    analyze(sentence, predictiveTable, firstSets);
    analyzedot(sentence, predictiveTable, firstSets);
    // 等待键盘输入以停止程序
    dotToPng("./outfile/analysis_process.dot", "./outfile/analysis_process.png");
    dotToPng("./outfile/analysis_set.dot", "./outfile/analysis_set.png");
    dotToPng("./outfile/dot_table.dot", "./outfile/dot_table.png");
    cout << "Press any key to exit..." << endl;
    cin.get(); // 等待键盘输入
    return 0;
}