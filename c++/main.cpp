#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "array.h"
#include "singleList.h"
#include "DoubleList.h"
#include "stack.h"
#include "quene.h"
#include "value3.h"

using namespace std;

// Хранит одну структуру данных выбранного типа.
struct Data {
    char type;
    Array* array = nullptr;
    SingleList* single = nullptr;
    DoubleList* doubly = nullptr;
    Stack* stack = nullptr;
    Queue* queue = nullptr;
    CompleteTree* tree = nullptr;

    explicit Data(char type) : type(type) {
        if (type == 'M') array = arrayCreate();
        if (type == 'F') single = singleCreate();
        if (type == 'L') doubly = doubleCreate();
        if (type == 'S') stack = stackCreate();
        if (type == 'Q') queue = queueCreate();
        if (type == 'T') tree = treeCreate();
    }

    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;

    ~Data() {
        if (array != nullptr) arrayDestroy(array);
        if (single != nullptr) singleDestroy(single);
        if (doubly != nullptr) doubleDestroy(doubly);
        if (stack != nullptr) stackDestroy(stack);
        if (queue != nullptr) queueDestroy(queue);
        if (tree != nullptr) treeDestroy(tree);
    }
};

// Имя структуры связано с ее данными.
using Database = map<string, unique_ptr<Data>>;

Data& getData(Database& db, const string& name, char type, bool create = false) {
    auto it = db.find(name);
    if (it == db.end()) {
        if (!create) throw runtime_error("container does not exist");
        it = db.emplace(name, make_unique<Data>(type)).first;
    }
    if (it->second->type != type) throw runtime_error("wrong container type");
    return *it->second;
}

vector<string> values(const Data& d, bool reverse = false) {
    vector<string> result;
    if (d.type == 'M') result = arrayRead(d.array);
    if (d.type == 'F') result = singleRead(d.single, reverse);
    if (d.type == 'L') result = doubleRead(d.doubly, reverse);
    if (d.type == 'S') result = stackRead(d.stack);
    if (d.type == 'Q') result = queueRead(d.queue);
    if (d.type == 'T') {
        for (int key : treeValues(d.tree)) result.push_back(to_string(key));
    }
    if (reverse && d.type != 'F' && d.type != 'L') std::reverse(result.begin(), result.end());
    return result;
}

string show(const Data& d, bool reverse = false) {
    if (d.type == 'T') return treeRead(d.tree);
    vector<string> items = values(d, reverse);
    if (items.empty()) return "(empty)";
    ostringstream output;
    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) output << ' ';
        output << items[i];
    }
    return output.str();
}

int parseInteger(const string& text, bool index) {
    int value = 0;
    const auto parsed = from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || parsed.ec != errc{} || parsed.ptr != text.data() + text.size() ||
        (index && (value < 0 || text.front() == '-')))
        throw runtime_error(index ? "index must be a non-negative integer" : "tree key must be an integer");
    return value;
}
int treeNumber(const string& text) { return parseInteger(text, false); }

// Восстанавливает структуры из файла перед выполнением команды.
Database loadFile(const string& path) {
    Database db;
    if (!filesystem::exists(path)) return db;
    ifstream file(path);
    if (!file) throw runtime_error("cannot read data file");
    int count;
    if (!(file >> count) || count < 0 || count > 1000000) throw runtime_error("bad data file");
    for (int i = 0; i < count; i++) {
        string typeText;
        string name;
        int size;
        if (!(file >> typeText >> name >> size) || size < 0 || size > 1000000) throw runtime_error("bad data file");
        if (typeText.size() != 1 || string("MFLSQT").find(typeText) == string::npos || db.count(name))
            throw runtime_error("bad data file");
        const char type = typeText[0];
        Data& d = getData(db, name, type, true);
        vector<string> items;
        for (int j = 0; j < size; j++) {
            string value;
            if (!(file >> value)) throw runtime_error("bad data file");
            items.push_back(value);
        }
        if (type == 'S') {
            for (int j = size - 1; j >= 0; j--) stackPush(d.stack, items[j]);
        } else {
            for (const string& value : items) {
                if (type == 'M') arrayPush(d.array, value);
                if (type == 'F') singlePush(d.single, value);
                if (type == 'L') doublePush(d.doubly, value);
                if (type == 'Q') queuePush(d.queue, value);
                if (type == 'T') treeInsert(d.tree, treeNumber(value));
            }
        }
    }
    string extra;
    if (file >> extra) throw runtime_error("trailing data in file");
    if (!file.eof()) throw runtime_error("cannot read data file");
    return db;
}

