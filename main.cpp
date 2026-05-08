#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <locale>
#include <iomanip>
#include <fstream>
#include <codecvt>
#include <cstdlib>
using namespace std;

// функция подсчета частот
vector<pair<wchar_t,int>> count(wstring stroka) {
    vector<pair<wchar_t, int>> chastoti;
    
    for (wchar_t x : stroka) {
        bool found = false;
        for (int i = 0; i < chastoti.size(); i++) {
            if (chastoti[i].first == x) {
                chastoti[i].second++;
                found = true;
                break;
            }
        }

        if (!found) {
            chastoti.push_back({x, 1});
        }
    }

    return chastoti;
}

// структура дерева Хаффмана
struct Node {
    wchar_t symbol;
    int chastota;
    Node* left;
    Node* right;

    // конуструктор
    Node(wchar_t s, int f) : symbol(s), chastota(f), left(nullptr), right(nullptr) {}
};

// генерация кодов Хаффмана
void generateCodes(Node* node, wstring code, vector<pair<wchar_t, wstring>>& codes) {
    if (!node) return; // если пусто - delete(system32)
    
    // если дошли до листа (нет детей и символ не '\0'), добавляем код в вектор
    if (!node -> left && !node -> right && node -> symbol != L'\0') {
        codes.push_back({node -> symbol, code});
        return;
    }
    
    // влево - добавляем 0, вправо - добавляем 1
    generateCodes(node -> left, code + L"0", codes);
    generateCodes(node -> right, code + L"1", codes);
}

// построение дерева Хаффмана (снизу вверх)
Node* buildHuffmanTree(vector<pair<wchar_t,int>>& chastoti) {
    vector<Node*> nodes;

    for (int i = 0; i < chastoti.size(); i++) {
        nodes.push_back(new Node(chastoti[i].first, chastoti[i].second));
    }

    // сортируем по частоте (через лямбда функцию)
    sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) {return a -> chastota < b -> chastota; });

    while (nodes.size() > 1) {
        // берем 2 самых редких символа
        Node* left = nodes[0];
        Node* right = nodes[1];

        // удаляем их нафик из вектора
        nodes.erase(nodes.begin());
        nodes.erase(nodes.begin());
        
        // создаем родителя, дети у которого - те самые 2 символа, а частота - сумма частот этих символов
        Node* parent = new Node(L'\0', left -> chastota + right -> chastota);
        parent -> left = left;
        parent -> right = right;
        
        nodes.push_back(parent);

        // snova sortirovka
        sort(nodes.begin(), nodes.end(), 
            [](Node* a, Node* b) { return a -> chastota < b -> chastota; });
    }

    return nodes[0];
}

// вывод таблицы
void printTable(const vector<pair<wchar_t, int>>& chastoti, 
                const vector<pair<wchar_t, wstring>>& codes) {
    wcout << "\n";
    wcout << setw(10) << left << L"Символ " 
         << setw(12) << left << L"Частота " 
         << L"Код Хаффмана " << endl;
    wcout << wstring(35, L'-') << endl;
    
    for (int i = 0; i < chastoti.size(); i++) {
        wchar_t symbol = chastoti[i].first;
        int chastota = chastoti[i].second;
        wstring code = L"";
        
        // находим код для этого символа
        for (int j = 0; j < codes.size(); j++) {
            if (codes[j].first == symbol) {
                code = codes[j].second;
                break;
            }
        }
        wcout << setw(10) << left << (L"'" + wstring(1, symbol) + L"'")
              << setw(12) << left << chastota 
              << code << endl;
    }
    wcout << wstring(35, L'-') << endl;
}

void writeNodeRecursive(Node* node, int id, ofstream& file) {
    if (!node) return;
    
    // если лист
    if (!node -> left && !node -> right) {
        // конвертируем wchar_t в обычную строку
        string sym;
        if (node -> symbol == L' ') sym = "space";

        else if (node -> symbol == L'\n') sym = "\\\\n";

        else {
            // для русских букв и прочих широких символов
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            sym = converter.to_bytes(node -> symbol);
        }
        
        file << "    node" << id << " [shape=box, label=\"" << sym 
             << "\\nf=" << node -> chastota << "\"];\n";

    } else {
        file << "    node" << id << " [shape=circle, label=\"" 
             << node -> chastota << "\"];\n";
    }
    
    // готови айдишники детей
    static int nextId = 1; // переменная живет между вызовами функций
    int leftId = nextId++;
    int rightId = nextId++;
    
    if (node -> left) {
        writeNodeRecursive(node -> left, leftId, file);
        file << "    node" << id << " -> node" << leftId 
             << " [label=\"0\"];\n";
    }
    if (node -> right) {
        writeNodeRecursive(node -> right, rightId, file);
        file << "    node" << id << " -> node" << rightId 
             << " [label=\"1\"];\n";
    }
}

void exportToDot(Node* root, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        wcout << L"Ошибка создания файла!" << endl;
        return;
    }
    
        // заголовок .dot файла (стандартный для Graphviz)
    file << "digraph HuffmanTree {\n"; // digraph = направленный граф (стрелки)
    file << "    node [fontname=\"Impact\"];\n\n"; // шрифт по умолчанию для всех узлов
    
    writeNodeRecursive(root, 0, file);
    
    file << "}\n"; // закрывающая скобка
    file.close(); // пока
    
    // экспорт
    wcout << L"Дерево экспортировано в " << wstring(filename.begin(), filename.end()) << endl;

    // топ 10 смешных команд на линукс для начинающих
    string pngFile = filename.substr(0, filename.find_last_of('.')) + ".png";
    string command = "dot -Tpng \"" + filename + "\" -o \"" + pngFile + "\"";
    system(command.c_str());  // выполняет команду как в консоли
}

int main() {

    setlocale(LC_ALL, "");
    wcin.imbue(locale(""));
    wcout.imbue(locale(""));

    wcout << L"Введите строку: ";
    wstring input;  
    getline(wcin, input);
    
    if (input.empty()) {
        wcout << L"По русски же написано введите" << endl;
        return 0;
    }
    
    // считаем частоты
    auto chastoti = count(input);
    
    // строим дерево Хаффмана
    Node* root = buildHuffmanTree(chastoti);
    
    // генерируем коды
    vector<pair<wchar_t, wstring>> codes;
    generateCodes(root, L"", codes);
    
    // выводим таблицу
    printTable(chastoti, codes);

    exportToDot(root, "huffman_tree.dot");
    
    return 0;
}