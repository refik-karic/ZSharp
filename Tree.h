#pragma once

#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "Pair.h"
#include "PlatformMemory.h"

namespace ZSharp {

template<typename Key, typename Value>
class Tree {
  private:
  struct TreeNode {
    Key* key;
    Value* value;
    TreeNode* parent;
    TreeNode* left;
    TreeNode* right;
    enum class NodeColor : uint8 {
      RED,
      BLACK
    } color;

    TreeNode(Key* insertionKey, Value* insertionValue, TreeNode* nodeParent)
      : key(insertionKey),
        value(insertionValue),
        parent(nodeParent),
        left(nullptr),
        right(nullptr),
        color(NodeColor::RED) {

    }

    TreeNode* GetSibling() const {
      if (parent == nullptr) {
        return nullptr;
      }
      else {
        return (parent->left == this) ? parent->right : parent->left;
      }
    }

    TreeNode* GetUncle() const {
      if (parent == nullptr) {
        return nullptr;
      }
      else {
        return parent->GetSibling();
      }
    }

    bool HasRedChild() const {
      bool hasRedChild = false;
      if (left != nullptr) {
        hasRedChild = left->color == TreeNode::NodeColor::RED;
      }

      if (!hasRedChild && right != nullptr) {
        hasRedChild = right->color == TreeNode::NodeColor::RED;
      }

      return hasRedChild;
    }

    bool IsLeftNode() const {
      return (parent != nullptr) ? this == parent->left : false;
    }

    bool IsRightNode() const {
      return (parent != nullptr) ? this == parent->right : false;
    }

    void SwapKeyValue(TreeNode* node) {
      Key* oldKey = key;
      Value* oldValue = value;
      key = node->key;
      value = node->value;
      node->key = oldKey;
      node->value = oldValue;
    }

    void SwapValue(TreeNode* node) {
      Key* oldValue = value;
      value = node->value;
      node->value = oldValue;
    }

    void SwapColors(TreeNode* node) {
      NodeColor oldColor = color;
      color = node->color;
      node->color = oldColor;
    }
  };

  public:

  class Iterator {
  public:
    Iterator(TreeNode* node) : mNode(node) {}

    Iterator& operator++() {
      TreeNode* successor = MinSuccessor(mNode->right);
      if (successor != nullptr) {
        mNode = successor;
      }
      else {
        while (mNode->IsRightNode()) {
          mNode = mNode->parent;
        }

        mNode = mNode->parent;
      }

      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mNode == rhs.mNode;
    }

    bool operator!=(const Iterator& rhs) {
      return mNode != rhs.mNode;
    }

    Value& operator*() const {
      return *(mNode->value);
    }

    Value* operator->() {
      return mNode->value;
    }

  private:
    TreeNode* mNode;

    friend class Tree;
  };

  Tree() = default;

  Tree(const Tree& rhs) {
    for (Iterator iter = rhs.begin(); iter != rhs.end(); ++iter) {
      Key& key = *(iter.mNode->key);
      Value& value = *(iter.mNode->value);
      Add(key, value);
    }
  }

  void operator=(const Tree& rhs) {
    if (&rhs == this) {
      return;
    }

    DeleteTree(mRoot);

    for (Iterator iter = rhs.begin(); iter != rhs.end(); ++iter) {
      Key& key = *(iter.mNode->key);
      Value& value = *(iter.mNode->value);
      Add(key, value);
    }
  }

  Tree(Tree&&) = delete;

  ~Tree() {
    DeleteTree(mRoot);
  }

  bool HasKey(const Key& key) const {
    return SearchNode(mRoot, key) != nullptr;
  }

  bool Add(const Key& key, const Value& value) {
    TreeNode* node = InsertNode(key, value);
    if (node == nullptr) {
      return false;
    }

    ++mSize;
    InsertionRecolor(node);
    return true;
  }