// Сохраняет все структуры после команды, изменившей данные.
void saveFile(const Database& db, const string& path) {
    // Временный файл в том же каталоге: ошибка записи не разрушит исходные данные.
    string temporary = path + ".tmp.XXXXXX";
    const int descriptor = mkstemp(temporary.data());
    if (descriptor == -1) throw runtime_error("cannot create temporary data file");
    close(descriptor);
    try {
        ofstream file(temporary);
        if (!file) throw runtime_error("cannot write data file");
        file << db.size() << '\n';
        for (const auto& [name, data] : db) {
            const auto items = values(*data);
            file << data->type << ' ' << name << ' ' << items.size() << '\n';
            for (const auto& value : items) file << value << '\n';
        }
        file.close();
        if (!file) throw runtime_error("cannot write data file");
        filesystem::rename(temporary, path);
    } catch (...) {
        std::remove(temporary.c_str());
        throw;
    }
}

void need(const vector<string>& words, size_t count) {
    if (words.size() != count) throw runtime_error("wrong number of arguments");
}

int number(const string& text) { return parseInteger(text, true); }

// Проверяем команду до создания контейнера или изменения данных.
void validateCommand(const vector<string>& words) {
    const string& op = words[0];
    const string action = op.substr(1);
    size_t count = 0;
    if (op == "HELP") count = 1;
    else if (op == "PRINT") count = 2;
    else if (op == "ISMEMBER") count = 3;
    else if (op.size() >= 2 && string("MFLSQT").find(op[0]) != string::npos) {
        if (action == "CREATE") count = 2;
        else if (op[0] == 'M') {
            if (action == "PUSH" || action == "GET" || action == "DEL") count = 3;
            else if (action == "INSERT" || action == "SET") count = 4;
            else if (action == "LEN" || action == "READ") count = 2;
        } else if (op[0] == 'F' || op[0] == 'L') {
            if (action == "PUSH" || action == "INSERTHEAD" || action == "INSERTTAIL" ||
                action == "DEL" || action == "DELBEFORE" || action == "DELAFTER" ||
                action == "REMOVE" || action == "GET" || action == "FIND") count = 3;
            else if (action == "INSERTBEFORE" || action == "INSERTAFTER") count = 4;
            else if (action == "DELHEAD" || action == "DELTAIL") count = 2;
            else if (action == "READ") {
                if (words.size() == 3) {
                    if (words[2] != "FORWARD" && words[2] != "REVERSE")
                        throw runtime_error("use FORWARD or REVERSE");
                    count = 3;
                } else count = 2;
            }
        } else if (op[0] == 'S' || op[0] == 'Q') {
            if (action == "PUSH") count = 3;
            else if (action == "POP" || action == "GET" || action == "READ") count = 2;
        } else if (op[0] == 'T') {
            if (action == "INSERT" || action == "GET") count = 3;
            else if (action == "READ" || action == "ISCOMPLETE") count = 2;
        }
    }
    if (count == 0) throw runtime_error("unknown command");
    need(words, count);
}

// Выполняет команды массива.
string runArray(Database& db, const vector<string>& words, bool& changed) {
    const string& op = words[0];
    Data& d = getData(db, words[1], 'M', op == "MPUSH" || op == "MINSERT");
    if (op == "MPUSH") {
        need(words, 3);
        arrayPush(d.array, words[2]);
        changed = true;
        return words[2];
    }
    if (op == "MINSERT") {
        need(words, 4);
        arrayInsert(d.array, number(words[2]), words[3]);
        changed = true;
        return words[3];
    }
    if (op == "MGET") { need(words, 3); return arrayGet(d.array, number(words[2])); }
    if (op == "MSET") {
        need(words, 4);
        arraySet(d.array, number(words[2]), words[3]);
        changed = true;
        return words[3];
    }
    if (op == "MDEL") {
        need(words, 3);
        string result = arrayDelete(d.array, number(words[2]));
        changed = true;
        return result;
    }
    if (op == "MLEN") { need(words, 2); return to_string(arraySize(d.array)); }
    if (op == "MREAD") { need(words, 2); return show(d); }
    throw runtime_error("unknown command");
}

