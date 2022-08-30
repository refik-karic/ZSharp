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
  };

  public:
  Tree() {

  }

  Tree(const Tree& rhs) {

  }

  Tree(const Tree&&) = delete;

  ~Tree() {
    DeleteTree(mRoot);
  }

  void Add(const T& item) {
    // Perform normal binary tree insertion.
    TreeNode* node = InsertNode(item);
    if (node == nullptr) {
      return; // Value already exists in the tree.
    }

    ++mSize;

    Recolor(node);
  }

  void Remove() {

  }

  size_t Size() const {
    return mSize;
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
        if (*(insertionNode->value) > item) {
          // Larger values on the right
          if (insertionNode->left == nullptr) {
            insertionNode->left = ConstructNode(insertionNode, item);
            return insertionNode->left;
          }
          else {
            insertionNode = insertionNode->left;
          }
        }
        else {
          // Smaller values on the left
          if (insertionNode->right == nullptr) {
            insertionNode->right = ConstructNode(insertionNode, item);
            return insertionNode->right;
          }
          else {
            insertionNode = insertionNode->right;
          }
        }
      }
    }

    return nullptr;
  }

  void Recolor(TreeNode* node) {
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

    TreeNode* uncle = (grandparent->right == parent) ? grandparent->left : grandparent->right;
    if ((uncle != nullptr) && (uncle->color == TreeNode::NodeColor::RED)) {
      parent->color = TreeNode::NodeColor::BLACK;
      uncle->color = TreeNode::NodeColor::BLACK;
      grandparent->color = TreeNode::NodeColor::RED;
      Recolor(grandparent);
    }
    else {
      Rebalance(node);
    }
  }

  void Rebalance(TreeNode* node) {
    TreeNode* parent = node->parent;
    TreeNode* grandparent = parent->parent;
    
    if (grandparent->left == parent) {
      if (parent->left == node) {
        LLRotation(node);
      }
      else {
        LRRotation(node);
      }
    }
    else {
      if (parent->right == node) {
        RRRotation(node);
      }
      else {
        RLRotation(node);
      }
    }
  }

  void LLRotation(TreeNode* node) {
    TreeNode* parent = node->parent;
    TreeNode* grandparent = parent->parent;
    TreeNode* subtreeParent = grandparent->parent;

    if (subtreeParent != nullptr) {
      if (subtreeParent->left == grandparent) {
        subtreeParent->left = parent;
      }
      else {
        subtreeParent->right = parent;
      }

      parent->parent = subtreeParent;
    }
    else {
      parent->parent = nullptr;
    }

    if (grandparent == mRoot) {
      mRoot = parent;
    }

    TreeNode::NodeColor parentColor = parent->color;
    grandparent->left = parent->right;
    grandparent->parent = parent;
    parent->right = grandparent;
    parent->color = grandparent->color;
    grandparent->color = parentColor;
  }

  void LRRotation(TreeNode* node) {
    // Swap to left.
    TreeNode* parent = node->parent;
    TreeNode* grandparent = parent->parent;
    TreeNode* leftInsertNode = node->left;
    grandparent->left = node;
    node->left = parent;
    node->parent = grandparent;
    parent->right = leftInsertNode;
    parent->parent = node;

    LLRotation(parent);
  }

  void RRRotation(TreeNode* node) {
    TreeNode* parent = node->parent;
    TreeNode* grandparent = parent->parent;
    TreeNode* subtreeParent = grandparent->parent;

    if (subtreeParent != nullptr) {
      if (subtreeParent->left == grandparent) {
        subtreeParent->left = parent;
      }
      else {
        subtreeParent->right = parent;
      }

      parent->parent = subtreeParent;
    }
    else {
      parent->parent = nullptr;
    }

    if (grandparent == mRoot) {
      mRoot = parent;
    }

    TreeNode::NodeColor parentColor = parent->color;
    grandparent->right = parent->left;
    grandparent->parent = parent;
    parent->left = grandparent;
    parent->color = grandparent->color;
    grandparent->color = parentColor;
  }

  void RLRotation(TreeNode* node) {
    // Swap to right.
    TreeNode* parent = node->parent;
    TreeNode* grandparent = parent->parent;
    TreeNode* rightInsertNode = node->right;
    grandparent->right = node;
    node->right = parent;
    node->parent = grandparent;
    parent->left = rightInsertNode;
    parent->parent = node;

    RRRotation(parent);
  }
};

}
