#pragma once

#include "ZBaseTypes.h"
#include "ZAssert.h"
#include "PlatformMemory.h"

namespace ZSharp {

template<typename T>
class Tree {
  private:
  struct TreeNode {
    T* value = nullptr;
    TreeNode* parent = nullptr;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
    enum class NodeColor {
      RED,
      BLACK
    } color = NodeColor::RED;

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
      ZAssert(parent != nullptr);
      return (parent != nullptr) ? this == parent->left : false;
    }

    void SwapValues(TreeNode* node) {
      T* oldValue = value;
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
  Tree() = default;

  Tree(const Tree& rhs) = delete;

  Tree(const Tree&&) = delete;

  ~Tree() {
    DeleteTree(mRoot);
  }

  bool HasItem(const T& item) {
    return SearchNode(mRoot, item) != nullptr;
  }

  bool Add(const T& item) {
    // Perform normal binary tree insertion.
    TreeNode* node = InsertNode(item);
    if (node == nullptr) {
      return false;
    }

    ++mSize;
    InsertionRecolor(node);
    return true;
  }

  bool Remove(const T& item) {
    bool isDoubleBlack = false;
    TreeNode* replacedNode = RemoveNode(item, mRoot, isDoubleBlack);

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

  private:
  TreeNode* mRoot = nullptr;
  size_t mSize = 0;

  TreeNode* ConstructNode(TreeNode* parent, const T& item) {
    TreeNode* node = new TreeNode;
    node->value = new T(item);
    node->parent = parent;
    return node;
  }

  void DeleteNode(TreeNode* node) {
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

  TreeNode* InsertNode(const T& item) {
    if (mRoot == nullptr) {
      mRoot = ConstructNode(mRoot, item);
      return mRoot;
    }
    else {
      for (TreeNode* insertionNode = mRoot; insertionNode != nullptr;) {
        const T& matchedValue = *(insertionNode->value);
        if (matchedValue > item) {
          // Larger values on the right
          if (insertionNode->left == nullptr) {
            insertionNode->left = ConstructNode(insertionNode, item);
            return insertionNode->left;
          }
          else {
            insertionNode = insertionNode->left;
          }
        }
        else if (matchedValue < item) {
          // Smaller values on the left
          if (insertionNode->right == nullptr) {
            insertionNode->right = ConstructNode(insertionNode, item);
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

  TreeNode* RemoveNode(const T& item, TreeNode* node, bool& isDoubleBlack) {
    if (node == nullptr) {
      return node;
    }

    TreeNode* matchedNode = SearchNode(node, item);
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

        DeleteNode(matchedNode);
        if (matchedNode->IsLeftNode()) {
          parent->left = nullptr;
        }
        else {
          parent->right = nullptr;
        }

        matchedNode = parent;
      }
    }
    else if (leftChild == nullptr) {
      rightChild->SwapValues(matchedNode);
      matchedNode->color = TreeNode::NodeColor::BLACK;
      DeleteNode(matchedNode->right);
      matchedNode->right = nullptr;
    }
    else if (rightChild == nullptr) {
      leftChild->SwapValues(matchedNode);
      matchedNode->color = TreeNode::NodeColor::BLACK;
      DeleteNode(matchedNode->left);
      matchedNode->left = nullptr;
    }
    else {
      TreeNode* minRight = MinSuccessor(matchedNode->right);
      *(matchedNode->value) = *(minRight->value);
      return RemoveNode(*(minRight->value), matchedNode->right, isDoubleBlack);
    }

    return matchedNode;
  }

  TreeNode* MinSuccessor(TreeNode* node) const {
    TreeNode* minNode = node;

    for (TreeNode* currentNode = node; currentNode != nullptr;) {
      if (*(currentNode->value) < *(minNode->value)) {
        minNode = currentNode;
      }

      currentNode = currentNode->left;
    }

    return minNode;
  }

  TreeNode* SearchNode(TreeNode* startingNode, const T& item) const {
    for (TreeNode* matchingNode = startingNode; matchingNode != nullptr;) {
      const T& matchingNodeValue = *(matchingNode->value);
      if (matchingNodeValue == item) {
        return matchingNode;
      }
      else if (matchingNodeValue > item) {
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
    ZAssert(parent != nullptr);

    if (parent->color == TreeNode::NodeColor::BLACK) {
      return;
    }

    TreeNode* grandparent = parent->parent;
    if (grandparent == nullptr) {
      return;
    }

    TreeNode* uncle = node->GetUncle();
    if ((uncle != nullptr) && (uncle->color == TreeNode::NodeColor::RED)) {
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
    if (node->parent->IsLeftNode()) {
      if (node->IsLeftNode()) {
        LLRotation(node->parent);
      }
      else {
        LRRotation(node->parent);
        LLRotation(node->parent);
      }
    }
    else {
      if (!node->IsLeftNode()) {
        RRRotation(node->parent);
      }
      else {
        RLRotation(node->parent);
        RRRotation(node->parent);
      }
    }
  }

  void ResolveDoubleBlack(TreeNode* node) {
    TreeNode* leftSibling = node->left;
    TreeNode* rightSibling = node->right;
    TreeNode* parent = node->parent;

    if (node->color == TreeNode::NodeColor::BLACK) {
      if (node->HasRedChild()) {
        // Left cases.
        if (node->IsLeftNode()) {
          if (leftSibling == nullptr || leftSibling->color == TreeNode::NodeColor::BLACK) {
            // LR case
            LRRotation(node);
          }

          // LL case
          LLRotation(node);
          node->color = TreeNode::NodeColor::BLACK;
          parent->left->color = TreeNode::NodeColor::BLACK;
          if (parent->left->left != nullptr) {
            parent->left->left->color = TreeNode::NodeColor::BLACK;
          }
        }
        else {
          if (rightSibling == nullptr || rightSibling->color == TreeNode::NodeColor::BLACK) {
            // RL case.
            RLRotation(node);
          }

          // RR case
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
    node->SwapValues(parent);
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
    node->SwapValues(parent);
    parent->left = node;
    parent->right = oldRight;
  }

  void LRRotation(TreeNode* node) {
    node->right->SwapValues(node);
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
    node->left->SwapValues(node);
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

  bool IsRoot(TreeNode* node) const {
    return node == mRoot;
  }

  bool ValidBlackHeight(TreeNode* node, size_t expectedHeight) const {
    if (node != nullptr && !IsRoot(node)) {
      bool isLeftNode = node->parent->left == node;
      bool isRightNode = node->parent->right == node;
      ZAssert(isLeftNode || isRightNode);
    }

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