// Выполняет команды односвязного и двусвязного списков.
string runList(Database& db, const vector<string>& words, bool& changed) {
    const string& op = words[0];
    string action = op.substr(1);
    bool adding = action == "PUSH" || action == "INSERTHEAD" || action == "INSERTTAIL" ||
                  action == "INSERTBEFORE" || action == "INSERTAFTER";
    Data& d = getData(db, words[1], op[0], adding);
    if (action == "PUSH" || action == "INSERTHEAD" || action == "INSERTTAIL") {
        need(words, 3);
        if (d.type == 'F') {
            if (action == "PUSH") singlePush(d.single, words[2]);
            if (action == "INSERTHEAD") singleInsertHead(d.single, words[2]);
            if (action == "INSERTTAIL") singleInsertTail(d.single, words[2]);
        } else {
            if (action == "PUSH") doublePush(d.doubly, words[2]);
            if (action == "INSERTHEAD") doubleInsertHead(d.doubly, words[2]);
            if (action == "INSERTTAIL") doubleInsertTail(d.doubly, words[2]);
        }
        changed = true;
        return words[2];
    }
    if (action == "INSERTBEFORE" || action == "INSERTAFTER") {
        need(words, 4);
        if (d.type == 'F') {
            if (action == "INSERTBEFORE") singleInsertBefore(d.single, words[2], words[3]);
            else singleInsertAfter(d.single, words[2], words[3]);
        } else {
            if (action == "INSERTBEFORE") doubleInsertBefore(d.doubly, words[2], words[3]);
            else doubleInsertAfter(d.doubly, words[2], words[3]);
        }
        changed = true;
        return words[3];
    }
    if (action == "DEL" || action == "DELBEFORE" || action == "DELAFTER") {
        need(words, 3);
        string result;
        if (d.type == 'F') {
            if (action == "DEL") result = singleDelete(d.single, words[2]);
            if (action == "DELBEFORE") result = singleDeleteBefore(d.single, words[2]);
            if (action == "DELAFTER") result = singleDeleteAfter(d.single, words[2]);
        } else {
            if (action == "DEL") result = doubleDelete(d.doubly, words[2]);
            if (action == "DELBEFORE") result = doubleDeleteBefore(d.doubly, words[2]);
            if (action == "DELAFTER") result = doubleDeleteAfter(d.doubly, words[2]);
        }
        changed = true;
        return result;
    }
    if (action == "DELHEAD" || action == "DELTAIL") {
        need(words, 2);
        string result = d.type == 'F'
            ? (action == "DELHEAD" ? singleDeleteHead(d.single) : singleDeleteTail(d.single))
            : (action == "DELHEAD" ? doubleDeleteHead(d.doubly) : doubleDeleteTail(d.doubly));
        changed = true;
        return result;
    }
    if (action == "REMOVE") {
        need(words, 3);
        string result = d.type == 'F' ? singleRemove(d.single, words[2]) : doubleRemove(d.doubly, words[2]);
        changed = true;
        return result;
    }
    if (action == "FIND") {
        need(words, 3);
        return to_string(d.type == 'F' ? singleFind(d.single, words[2]) : doubleFind(d.doubly, words[2]));
    }
    if (action == "GET") {
        need(words, 3);
        return d.type == 'F' ? singleGet(d.single, words[2]) : doubleGet(d.doubly, words[2]);
    }
    if (action == "READ") {
        if (words.size() != 2 && words.size() != 3) throw runtime_error("wrong number of arguments");
        if (words.size() == 3 && words[2] != "FORWARD" && words[2] != "REVERSE") throw runtime_error("use FORWARD or REVERSE");
        return show(d, words.size() == 3 && words[2] == "REVERSE");
    }
    throw runtime_error("unknown command");
}

