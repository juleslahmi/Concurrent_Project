#include "galaxy.hpp"
#include "body.hpp"
#include "vector.hpp"
#include <thread>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>

class Node {
public:
    double mass;
    Vector centerOfMass;
    Vector min, max;
    bool isLeaf;
    Node* children[4];
    Body* body;
    
    Node(Vector min, Vector max) : mass(0), centerOfMass(0, 0), min(min), max(max), isLeaf(true), body(nullptr) {
        for (int i = 0; i < 4; i++) {
            children[i] = nullptr;
        }
    }
    
    ~Node() {
        for (int i = 0; i < 4; i++) {
            delete children[i];
        }
    }
    
    double getSize() const {
        return std::max(max.x - min.x, max.y - min.y);
    }
    
    int quadrant(const Vector& pos) const {
        Vector center = (min + max) * 0.5;
        int quadrant = 0;
        if (pos.x >= center.x) quadrant += 1;
        if (pos.y >= center.y) quadrant += 2;
        return quadrant;
    }
    
    bool contains(const Vector& pos) const {
        return pos.x >= min.x && pos.x <= max.x && 
               pos.y >= min.y && pos.y <= max.y;
    }

};