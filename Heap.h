#pragma once

#include "ZBaseTypes.h"

#include <initializer_list>

namespace ZSharp {

// Min/Max heap based on the compare functor given.
template<typename T, typename Compare>
class Heap final {
  private:
  struct HeapNode {
    T value;
    size_t height = 0;
    HeapNode* left = nullptr;
    HeapNode* right = nullptr;
    HeapNode* parent = nullptr;

    HeapNode() {}

    HeapNode(const T& inValue, HeapNode* inParent, size_t inHeight) : value(inValue), parent(inParent), height(inHeight) {}

    HeapNode(const HeapNode& rhs) = delete;

    bool HasLeft() const {
      return left != nullptr;
    }

    bool HasRight() const {
      return right != nullptr;
    }

    size_t NumChildren() const {
      size_t count = HasLeft() ? 1 : 0;
      if (HasRight()) {
        ++count;
      }

      return count;
    }

    HeapNode* Sibling() {
      if (parent != nullptr) {
        if (parent->left == this) {
          return parent->right;
        }
        else {
          return parent->left;
        }
      }
      else {
        return nullptr;
      }
    }
  };
  
  public:

  Heap() {}

  Heap(std::initializer_list<T> initList) {
    for (const T& item : initList) {
      Add(item);
    }
  }

  Heap(const Heap& rhs) = delete;
  void operator=(const Heap& rhs) = delete;

  ~Heap() {
    DeleteTree(mRoot);
  }

  bool Add(const T& item) {
    if (mRoot == nullptr) {
      mRoot = new HeapNode(item, nullptr, 0);
      ++mSize;
      return true;
    }

    HeapNode* insertionPoint = InsertionPoint(mRoot, &item);
    if (insertionPoint == nullptr) {
      // Contains duplicate value.
      return false;
    }

    HeapNode* nodeToInsert = nullptr;
    if (insertionPoint->left == nullptr) {
      nodeToInsert = new HeapNode(item, insertionPoint, insertionPoint->height + 1);
      insertionPoint->left = nodeToInsert;
    }
    else if (insertionPoint->right == nullptr) {
      nodeToInsert = new HeapNode(item, insertionPoint, insertionPoint->height + 1);
      insertionPoint->right = nodeToInsert;
    }
    else {
      nodeToInsert = new HeapNode(item, insertionPoint->left, insertionPoint->left->height + 1);
      insertionPoint->left->left = nodeToInsert;
    }

    ++mSize;

    Heapify(nodeToInsert->parent);

    return true;
  }

  bool Pop() {
    if (mRoot == nullptr) {
      return false;
    }
    else if (mSize == 1) {
      delete mRoot;
      mRoot = nullptr;
      mSize = 0;
      return true;
    }

    HeapNode* insertionPoint = InsertionPoint(mRoot, nullptr);
    if (insertionPoint->NumChildren() == 0) {
      HeapNode* sibling = insertionPoint->Sibling();
      HeapNode* siblingLeaf = sibling->right;
      if (siblingLeaf == nullptr) {
        // Perfect binary tree, we must walk to the other side.
        // Since we fill it out left to right, we can walk up and then to the right.
        HeapNode* distantNode = insertionPoint;
        for (size_t i = 0; i < insertionPoint->height; ++i) {
          distantNode = distantNode->parent;
        }

        for (size_t i = 0; i < insertionPoint->height; ++i) {
          distantNode = distantNode->right;
        }

        HeapNode* distantParent = distantNode->parent;
        bool isRoot = distantParent == mRoot;
        SwapRoot(mRoot, distantNode);
        if (isRoot) {
          Heapify(mRoot);
        }
        else {
          Heapify(distantParent);
        }
      }
      else {
        // Delete leaf node of sibling.
        SwapRoot(mRoot, siblingLeaf);
        Heapify(sibling);
      }
    }
    else {
      HeapNode* leftChild = insertionPoint->left;
      bool isRoot = insertionPoint == mRoot;
      SwapRoot(mRoot, leftChild);
      if (!isRoot) {
        Heapify(insertionPoint);
      }
    }

    return true;
  }

  T* Peek() const {
    if (mRoot == nullptr) {
      return nullptr;
    }
    else {
      return &mRoot->value;
    }
  }

  size_t Size() const {
    return mSize;
  }