  bool Remove(const Key& key) {
    bool isDoubleBlack = false;
    TreeNode* replacedNode = RemoveNode(key, mRoot, isDoubleBlack);
    if (replacedNode == nullptr) {
      return false;
    }

    --mSize;
    if (isDoubleBlack && mSize > 0) {
      // Root can never be a double black.
      if (replacedNode->left == nullptr) {
        ResolveDoubleBlack(replacedNode->right);
      }
      else {
        ResolveDoubleBlack(replacedNode->left);
      }
    }

    return true;
  }

  Value& operator[](const Key& key) {
    TreeNode* node = SearchNode(mRoot, key);
    ZAssert(node != nullptr);
    return *(node->value);
  }

  const Value& operator[](const Key& key) const {
    TreeNode* node = SearchNode(mRoot, key);
    ZAssert(node != nullptr);
    return *(node->value);
  }

  size_t Size() const {
    return mSize;
  }

  bool IsValid() const {
    size_t expectedHeight = 0;
    for (TreeNode* current = mRoot; current != nullptr; current = current->left) {
      if (current->color == TreeNode::NodeColor::BLACK) {
        ++expectedHeight;
      }
    }

    return ValidBlackHeight(mRoot, expectedHeight);
  }

  Iterator begin() const {
    return Iterator(MinSuccessor(mRoot));
  }

  Iterator end() const {
    return Iterator(nullptr);
  }

  private:
  TreeNode* mRoot = nullptr;
  size_t mSize = 0;

  TreeNode* ConstructNode(TreeNode* parent, const Key& key, const Value& value) {
    return new TreeNode(new Key(key), new Value(value), parent);
  }

  void DeleteNode(TreeNode* node) {
    if (node->key != nullptr) {
      delete node->key;
    }

    if (node->value != nullptr) {
      delete node->value;
    }

    delete node;
  }

  void DeleteTree(TreeNode* node) {
    if (node != nullptr) {
      TreeNode* left = node->left;
      TreeNode* right = node->right;
      
      DeleteNode(node);
      
      if (left != nullptr) {
        DeleteTree(left);
      }

      if (right != nullptr) {
        DeleteTree(right);
      }
    }
  }

  TreeNode* InsertNode(const Key& key, const Value& value) {
    if (mRoot == nullptr) {
      mRoot = ConstructNode(mRoot, key, value);
      return mRoot;
    }
    else {
      for (TreeNode* insertionNode = mRoot; insertionNode != nullptr;) {
        const Key& matchedKey = *(insertionNode->key);
        if (matchedKey > key) {
          if (insertionNode->left == nullptr) {
            insertionNode->left = ConstructNode(insertionNode, key, value);
            return insertionNode->left;
          }
          else {
            insertionNode = insertionNode->left;
          }
        }
        else if (matchedKey < key) {
          if (insertionNode->right == nullptr) {
            insertionNode->right = ConstructNode(insertionNode, key, value);
            return insertionNode->right;
          }
          else {
            insertionNode = insertionNode->right;
          }
        }
        else {
          break; // Don't add duplicates.
        }
      }
    }

    return nullptr;
  }

  TreeNode* RemoveNode(const Key& key, TreeNode* node, bool& isDoubleBlack) {
    if (node == nullptr) {
      return node;
    }

    TreeNode* matchedNode = SearchNode(node, key);
    if (matchedNode == nullptr) {
      return nullptr;
    }

    TreeNode* parent = matchedNode->parent;
    TreeNode* leftChild = matchedNode->left;
    TreeNode* rightChild = matchedNode->right;

    if (leftChild == nullptr && rightChild == nullptr) {
      if (IsRoot(matchedNode)) {
        DeleteNode(mRoot);
        mRoot = nullptr;
      }
      else {
        if (matchedNode->color == TreeNode::NodeColor::BLACK) {
          // Black leaf, mark double black deletion.
          isDoubleBlack = true;
        }

        if (matchedNode->IsLeftNode()) {
          parent->left = nullptr;
        }
        else {
          parent->right = nullptr;
        }

        DeleteNode(matchedNode);
        matchedNode = parent;
      }
    }
    else if (leftChild == nullptr) {
      rightChild->SwapKeyValue(matchedNode);
      matchedNode->color = TreeNode::NodeColor::BLACK;
      DeleteNode(matchedNode->right);
      matchedNode->right = nullptr;
    }
    else if (rightChild == nullptr) {
      leftChild->SwapKeyValue(matchedNode);
      matchedNode->color = TreeNode::NodeColor::BLACK;
      DeleteNode(matchedNode->left);
      matchedNode->left = nullptr;
    }
    else {
      TreeNode* minRight = MinSuccessor(matchedNode->right);
      *(matchedNode->key) = *(minRight->key);
      matchedNode->SwapValue(minRight);
      return RemoveNode(*(minRight->key), matchedNode->right, isDoubleBlack);
    }

    return matchedNode;
  }

