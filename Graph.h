#pragma once

#include "ZBaseTypes.h"
#include "List.h"
#include "Heap.h"

namespace ZSharp {

template<typename T>
class Graph final {
  private:
  struct GraphNode;
  struct DijkstraNode;

  struct GraphEdge {
    GraphNode* v;
    size_t weight;

    GraphEdge(GraphNode* inV, size_t inWeight) : v(inV), weight(inWeight) {}

    bool operator==(const GraphEdge& rhs) const {
      return v == rhs.v;
    }
  };

  struct GraphNode {
    T value;
    List<GraphEdge> adjacentList;
    DijkstraNode* dijkstraNode = nullptr;

    GraphNode(const T& inValue) : value(inValue) {
    }

    GraphNode(const GraphNode& rhs) : value(rhs.value), adjacentList(rhs.adjacentList) {
    }

    ~GraphNode() {
      if (dijkstraNode != nullptr) {
        delete dijkstraNode;
      }
    }

    bool operator==(const T& rhs) {
      return value == rhs;
    }

    bool operator==(const T& rhs) const {
      return value == rhs;
    }

    bool operator==(const GraphNode& rhs) {
      return value == rhs.value;
    }

    bool operator==(const GraphNode& rhs) const {
      return value == rhs.value;
    }

    bool operator>(const GraphNode& rhs) const {
      return value > rhs.value;
    }

    bool operator<(const GraphNode& rhs) const {
      return value < rhs.value;
    }
  };

  struct DijkstraNode {
    bool visited = false;
    size_t distance = 0;
    GraphNode* previous = nullptr;

    DijkstraNode(bool inVisited, size_t inDistance) :
      visited(inVisited), distance(inDistance) {}
  };

  struct MinFunctor {
    bool operator()(GraphNode*& lhs, GraphNode*& rhs) const {
      return lhs->dijkstraNode->distance > rhs->dijkstraNode->distance;
    }
  };

  public:
  class Iterator {
    public:
    Iterator(typename List<GraphNode>::Iterator iter) : mIter(iter) {
    }

    Iterator& operator++() {
      mIter++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(mIter);
      ++(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mIter == rhs.mIter;
    }

    bool operator!=(const Iterator& rhs) {
      return mIter != rhs.mIter;
    }

    T& operator*() const {
      return (*mIter).value;
    }

    T* operator->() {
      return (&(mIter.operator->()).value);
    }

    private:
    typename List<GraphNode>::Iterator mIter;
  };

  Graph() {
  }

  bool AddVertex(const T& value) {
    for (const GraphNode& node : mGraph) {
      if (node == value) {
        return false;
      }
    }

    GraphNode node(value);
    mGraph.Add(node);
    return true;
  }

  bool AddEdge(const T& a, const T& b, size_t weight) {
    // Check for self references.
    if (a == b) {
      return false;
    }

    GraphNode* aNode = nullptr;
    GraphNode* bNode = nullptr;

    // Find nodes matching the values.
    for (GraphNode& vertex : mGraph) {
      if (aNode != nullptr && bNode != nullptr) {
        break;
      }

      if (vertex == a) {
        aNode = &vertex;
      }
      else if (vertex == b) {
        bNode = &vertex;
      }
    }

    // Only add edges if the vertices connecting them exists.
    if (aNode == nullptr || bNode == nullptr) {
      return false;
    }

    // Check for duplicate edges.
    GraphEdge edgeA(bNode, weight);
    GraphEdge edgeB(aNode, weight);

    const bool aContainsB = aNode->adjacentList.Contains(edgeA);
    const bool bContainsA = bNode->adjacentList.Contains(edgeB);
    if (aContainsB && bContainsA) {
      return false;
    }
    else {
      aNode->adjacentList.Add(edgeA);
      bNode->adjacentList.Add(edgeB);
    }

    return true;
  }

