#include <iostream>
#include <random>
#include <stack>
#include <ctime>
#include <unordered_set>
#include <algorithm>
#include <array>

class Node{
public:
    Node(Node* up_, Node* left_, Node* right_, Node* down_):
        up(up_), left(left_), right(right_), down(down_){}


    Node* up = nullptr;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* down = nullptr;
    bool isMain = false;
    bool isRoom = false;
    bool isElevator = false;
};



class Map{
private:
    Node* map;
    Node* elevator;
    int mapSize;
    int roomCount;
    int mainRoomsCount;

public:
    Map(const int& mapSize_, const int& roomCount_) : mapSize(mapSize_), roomCount(roomCount_) {
        if(pow(mapSize, 2) < roomCount){
            roomCount = pow(mapSize, 2);
        }
        if(roomCount <= 0){
            roomCount = 1;
        }
        if(mapSize <= 0){
            mapSize = 1;
        }
    }


    void startGenerate(){
        map = createFullMap();
        elevator = createElevator();
        mainRoomsCount = roomCount * 0.6;
        if(roomCount > 0 && !elevator->left->isRoom) generateMainPaths(elevator->left);
        if(roomCount > 0 && !elevator->right->isRoom) generateMainPaths(elevator->right);
        if(roomCount > 0 && !elevator->down->isRoom) generateMainPaths(elevator->down);
        if(roomCount > 0 && !elevator->up->isRoom) generateMainPaths(elevator->up);
        if(roomCount > 0) generateOtherRoom();
    }


    void printMap() const noexcept{
        if(roomCount != 0){
            std::cout << "Error: Room count is not empty" << std::endl;
            return;
        }
        Node* cur = map;
        Node* saveLeft = map;
        while(saveLeft){
            if(cur == elevator) std::cout << '@';
            else if(cur->isMain) std::cout << '#';
            else if(cur->isRoom) std::cout << '+';
            else std::cout << ' ';

            if(cur->right == nullptr){
                std::cout << std::endl;
                saveLeft = saveLeft->down;
                cur = saveLeft;
            }else{
                cur = cur->right;
            }
        }
    }


    Node* getMap() const noexcept{
        return map;
    }


    Node* getElevatorRoom() const noexcept{
        return elevator;
    }


    int getMapSize() const noexcept{
        return mapSize;
    }


    int getRoomCount() const noexcept{
        return roomCount;
    }


    ~Map() {
        if (map) {
            for (int i = 0; i < mapSize * mapSize; ++i) {
                map[i].~Node();
            }
            operator delete[](map);
        }
    }


protected:
    Node* createElevator(){
        if(!map){
            std::cout << "Empty map!" <<  std::endl;
            return nullptr;
        }
        Node* cur = map;
        for(size_t i = 0; i < mapSize / 2; i++){
            cur = cur->right;
            cur = cur->down;
        }
        cur->isElevator = true;
        cur->isRoom = true;
        cur->isMain = true;
        roomCount--;
        return cur;
    }


    void generateMainPaths(Node* side) {
        if (!side || side->isRoom) return;

        int partOfMainRoom = mainRoomsCount < 4 ? 1 : mainRoomsCount / 4;
        std::stack<Node*> nodesStack;

        std::mt19937 gen(static_cast<unsigned int>(std::time(0) + reinterpret_cast<uintptr_t>(side)));
        std::uniform_int_distribution<> dis(1, 100);

        nodesStack.push(side);

        while (partOfMainRoom > 0 && !nodesStack.empty()) {
            side = nodesStack.top();

            if (!side->isRoom) {
                side->isMain = true;
                side->isRoom = true;
                partOfMainRoom--;
                roomCount--;
            }

            std::array<Node*, 4> neighbors = {side->up, side->down, side->left, side->right};
            std::shuffle(neighbors.begin(), neighbors.end(), gen);

            bool moved = false;
            for (Node* next : neighbors) {
                if (next && !next->isRoom && countFreeN(next) >= 2) {
                    nodesStack.push(next); // Шагаем вперед
                    moved = true;
                    break;
                }
            }
            if (!moved) {
                nodesStack.pop();
            }
        }
    }


    int countFreeN(const Node* node) const noexcept{
        int countN = 4;
        if(node->left && node->left->isRoom) countN--;
        if(node->right && node->right->isRoom) countN--;
        if(node->down && node->down->isRoom) countN--;
        if(node->up && node->up->isRoom) countN--;
        return countN;
    }


    void generateOtherRoom() {
        if (roomCount <= 0) return;

        std::vector<Node*> nodesWithNeighbors;
        nodesWithNeighbors.reserve(roomCount);
        std::unordered_set<Node*> visited;

        Node* col = map;
        while(col) {
            Node* line = col;
            while(line) {
                if(!line->isRoom) {
                    if((line->left && line->left->isRoom) || (line->up && line->up->isRoom) ||
                        (line->right && line->right->isRoom) || (line->down && line->down->isRoom)) {
                        nodesWithNeighbors.push_back(line);
                        visited.insert(line);
                    }
                }
                line = line->right;
            }
            col = col->down;
        }

        std::mt19937 gen(static_cast<unsigned int>(std::time(0)));

        while(roomCount > 0 && !nodesWithNeighbors.empty()) {
            std::uniform_int_distribution<> dis(0, nodesWithNeighbors.size() - 1);
            int roll = dis(gen);

            Node* cur = nodesWithNeighbors[roll];

            nodesWithNeighbors[roll] = nodesWithNeighbors.back();
            nodesWithNeighbors.pop_back();

            if(!cur->isRoom) {
                cur->isRoom = true;
                roomCount--;

                auto addCandidate = [&](Node* n) {
                    if(n && !n->isRoom && visited.find(n) == visited.end()) {
                        nodesWithNeighbors.push_back(n);
                        visited.insert(n);
                    }
                };

                addCandidate(cur->left);
                addCandidate(cur->up);
                addCandidate(cur->down);
                addCandidate(cur->right);
            }
        }
    }


    Node* createFullMap() {
        if (mapSize <= 0) return nullptr;

        Node* pool = static_cast<Node*>(operator new[](mapSize * mapSize * sizeof(Node)));

        for (int i = 0; i < mapSize; ++i) {
            for (int j = 0; j < mapSize; ++j) {
                int idx = i * mapSize + j;

                Node* up = (i > 0) ? &pool[(i - 1) * mapSize + j] : nullptr;
                Node* down = (i < mapSize - 1) ? &pool[(i + 1) * mapSize + j] : nullptr;
                Node* left = (j > 0) ? &pool[i * mapSize + (j - 1)] : nullptr;
                Node* right = (j < mapSize - 1) ? &pool[i * mapSize + (j + 1)] : nullptr;

                new (&pool[idx]) Node(up, left, right, down);
            }
        }
        map = pool;
        return &pool[0];
    }
};

int main()
{
    Map mp(5, 25);
    mp.startGenerate();
    mp.printMap();
    return 0;
}