  static TreeNode* MinSuccessor(TreeNode* node) {
    TreeNode* minNode = node;

    for (TreeNode* currentNode = node; currentNode != nullptr;) {
      if (*(currentNode->key) < *(minNode->key)) {
        minNode = currentNode;
      }

      currentNode = currentNode->left;
    }

    return minNode;
  }

  TreeNode* SearchNode(TreeNode* startingNode, const Key& key) const {
    for (TreeNode* matchingNode = startingNode; matchingNode != nullptr;) {
      const Key& matchingKey = *(matchingNode->key);
      if (matchingKey == key) {
        return matchingNode;
      }
      else if (matchingKey > key) {
        if (matchingNode->left == nullptr) {
          return nullptr;
        }
        else {
          matchingNode = matchingNode->left;
        }
      }
      else {
        if (matchingNode->right == nullptr) {
          return nullptr;
        }
        else {
          matchingNode = matchingNode->right;
        }
      }
    }

    return nullptr;
  }

  void InsertionRecolor(TreeNode* node) {
    if (node == mRoot) {
      node->color = TreeNode::NodeColor::BLACK;
      return;
    }

    TreeNode* parent = node->parent;
    if (parent->color == TreeNode::NodeColor::BLACK) {
      return;
    }

    TreeNode* uncle = node->GetUncle();
    if ((uncle != nullptr) && (uncle->color == TreeNode::NodeColor::RED)) {
      TreeNode* grandparent = parent->parent;
      parent->color = TreeNode::NodeColor::BLACK;
      uncle->color = TreeNode::NodeColor::BLACK;
      grandparent->color = TreeNode::NodeColor::RED;
      InsertionRecolor(grandparent);
    }
    else {
      InsertionRebalance(node);
    }
  }

  void InsertionRebalance(TreeNode* node) {
    TreeNode* parent = node->parent;
    if (parent->IsLeftNode()) {
      if (node->IsLeftNode()) {
        LLRotation(parent);
      }
      else {
        LRRotation(parent);
        LLRotation(parent);
      }
    }
    else {
      if (!node->IsLeftNode()) {
        RRRotation(parent);
      }
      else {
        RLRotation(parent);
        RRRotation(parent);
      }
    }
  }

  void ResolveDoubleBlack(TreeNode* node) {
    TreeNode* leftSibling = node->left;
    TreeNode* rightSibling = node->right;
    TreeNode* parent = node->parent;

    if (node->color == TreeNode::NodeColor::BLACK) {
      if (node->HasRedChild()) {
        if (node->IsLeftNode()) {
          if (leftSibling == nullptr || leftSibling->color == TreeNode::NodeColor::BLACK) {
            LRRotation(node);
          }

          LLRotation(node);
          node->color = TreeNode::NodeColor::BLACK;
          parent->left->color = TreeNode::NodeColor::BLACK;
          if (parent->left->left != nullptr) {
            parent->left->left->color = TreeNode::NodeColor::BLACK;
          }
        }
        else {
          if (rightSibling == nullptr || rightSibling->color == TreeNode::NodeColor::BLACK) {
            RLRotation(node);
          }

          RRRotation(node);
          node->color = TreeNode::NodeColor::BLACK;
          parent->right->color = TreeNode::NodeColor::BLACK;
          if (parent->right->right != nullptr) {
            parent->right->right->color = TreeNode::NodeColor::BLACK;
          }
        }
      }
      else {
        node->color = TreeNode::NodeColor::RED;
        TreeNode* nodeSibling = node->GetSibling();
        if (nodeSibling != nullptr) {
          nodeSibling->color = TreeNode::NodeColor::BLACK;
        }

        if (parent->color == TreeNode::NodeColor::RED) {
          parent->color = TreeNode::NodeColor::BLACK;
        }
        else if (!IsRoot(parent)) {
          ResolveDoubleBlack(parent->GetSibling());
        }
      }
    }
    else {
      // Node's color stays red because it's carried over.
      if (node->IsLeftNode()) {
        LLRotation(node);
        parent->color = TreeNode::NodeColor::BLACK;
        node->color = TreeNode::NodeColor::RED;
        ResolveDoubleBlack(node->left);
      }
      else {
        RRRotation(node);
        parent->color = TreeNode::NodeColor::BLACK;
        node->color = TreeNode::NodeColor::RED;
        ResolveDoubleBlack(node->right);
      }
    }
  }

