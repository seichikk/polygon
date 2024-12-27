#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <limits>
#include <chrono>
#include <algorithm>

using namespace std;

// Структура для хранения узла (координат)
struct Node {
    double x;
    double y;

    Node() : x(0.0), y(0.0) {}
    Node(double x_, double y_) : x(x_), y(y_) {}

    // Округление координат до заданной точности
    void roundCoord(int precision = 6) {
        double scale = pow(10.0, precision);
        x = floor(x * scale + 0.5) / scale;
        y = floor(y * scale + 0.5) / scale;
    }

    // Оператор сравнения для unordered_map
    bool operator==(const Node &other) const {
        return (fabs(x - other.x) < 1e-9) && (fabs(y - other.y) < 1e-9);
    }
};

// Хеш-функция для структуры Node
struct NodeHash {
    size_t operator()(const Node &n) const {
        // Переводим координаты в целые числа после умножения на 1e6
        long long scaled_x = static_cast<long long>(round(n.x * 1e6));
        long long scaled_y = static_cast<long long>(round(n.y * 1e6));
        return hash<long long>()(scaled_x) ^ (hash<long long>()(scaled_y) << 1);
    }
};

// Список смежности, Ключ: Node, Значение: вектор пар(соседний Node, вес ребра)
unordered_map<Node, vector<pair<Node, double>>, NodeHash> adjacencyList;

// Функция для вычисления Евклидова расстояния между двумя узлами
double euclidDistance(const Node &a, const Node &b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

// Функция парсинга файла и построения списка смежности
bool parseFile(const string &filePath) {
    adjacencyList.clear();
    ifstream fin(filePath);
    if (!fin.is_open()) {
        cerr << "Не удалось открыть файл: " << filePath << endl;
        return false;
    }

    string line;
    int lineNum = 0;
    while (getline(fin, line)) {
        lineNum++;
        if (line.empty()) continue;

        // Разделение строки на части, разделённые ';'
        vector<string> parts;
        stringstream ss(line);
        string part;
        while (getline(ss, part, ';')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }

        if (parts.empty()) continue;

        // Обработка первого элемента (from_node:to_node,weight)
        size_t colonPos = parts[0].find(':');
        if (colonPos == string::npos) {
            cerr << "Строка " << lineNum << ": Отсутствует ':' в первой части." << endl;
            continue;
        }

        string fromStr = parts[0].substr(0, colonPos);
        string toWeightStr = parts[0].substr(colonPos + 1);

        // Парсинг from_node
        double from_x, from_y;
        {
            stringstream fs(fromStr);
            string coord;
            if (!getline(fs, coord, ',')) {
                cerr << "Строка " << lineNum << ": Некорректный формат from_node." << endl;
                continue;
            }
            from_x = stod(coord);
            if (!getline(fs, coord)) {
                cerr << "Строка " << lineNum << ": Некорректный формат from_node." << endl;
                continue;
            }
            from_y = stod(coord);
        }
        Node fromNode(from_x, from_y);
        fromNode.roundCoord();

        // Парсинг первого to_node и weight
        double to_x, to_y, weight;
        size_t lastComma = toWeightStr.rfind(',');
        if (lastComma == string::npos) {
            cerr << "Строка " << lineNum << ": Отсутствует вес в первой части." << endl;
            continue;
        }
        string toStr = toWeightStr.substr(0, lastComma);
        string weightStr = toWeightStr.substr(lastComma + 1);
        try {
            // Парсинг to_node
            stringstream ts(toStr);
            string coord;
            if (!getline(ts, coord, ',')) {
                cerr << "Строка " << lineNum << ": Некорректный формат to_node." << endl;
                continue;
            }
            to_x = stod(coord);
            if (!getline(ts, coord)) {
                cerr << "Строка " << lineNum << ": Некорректный формат to_node." << endl;
                continue;
            }
            to_y = stod(coord);

            // Парсинг веса
            weight = stod(weightStr);
        } catch (const invalid_argument &) {
            cerr << "Строка " << lineNum << ": Ошибка при парсинге чисел." << endl;
            continue;
        }

        Node toNode(to_x, to_y);
        toNode.roundCoord();

        // Добавление рёбер в обоих направлениях (неориентированный граф)
        adjacencyList[fromNode].emplace_back(toNode, weight);
        adjacencyList[toNode].emplace_back(fromNode, weight);

        // Обработка остальных частей строки (to_node,weight)
        for (size_t i = 1; i < parts.size(); ++i) {
            string &edgeStr = parts[i];
            // Парсинг to_node и weight
            lastComma = edgeStr.rfind(',');
            if (lastComma == string::npos) {
                cerr << "Строка " << lineNum << ": Отсутствует вес в части " << i + 1 << "." << endl;
                continue;
            }
            string toPart = edgeStr.substr(0, lastComma);
            string wStr = edgeStr.substr(lastComma + 1);
            try {
                stringstream ts(toPart);
                string coord;
                if (!getline(ts, coord, ',')) {
                    cerr << "Строка " << lineNum << ": Некорректный формат to_node в части " << i + 1 << "." << endl;
                    continue;
                }
                to_x = stod(coord);
                if (!getline(ts, coord)) {
                    cerr << "Строка " << lineNum << ": Некорректный формат to_node в части " << i + 1 << "." << endl;
                    continue;
                }
                to_y = stod(coord);

                weight = stod(wStr);
            } catch (const invalid_argument &) {
                cerr << "Строка " << lineNum << ": Ошибка при парсинге чисел в части " << i + 1 << "." << endl;
                continue;
            }

            Node toNodePart(to_x, to_y);
            toNodePart.roundCoord();

            // Добавление рёбер в обоих направлениях
            adjacencyList[fromNode].emplace_back(toNodePart, weight);
            adjacencyList[toNodePart].emplace_back(fromNode, weight);
        }
    }

    fin.close();
    return true;
}

// Функция поиска ближайшего узла к заданной координате
Node findClosestNode(const Node &target) {
    Node closest;
    double minDist = numeric_limits<double>::max();
    for (const auto &kv : adjacencyList) {
        double dist = euclidDistance(kv.first, target);
        if (dist < minDist) {
            minDist = dist;
            closest = kv.first;
            if (dist == 0.0) break; // Точный узел найден
        }
    }
    return closest;
}

// Функция для восстановления пути из родительской мапы
vector<Node> reconstructPath(const unordered_map<Node, Node, NodeHash> &parent, const Node &end) {
    vector<Node> path;
    Node current = end;
    while (parent.find(current) != parent.end()) {
        path.push_back(current);
        current = parent.at(current);
    }
    // Добавляем стартовый узел
    path.push_back(current);
    // Переворачиваем путь
    reverse(path.begin(), path.end());
    return path;
}

// Кастомный компаратор для priority_queue
// Сравнивает только первый элемент пары(приоритет)

struct ComparePair {
    bool operator()(const pair<double, Node> &a, const pair<double, Node> &b) const {
        return a.first > b.first; // Минимальный приоритет (минимальное расстояние) будет на вершине
    }
};

// Поиск в ширину (BFS)
vector<Node> bfs(const Node &start, const Node &end) {
    unordered_set<Node, NodeHash> visited;
    unordered_map<Node, Node, NodeHash> parent;
    queue<Node> q;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Node current = q.front();
        q.pop();

        if (current == end) {
            return reconstructPath(parent, end);
        }

        for (const auto &neighbor : adjacencyList[current]) {
            if (visited.find(neighbor.first) == visited.end()) {
                visited.insert(neighbor.first);
                parent[neighbor.first] = current;
                q.push(neighbor.first);
            }
        }
    }

    // Путь не найден
    return {};
}