  bool RemoveVertex(const T& value) {
    GraphNode* nodeToRemove = nullptr;

    for (GraphNode& node : mGraph) {
      if (node.value != value) {
        GraphEdge* edgeToRemove = nullptr;

        for (GraphEdge& edge : node.adjacentList) {
          if (edge.v->value == value) {
            edgeToRemove = &edge;
            break;
          }
        }

        if (edgeToRemove != nullptr) {
          node.adjacentList.Remove(*edgeToRemove);
        }
      }
      else {
        nodeToRemove = &node;
      }
    }

    if (nodeToRemove == nullptr) {
      return false;
    }
    else {
      return mGraph.Remove(*nodeToRemove);
    }
  }

  bool RemoveEdge(const T& a, const T& b) {
    GraphNode* nodeToRemoveA = nullptr;
    GraphNode* nodeToRemoveB = nullptr;

    for (GraphNode& node : mGraph) {
      if (node.value == a) {
        nodeToRemoveA = &node;
      }
      else if (node.value == b) {
        nodeToRemoveB = &node;
      }
    }

    if (nodeToRemoveA == nullptr || nodeToRemoveB == nullptr) {
      return false;
    }

    GraphEdge* edgeToRemoveA = nullptr;
    GraphEdge* edgeToRemoveB = nullptr;

    for (GraphEdge& edge : nodeToRemoveA->adjacentList) {
      if (edge.v->value == b) {
        edgeToRemoveA = &edge;
        break;
      }
    }

    for (GraphEdge& edge : nodeToRemoveB->adjacentList) {
      if (edge.v->value == a) {
        edgeToRemoveB = &edge;
        break;
      }
    }

    if (edgeToRemoveA == nullptr || edgeToRemoveB == nullptr) {
      return false;
    }
    else {
      return nodeToRemoveA->adjacentList.Remove(*edgeToRemoveA) &&
        nodeToRemoveB->adjacentList.Remove(*edgeToRemoveB);
    }
  }

  List<T*> MinimumDistance(const T& start, const T& end) {
    if (mGraph.Size() == 0) {
      List<T*> path;
      return path;
    }

    return Dijkstra(start, end);
  }

  Iterator begin() const {
    return mGraph.begin();
  }

  Iterator end() const {
    return mGraph.end();
  }

  private:
  List<GraphNode> mGraph;

  List<T*> Dijkstra(const T& start, const T& end) {
    Heap<GraphNode*, MinFunctor> minHeap;

    GraphNode* startingNode = nullptr;

    for (GraphNode& node : mGraph) {
      if (node.value == start) {
        node.dijkstraNode = new DijkstraNode(true, 0);
        minHeap.Add(&node);
        startingNode = &node;
      }
      else {
        node.dijkstraNode = new DijkstraNode(false, max_size_t);
      }
    }

    if (startingNode == nullptr) {
      List<T*> path;
      return path;
    }

    GraphNode* shortestPath = nullptr;

    while (minHeap.Size() > 0) {
      GraphNode* nextNode = *minHeap.Peek();
      nextNode->dijkstraNode->visited = true;
      minHeap.Pop();

      if (nextNode->value == end) {
        // We mark the end node as visited and proceed to check remaining paths.
        shortestPath = nextNode;
        continue;
      }
      
      for (GraphEdge& edge : nextNode->adjacentList) {
        if (edge.v->dijkstraNode->visited) {
          continue;
        }

        size_t nextWeight = nextNode->dijkstraNode->distance + edge.weight;
        if (nextWeight < edge.v->dijkstraNode->distance) {
          edge.v->dijkstraNode->distance = nextWeight;
          edge.v->dijkstraNode->previous = nextNode;
        }

        minHeap.Add(edge.v);
      }
    }

    List<T*> path;
    if (shortestPath != nullptr) {
      for (GraphNode* current = shortestPath; current != nullptr; current = current->dijkstraNode->previous) {
        path.Add(&(current->value));
      }
    }

    for (GraphNode& node : mGraph) {
      if (node.dijkstraNode != nullptr) {
        delete node.dijkstraNode;
        node.dijkstraNode = nullptr;
      }
    }

    return path;
  }
};

}
