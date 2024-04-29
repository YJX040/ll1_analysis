#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;

struct Production {
    set<string> productions;
};
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
        for (const string &betaProd : betaProductions)
        {
            grammar[newNonTerminal].productions.insert(betaProd + newNonTerminal);
        }
        // 清空除了 ε 之外的所有产生式
        grammar[newNonTerminal].productions.clear();
        for (const string &alphaProd : alphaProductions)
        {
            // 将左递归产生式转移到新的非终结符上
            grammar[newNonTerminal].productions.insert(alphaProd + newNonTerminal);
        }
        grammar[newNonTerminal].productions.insert("ε");
        // 更新原始非终结符的产生式
        set<string> updatedProductions;
        for (const string &betaProd : betaProductions)
        {
            updatedProductions.insert(betaProd + newNonTerminal);
        }
        grammar[nonTerminal].productions.clear();
        for (const auto &updatedProd : updatedProductions)
        {
            grammar[nonTerminal].productions.insert(updatedProd);
        }
        // 添加新的非终结符到文法中
        nonTerminals.insert(newNonTerminal);
    }
}
// 检查是否存在间接左递归
bool hasIndirectLeftRecursion(const string &nonTerminal, const string &originalNonTerminal, map<string, Production> &grammar, set<string> &visited, set<string> &nonTerminals) {
    if (nonTerminal == originalNonTerminal) {
        return true;
    }
    visited.insert(nonTerminal);
    for (const string &production : grammar[nonTerminal].productions) {
        string nextNonTerminal = production.substr(0, 1);
        if (nonTerminals.count(nextNonTerminal) && visited.count(nextNonTerminal) && hasIndirectLeftRecursion(nextNonTerminal, originalNonTerminal, grammar, visited, nonTerminals)) {
            return true;
        }
    }
    return false;
}

// 消除间接左递归
void eliminateIndirectLeftRecursion(string nonTerminal, map<string, Production> &grammar, set<string> &nonTerminals, set<string> &processed) {
    // 如果这个非终结符已经处理过，则直接返回
    if (processed.count(nonTerminal)) {
        return;
    }
    
    // 将当前非终结符标记为已处理
    processed.insert(nonTerminal);
    
    for (const string &production : grammar[nonTerminal].productions) {
        string nextNonTerminal = production.substr(0, 1);
        if (nonTerminals.count(nextNonTerminal)) {
            set<string> visited; // 创建一个空集合，用于检查间接左递归
            if (hasIndirectLeftRecursion(nextNonTerminal, nextNonTerminal, grammar, visited, nonTerminals)) {
                vector<string> temp;
                for (const string &subProduction : grammar[nextNonTerminal].productions) {
                    temp.push_back(subProduction + production.substr(1));
                }
                for (const string &subProduction : temp) {
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



int main() {
    // 示例文法
    map<string, Production> grammar;
    grammar["A"].productions.insert("Bα");
    grammar["B"].productions.insert("Cβ");
    grammar["C"].productions.insert("Aγ");
    grammar["C"].productions.insert("a");
    
    // 非终结符集合
    set<string> nonTerminals;
    nonTerminals.insert("A");
    nonTerminals.insert("B");
    nonTerminals.insert("C");
    set<string> processed; // 创建一个空的 processed 集合
    

    // 消除间接左递归
    for (const string &nt : nonTerminals) {
        eliminateIndirectLeftRecursion(nt, grammar, nonTerminals, processed);
    }
    
    // 输出结果
    for (const auto &nt : nonTerminals) {
        cout << nt << " -> ";
        for (const auto &prod : grammar[nt].productions) {
            cout << prod << " | ";
        }
        cout << endl;
    }
    
    return 0;
}