// Поиск в глубину (DFS)
vector<Node> dfs(const Node &start, const Node &end) {
    unordered_set<Node, NodeHash> visited;
    unordered_map<Node, Node, NodeHash> parent;
    stack<Node> s;

    s.push(start);
    visited.insert(start);

    while (!s.empty()) {
        Node current = s.top();
        s.pop();

        if (current == end) {
            return reconstructPath(parent, end);
        }

        for (const auto &neighbor : adjacencyList[current]) {
            if (visited.find(neighbor.first) == visited.end()) {
                visited.insert(neighbor.first);
                parent[neighbor.first] = current;
                s.push(neighbor.first);
            }
        }
    }

    // Путь не найден
    return {};
}

// Поиск кратчайшего пути по весу (Dijkstra)
vector<Node> dijkstra(const Node &start, const Node &end) {
    unordered_map<Node, double, NodeHash> dist;
    unordered_map<Node, Node, NodeHash> parent;

    // Инициализация расстояний
    for (const auto &kv : adjacencyList) {
        dist[kv.first] = numeric_limits<double>::max();
    }
    dist[start] = 0.0;

    // Приоритетная очередь с кастомным компаратором
    priority_queue<pair<double, Node>, vector<pair<double, Node>>, ComparePair> pq;
    pq.emplace(0.0, start);

    while (!pq.empty()) {
        auto [currentDist, currentNode] = pq.top();
        pq.pop();

        // Если мы достигли конца, завершаем
        if (currentNode == end) {
            return reconstructPath(parent, end);
        }

        // Пропускаем устаревшие записи
        if (currentDist > dist[currentNode] + 1e-9) continue;

        for (const auto &neighbor : adjacencyList[currentNode]) {
            double newDist = currentDist + neighbor.second;
            if (newDist + 1e-9 < dist[neighbor.first]) {
                dist[neighbor.first] = newDist;
                parent[neighbor.first] = currentNode;
                pq.emplace(newDist, neighbor.first);
            }
        }
    }

    // Путь не найден
    return {};
}

