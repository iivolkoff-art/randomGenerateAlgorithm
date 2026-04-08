#include <iostream>
#include <random>
#include <stack>
#include <ctime>
#include <unordered_set>

class Node{
public:
    Node(Node* up_, Node* left_, Node* right_, Node* down_):
        up(up_), left(left_), right(right_), down(down_){}

    bool isMain = false;
    bool isRoom = false;
    bool isElevator = false;

    Node* up = nullptr;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* down = nullptr;
};



class Map{
private:
    Node* map;
    int mapSize;
    int roomCount;
    Node* elevator;
    int mainRoomsCount;

public:
    Map(const int& mapSize_, const int& roomCount_) noexcept : mapSize(mapSize_), roomCount(roomCount_) {
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
        if(roomCount > 0) generateMainPaths(elevator->left);
        if(roomCount > 0) generateMainPaths(elevator->right);
        if(roomCount > 0) generateMainPaths(elevator->down);
        if(roomCount > 0) generateMainPaths(elevator->up);

        if(roomCount > 0) generateOtherRoom();
    }


    void printMap() noexcept{
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

    Node* getMap() noexcept{
        return map;
    }

    Node* getElevatorRoom()noexcept{
        return elevator;
    }

    int getMapSize()noexcept{
        return mapSize;
    }

    int getRoomCount()noexcept{
        return roomCount;
    }

    ~Map() noexcept {
        Node* currentRow = map;
        while (currentRow) {
            Node* nextRow = currentRow->down;
            Node* currentCol = currentRow;
            while (currentCol) {
                Node* nextCol = currentCol->right;
                delete currentCol;
                currentCol = nextCol;
            }
            currentRow = nextRow;
        }
    }


protected:
    Node* createElevator() noexcept{
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


    void generateMainPaths(Node* side){
        if(!side){
            std::cout << ("Error: Side for main path is none") << std::endl;
            return;
        }

        int partOfMainRoom = mainRoomsCount < 4 ? 1 : mainRoomsCount / 4;

        std::stack<Node*> nodesStack;

        std::mt19937 gen = std::mt19937(static_cast<unsigned int>(std::time(0)));
        std::uniform_int_distribution<> dis = std::uniform_int_distribution<>(1, 100);

        while(partOfMainRoom){
            if (side == nullptr) break;
            side->isMain = true;
            side->isRoom = true;
            partOfMainRoom--;
            roomCount--;
            nodesStack.push(side);

            int roll = dis(gen);

            std::vector<Node*> neighbors;

            if (roll <= 25) neighbors = {side->up, side->down, side->left, side->right};
            else if (roll <= 50) neighbors = {side->down, side->up, side->left, side->right};
            else if (roll <= 75) neighbors = {side->left, side->down, side->up, side->right};
            else neighbors = {side->right, side->down, side->up, side->left};

            bool moved = false;
            for (Node* next : neighbors) {
                if (next && !next->isRoom && countFreeN(next) >= 3) {
                    side = next;
                    moved = true;
                    break;
                }
            }

            if (!moved) {
                if (!nodesStack.empty()) {
                    side = nodesStack.top();
                    nodesStack.pop();
                } else {
                    break;
                }
            }

        }
    }

    int countFreeN(const Node* node) noexcept{
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
    Node* createFullMap() noexcept{
        if (mapSize <= 0) return nullptr;

        Node* firstInRow = nullptr;
        Node* prevRowNode = nullptr;
        Node* leftNode = nullptr;
        Node* topLeftCorner = nullptr;

        for (int i = 0; i < mapSize; ++i) {
            leftNode = nullptr;
            for (int j = 0; j < mapSize; ++j) {
                Node* currentNode = new Node(nullptr, nullptr, nullptr, nullptr);

                if (i == 0 && j == 0) topLeftCorner = currentNode;
                if (leftNode) {
                    leftNode->right = currentNode;
                    currentNode->left = leftNode;
                }
                if (prevRowNode) {
                    prevRowNode->down = currentNode;
                    currentNode->up = prevRowNode;
                    prevRowNode = prevRowNode->right;
                }
                leftNode = currentNode;

                if (j == 0) firstInRow = currentNode;
            }
            prevRowNode = firstInRow;
        }

        return topLeftCorner;
    }
};

int main()
{
    Map mp(5, 24);
    mp.startGenerate();
    mp.printMap();
}