  private:
  HeapNode* mRoot = nullptr;
  Compare mComparer;
  size_t mSize = 0;

  static void SwapNodes(HeapNode* parent, HeapNode* node) {
    /* 
      We go through all this trouble to avoid doing a couple things:
        1) Allocating node values separately.
        2) Performing extra copies on those values stored in each node.
      Not ideal if we want to work on a small number of fixed items (i.e. array)
      This does make it easy to handle arbitrary input though.
    */
    HeapNode* parentLeft = parent->left;
    HeapNode* parentRight = parent->right;
    HeapNode* grandparent = parent->parent;

    HeapNode* nodeLeft = node->left;
    HeapNode* nodeRight = node->right;

    bool leftChild = parent->left == node;

    bool leftGrandparent = false;
    bool rightGrandparent = false;

    if (grandparent != nullptr) {
      leftGrandparent = grandparent->left == parent;
      rightGrandparent = grandparent->right == parent;
    }

    node->parent = grandparent;

    if (leftChild) {
      node->left = parent;
      node->right = parentRight;
      if (parentRight != nullptr) {
        parentRight->parent = node;
      }
    }
    else {
      node->left = parentLeft;
      node->right = parent;
      if (parentLeft != nullptr) {
        parentLeft->parent = node;
      }
    }

    parent->parent = node;
    parent->left = nodeLeft;
    parent->right = nodeRight;

    if (nodeLeft != nullptr) {
      nodeLeft->parent = parent;
    }

    if (nodeRight != nullptr) {
      nodeRight->parent = parent;
    }

    const size_t parentHeight = parent->height;
    parent->height = node->height;
    node->height = parentHeight;

    if (leftGrandparent) {
      grandparent->left = node;
    }
    else if (rightGrandparent) {
      grandparent->right = node;
    }
  }

  void SwapRoot(HeapNode* root, HeapNode* node) {
    // When there's only a few nodes left, this can cause cycles.
    HeapNode* rootLeft = (root->left == node) ? nullptr : root->left;
    HeapNode* rootRight = (root->right == node) ? nullptr : root->right;

    HeapNode* nodeParent = node->parent;

    if (nodeParent->left == node) {
      nodeParent->left = nullptr;
    }
    else {
      nodeParent->right = nullptr;
    }

    node->parent = nullptr;
    node->left = rootLeft;
    node->right = rootRight;

    if (rootLeft != nullptr) {
      rootLeft->parent = node;
    }

    if (rootRight != nullptr) {
      rootRight->parent = node;
    }

    node->height = 0;

    delete root;
    --mSize;

    mRoot = node;
  }

  void DeleteTree(HeapNode* node) {
    if (node == nullptr) {
      return;
    }

    HeapNode* left = node->left;
    HeapNode* right = node->right;

    delete node;

    DeleteTree(left);
    DeleteTree(right);
  }

  void Heapify(HeapNode* node) {
    // Check and swap left side, then right, then move up the tree.
    if (node->HasLeft()) {
      if (mComparer(node->value, node->left->value)) {
        SwapNodes(node, node->left);
      }
    }

    if (node->HasRight()) {
      if (mComparer(node->value, node->right->value)) {
        SwapNodes(node, node->right);
      }
    }

    if (node->parent == nullptr) {
      mRoot = node;
    }
    else {
      Heapify(node->parent);
    }
  }

  HeapNode* InsertionPoint(HeapNode* node, const T* valuePtr) {
    // Check for duplicates.
    if (valuePtr != nullptr) {
      if ((*valuePtr) == node->value) {
        return nullptr;
      }
    }

    if (node->NumChildren() == 2) {
      HeapNode* leftNode = InsertionPoint(node->left, valuePtr);
      HeapNode* rightNode = InsertionPoint(node->right, valuePtr);

      if (leftNode == nullptr || rightNode == nullptr) {
        return nullptr;
      }

      if (leftNode->NumChildren() < 2) {
        if (leftNode->height > rightNode->height) {
          return rightNode;
        }
        else {
          return leftNode;
        }
      }
      else if (rightNode->NumChildren() < 2) {
        if (leftNode->height < rightNode->height) {
          return leftNode;
        }
        else {
          return rightNode;
        }
      }
      else {
        if (leftNode->height > rightNode->height) {
          return rightNode;
        }
        else {
          return leftNode;
        }
      }
    }
    else {
      return node;
    }
  }
};

}