  void LLRotation(TreeNode* node) {
    TreeNode* parent = node->parent;
    TreeNode* oldRight = parent->right;
    TreeNode* oldLeft = node->left;

    if (oldRight != nullptr) {
      oldRight->parent = node;
    }
    
    if (oldLeft != nullptr) {
      oldLeft->parent = parent;
    }

    node->left = node->right;
    node->right = oldRight;
    node->SwapKeyValue(parent);
    parent->left = oldLeft;
    parent->right = node;
  }

  void RRRotation(TreeNode* node) {
    TreeNode* parent = node->parent;
    TreeNode* oldLeft = parent->left;
    TreeNode* oldRight = node->right;

    if (oldLeft != nullptr) {
      oldLeft->parent = node;
    }

    if (oldRight != nullptr) {
      oldRight->parent = parent;
    }

    node->right = node->left;
    node->left = oldLeft;
    node->SwapKeyValue(parent);
    parent->left = node;
    parent->right = oldRight;
  }

  void LRRotation(TreeNode* node) {
    node->right->SwapKeyValue(node);
    TreeNode* oldRight = node->right;
    TreeNode* oldLeft = node->left;
    if (oldRight->right != nullptr) {
      oldRight->right->parent = node;
    }

    if (oldLeft != nullptr) {
      oldLeft->parent = oldRight;
    }

    node->left = oldRight;
    node->right = oldRight->right;
    oldRight->right = oldRight->left;
    oldRight->left = oldLeft;
  }

  void RLRotation(TreeNode* node) {
    node->left->SwapKeyValue(node);
    TreeNode* oldRight = node->right;
    TreeNode* oldLeft = node->left;
    if (oldLeft->left != nullptr) {
      oldLeft->left->parent = node;
    }

    if (oldRight != nullptr) {
      oldRight->parent = oldLeft;
    }

    node->left = oldLeft->left;
    node->right = oldLeft;
    oldLeft->left = oldLeft->right;
    oldLeft->right = oldRight;
  }

  bool IsRoot(const TreeNode* node) const {
    return node == mRoot;
  }

  bool ValidBlackHeight(const TreeNode* node, size_t expectedHeight) const {
#ifdef _DEBUG
    if (node != nullptr && !IsRoot(node)) {
      bool isLeftNode = node->parent->left == node;
      bool isRightNode = node->parent->right == node;
      ZAssert(isLeftNode || isRightNode);
    }
#endif

    if (node == nullptr) {
      bool isZero = expectedHeight == 0;
      ZAssert(isZero);
      return isZero; // Black leaf.
    }

    size_t blackCount = (node->color == TreeNode::NodeColor::BLACK) ? 1 : 0;
    bool leftResult = ValidBlackHeight(node->left, expectedHeight - blackCount);
    ZAssert(leftResult);
    bool rightResult = ValidBlackHeight(node->right, expectedHeight - blackCount);
    ZAssert(rightResult);
    return leftResult && rightResult;
  }
};

}