// Поиск кратчайшего пути с эвристикой (A*)
vector<Node> aStar(const Node &start, const Node &end) {
    unordered_map<Node, double, NodeHash> gScore;
    unordered_map<Node, double, NodeHash> fScore;
    unordered_map<Node, Node, NodeHash> parent;

    // Инициализация оценок
    for (const auto &kv : adjacencyList) {
        gScore[kv.first] = numeric_limits<double>::max();
        fScore[kv.first] = numeric_limits<double>::max();
    }
    gScore[start] = 0.0;
    fScore[start] = euclidDistance(start, end);

    // Приоритетная очередь с кастомным компаратором
    priority_queue<pair<double, Node>, vector<pair<double, Node>>, ComparePair> pq;
    pq.emplace(fScore[start], start);

    unordered_set<Node, NodeHash> openSet;
    openSet.insert(start);

    while (!pq.empty()) {
        auto [currentF, currentNode] = pq.top();
        pq.pop();
        openSet.erase(currentNode);

        if (currentNode == end) {
            return reconstructPath(parent, end);
        }

        for (const auto &neighbor : adjacencyList[currentNode]) {
            double tentative_gScore = gScore[currentNode] + neighbor.second;
            if (tentative_gScore + 1e-9 < gScore[neighbor.first]) {
                parent[neighbor.first] = currentNode;
                gScore[neighbor.first] = tentative_gScore;
                fScore[neighbor.first] = tentative_gScore + euclidDistance(neighbor.first, end);

                if (openSet.find(neighbor.first) == openSet.end()) {
                    pq.emplace(fScore[neighbor.first], neighbor.first);
                    openSet.insert(neighbor.first);
                }
            }
        }
    }

    // Путь не найден
    return {};
}

// Вывод пути
void printPath(const vector<Node> &path) {
    if (path.empty()) {
        cout << "Путь не найден\n";
        return;
    }
    for (size_t i = 0; i < path.size(); ++i) {
        cout << "(" << path[i].x << "," << path[i].y << ")";
        if (i != path.size() - 1) cout << " -> ";
    }
    cout << "\n";
}

int main() {
    system("chcp 65001");
    string graphFile = "C:/Users/Akimu/CLionProjects/seichik/lab_8/graph.txt";
    if (!parseFile(graphFile)) {
        return 1;
    }

    Node start(30.49996, 59.936558);
    Node end(30.308108, 59.957238);
    start.roundCoord();
    end.roundCoord();

    Node realStart = findClosestNode(start);
    Node realEnd = findClosestNode(end);

    cout << "Стартовый узел: (" << realStart.x << "," << realStart.y << ")\n";
    cout << "Конечный узел: (" << realEnd.x << "," << realEnd.y << ")\n\n";

    // BFS
    {
        auto startTime = chrono::high_resolution_clock::now();
        vector<Node> path = bfs(realStart, realEnd);
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();

        cout << "Алгоритм: BFS\n";
        cout << "Время работы: " << duration << " мкс\n";
        printPath(path);
        cout << "\n";
    }

    // DFS
    {
        auto startTime = chrono::high_resolution_clock::now();
        vector<Node> path = dfs(realStart, realEnd);
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();

        cout << "Алгоритм: DFS\n";
        cout << "Время работы: " << duration << " мкс\n";
        printPath(path);
        cout << "\n";
    }

    // Dijkstra
    {
        auto startTime = chrono::high_resolution_clock::now();
        vector<Node> path = dijkstra(realStart, realEnd);
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();

        cout << "Алгоритм: Dijkstra\n";
        cout << "Время работы: " << duration << " мкс\n";
        printPath(path);
        cout << "\n";
    }

    // A*
    {
        auto startTime = chrono::high_resolution_clock::now();
        vector<Node> path = aStar(realStart, realEnd);
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();

        cout << "Алгоритм: A*\n";
        cout << "Время работы: " << duration << " мкс\n";
        printPath(path);
        cout << "\n";
    }

    return 0;
}
