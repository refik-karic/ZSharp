#pragma once


#include "ZBaseTypes.h"
#include "PlatformMemory.h"

#include "Array.h"
#include "HashTable.h"
#include "Pair.h"
#include "ZString.h"

namespace ZSharp {

class Trie {
  private:

  struct TrieNode;

  struct TrieHash : public Hash<char> {
    uint32 operator()(const char key) const {
      return key;
    }
  };

  typedef HashTable<char, TrieNode*, TrieHash> TrieStorage;

  struct TrieNode {
    TrieStorage children;
    TrieNode* parent = nullptr;
    uint32 isWord : 1;
    uint32 length : 31;

    TrieNode() : children(255), parent(nullptr), isWord(0), length(0) {

    }

    TrieNode(const TrieNode& rhs) : children(rhs.children), parent(rhs.parent), 
      isWord(rhs.isWord), length(rhs.length) {

    }

    ~TrieNode() {
      FreeNodes();
    }

    void operator=(const TrieNode& rhs) {
      if (this != &rhs) {
        FreeNodes();
        children = rhs.children;
        isWord = rhs.isWord;
        parent = rhs.parent;
      }
    }

    bool operator==(const TrieNode& rhs) const {
      return parent == rhs.parent;
    }

    private:
    void FreeNodes() {
      for (Pair<char, TrieNode*>& child : children) {
        if (child.mValue != nullptr) {
          delete child.mValue;
          child.mValue = nullptr;
        }
      }
    }
  };

  public:

  class Iterator {
    public:
    Iterator(TrieNode* node, TrieNode* root) : mNode(node), mRoot(root) {}

    Iterator& operator++() {
      mNode = GetNext(mNode);
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

    String operator*() const {
      if (mNode == nullptr) {
        return {};
      }

      const size_t wordLength = mNode->length;
      char* buff = (char*)PlatformMalloc(wordLength);
      size_t buffIndex = wordLength - 1;

      for (TrieNode* node = mNode; node != nullptr; node = node->parent) {
        TrieNode* parent = node->parent;
        if (parent != nullptr) {
          for (Pair<char, TrieNode*>& child : parent->children) {
            if (child.mValue == node) {
              buff[buffIndex] = child.mKey;
              buffIndex--;
              break;
            }
          }
        }
      }

      String result(buff, 0, wordLength);
      PlatformFree(buff);
      return result;
    }

    private:

    TrieNode* GetNext(TrieNode* node) {
      if (node == mRoot) {
        return nullptr;
      }

      if (node->children.Size() > 0) {
        TrieNode* child = node->children.begin()->mValue;
        if (child->isWord) {
          return child;
        }
        else {
          return GetNext(child);
        }
      }
      else {
        TrieNode* parent = node->parent;
        if (parent == nullptr) {
          return nullptr;
        }

        return NextSuccessor(parent, node);
      }
    }

    TrieNode* NextSuccessor(TrieNode* parent, TrieNode* node) {
      if (node == nullptr || parent == nullptr || parent == mRoot) {
        return mRoot;
      }

      for (TrieStorage::Iterator iter = parent->children.begin(), end = parent->children.end();
        iter != end; iter++) {
        if (iter->mValue == node) {
          iter++;
          if (iter != end) {
            if (iter->mValue->isWord) {
              return iter->mValue;
            }
            else {
              return GetNext(iter->mValue);
            }
          }
          else {
            break;
          }
        }
      }

      return NextSuccessor(parent->parent, parent);
    }

    TrieNode* mNode;
    TrieNode* mRoot;
  };

  Trie() {
  }

  ~Trie() {
  }

  bool Add(const String& str) {
    const char* inStr = str.Str();
    size_t inLength = str.Length();
    size_t currentIndex = 0;

    bool result = false;

    for (TrieNode* current = &mHead; currentIndex < inLength; ++currentIndex) {
      TrieStorage& node = current->children;
      char letter = inStr[currentIndex];

      if (node.HasKey(letter)) {
        current = node[letter];
      }
      else {
        TrieNode* nextNode = new TrieNode;
        nextNode->parent = current;
        node.Add(letter, nextNode);
        current = node[letter];
      }

      if (currentIndex + 1 >= inLength) {
        current->isWord = 1;
        current->length = inLength;
        result = true;
      }
    }

    return result;
  }

  Pair<Iterator, Iterator> NextWords(const String& str) {
    const char* inStr = str.Str();
    size_t inLength = str.Length();
    size_t currentIndex = 0;

    Pair<Iterator, Iterator> iters(end(), end());

    for (TrieNode* current = &mHead; currentIndex < inLength; ++currentIndex) {
      TrieStorage& node = current->children;
      char letter = inStr[currentIndex];

      if (node.HasKey(letter)) {
        current = node[letter];

        bool reachedEnd = currentIndex + 1 >= inLength;
        if (reachedEnd) {
          if (current->children.Size() > 0) {
            iters.mKey = Iterator(current, current->parent);
            iters.mValue = Iterator(current->parent, current->parent);
            iters.mKey++;
            break;
          }
        }
      }
      else {
        break;
      }
    }

    return iters;
  }

  bool Remove(const String& str) {
    const char* inStr = str.Str();
    size_t inLength = str.Length();
    size_t currentIndex = 0;

    bool result = false;

    for (TrieNode* current = &mHead; currentIndex < inLength; ++currentIndex) {
      TrieStorage& node = current->children;
      char letter = inStr[currentIndex];

      if (node.HasKey(letter)) {
        current = node[letter];

        bool reachedEnd = currentIndex + 1 >= inLength;
        if (reachedEnd) {
          if (current->children.Size() > 0) {
            if (current->isWord) {
              current->isWord = false;
              result = true;
            }

            break;
          }
          else {
            DeleteUpwards(current, current->parent);
            result = true;
          }
        }
      }
      else {
        break;
      }
    }

    return result;
  }

  Iterator begin() {
    Iterator iter(&mHead, mHead.parent);
    iter++;
    return iter;
  }

  Iterator end() {
    Iterator iter(mHead.parent, mHead.parent);
    return iter;
  }

  private:
  TrieNode mHead;

  void DeleteUpwards(TrieNode* node, TrieNode* parent) {
    if (node == nullptr || parent == nullptr) {
      return;
    }

    char* keyToDelete = nullptr;
    for (TrieStorage::Iterator iter = parent->children.begin(), end = parent->children.end();
      iter != end; iter++) {
      if (iter->mValue == node) {
        keyToDelete = &(iter->mKey);
        break;
      }
    }

    if (keyToDelete == nullptr) {
      return;
    }

    parent->children.Remove(*keyToDelete);

    if (parent->children.Size() == 0) {
      DeleteUpwards(parent, parent->parent);
    }
  }
};

}