// Выполняет команды стека и очереди.
string runStackQueue(Database& db, const vector<string>& words, bool& changed) {
    const string& op = words[0];
    string action = op.substr(1);
    Data& d = getData(db, words[1], op[0], action == "PUSH");
    if (action == "PUSH") {
        need(words, 3);
        if (d.type == 'S') stackPush(d.stack, words[2]);
        else queuePush(d.queue, words[2]);
        changed = true;
        return words[2];
    }
    if (action == "POP") {
        need(words, 2);
        string result = d.type == 'S' ? stackPop(d.stack) : queuePop(d.queue);
        changed = true;
        return result;
    }
    if (action == "GET") {
        need(words, 2);
        return d.type == 'S' ? stackGet(d.stack) : queueGet(d.queue);
    }
    if (action == "READ") { need(words, 2); return show(d); }
    throw runtime_error("unknown command");
}

// Complete Binary Tree: дубликаты допустимы, ключи не упорядочиваются.
string runTree(Database& db, const vector<string>& words, bool& changed) {
    const string& op = words[0];
    Data& d = getData(db, words[1], 'T', op == "TINSERT");
    if (op == "TINSERT") {
        treeInsert(d.tree, treeNumber(words[2]));
        changed = true;
        return words[2];
    }
    if (op == "TGET") return treeContains(d.tree, treeNumber(words[2])) ? "TRUE" : "FALSE";
    if (op == "TISCOMPLETE") return treeIsComplete(d.tree) ? "TRUE" : "FALSE";
    return show(d);
}

// Разбирает строку команды и выбирает нужную структуру данных.
string run(Database& db, const string& query, bool& changed) {
    istringstream input(query);
    vector<string> words;
    string word;
    while (input >> word) words.push_back(word);
    if (words.empty()) throw runtime_error("empty command");
    validateCommand(words);
    const string& op = words[0];
    if (op == "HELP") {
        need(words, 1);
        return "M: CREATE PUSH INSERT GET SET DEL LEN READ; F/L: CREATE PUSH INSERTHEAD INSERTTAIL INSERTBEFORE INSERTAFTER DEL DELHEAD DELTAIL DELBEFORE DELAFTER REMOVE FIND GET READ (list arguments are values; FIND returns an index); S/Q: CREATE PUSH POP GET READ; T: CREATE INSERT GET READ ISCOMPLETE; PRINT name; ISMEMBER name key";
    }
    if (words.size() < 2) throw runtime_error("container name required");
    if (op == "PRINT") {
        need(words, 2);
        auto it = db.find(words[1]);
        if (it == db.end()) throw runtime_error("container does not exist");
        return show(*it->second);
    }
    if (op == "ISMEMBER") {
        need(words, 3);
        Data& d = getData(db, words[1], 'T');
        return treeContains(d.tree, treeNumber(words[2])) ? "TRUE" : "FALSE";
    }
    if (op.size() < 2 || string("MFLSQT").find(op[0]) == string::npos) throw runtime_error("unknown command");
    char type = op[0];
    if (op == string(1, type) + "CREATE") {
        need(words, 2);
        if (db.count(words[1])) throw runtime_error("container already exists");
        getData(db, words[1], type, true);
        changed = true;
        return "OK";
    }
    if (type == 'M') return runArray(db, words, changed);
    if (type == 'F' || type == 'L') return runList(db, words, changed);
    if (type == 'T') return runTree(db, words, changed);
    return runStackQueue(db, words, changed);
}

// Точка входа: читает аргументы, загружает файл и запускает команду.
int main(int argc, char* argv[]) {
    string file;
    string query;
    if (argc != 5) {
        cerr << "usage: dbms --file data.txt --query \"COMMAND name arguments\"\n";
        return 1;
    }
    bool hasFile = false, hasQuery = false;
    for (int i = 1; i < argc; i += 2) {
        const string option = argv[i];
        if (option == "--file" && !hasFile) { file = argv[i + 1]; hasFile = true; }
        else if (option == "--query" && !hasQuery) { query = argv[i + 1]; hasQuery = true; }
        else { cerr << "error: unknown or repeated option\n"; return 1; }
    }
    if (file.empty() || query.empty()) { cerr << "error: empty option value\n"; return 1; }
    try {
        Database db = loadFile(file);
        bool changed = false;
        string result = run(db, query, changed);
        if (changed) saveFile(db, file);
        cout << result << '\n';
    } catch (const exception& error) {
        cerr << "error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
