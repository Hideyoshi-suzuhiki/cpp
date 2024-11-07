#pragma once
#include<iostream>
#include<assert.h>
#include <cstdlib>
#include <ctime>
using namespace std;


template<class K, class V>
struct AVLTreeNode
{
	pair<K, V> _kv;
	AVLTreeNode<K, V>* _left;
	AVLTreeNode<K, V>* _right;
	AVLTreeNode<K, V>* _parent;
	int _bf; // balance factor

	AVLTreeNode(const pair<K, V>& kv)
		:_kv(kv)
		, _left(nullptr)
		, _right(nullptr)
		, _parent(nullptr)
		, _bf(0)
	{}
};

template<class K, class V>
class AVLTree
{
	typedef AVLTreeNode<K, V> Node;
public:
	AVLTree() = default;

	AVLTree(const AVLTree<K, V>& t)
	{
		_root = Copy(t._root);
	}

	AVLTree<K, V>& operator=(AVLTree<K, V> t)
	{
		swap(_root, t._root);
		return *this;
	}

	~AVLTree()
	{
		Destroy(_root);
		_root = nullptr;
	}

	bool Insert(const pair<K, V>& kv)
	{
		if (_root == nullptr)
		{
			_root = new Node(kv);
			return true;
		}

		Node* parent = nullptr;
		Node* cur = _root;
		while (cur)
		{
			if (cur->_kv.first < kv.first)
			{
				parent = cur;
				cur = cur->_right;
			}
			else if (cur->_kv.first > kv.first)
			{
				parent = cur;
				cur = cur->_left;
			}
			else
			{
				return false;
			}
		}

		cur = new Node(kv);
		if (parent->_kv.first < kv.first)
		{
			parent->_right = cur;
		}
		else
		{
			parent->_left = cur;
		}
		cur->_parent = parent;

		// 更新平衡因子
		while (parent)
		{
			if (cur == parent->_left)
				parent->_bf--;
			else
				parent->_bf++;

			if (parent->_bf == 0)
			{
				break;
			}
			else if (parent->_bf == 1 || parent->_bf == -1)
			{
				// 继续往上更新
				cur = parent;
				parent = parent->_parent;
			}
			else if (parent->_bf == 2 || parent->_bf == -2)
			{
				if(parent->_bf == 2 && cur->_bf == 1)
				{
					RotateL(parent);
				}
				else if (parent->_bf == -2 && cur->_bf == -1)
				{
					RotateR(parent);
				}
				else if (parent->_bf == 2 && cur->_bf == -1)
				{
					RotateRL(parent);
				}
				else
				{
					RotateLR(parent);
				}
				break;
			}
			else
			{
				assert(false);
			}
		}

		return true;
	}

	Node* Find(const K& key)
	{
		Node* cur = _root;
		while (cur)
		{
			if (cur->_kv.first < key)
			{
				cur = cur->_right;
			}
			else if (cur->_kv.first > key)
			{
				cur = cur->_left;
			}
			else
			{
				return cur;
			}
		}

		return nullptr;
	}

	void InOrder()
	{
		_InOrder(_root);
		cout << endl;
	}

	int Height()
	{
		return _Height(_root);
	}

	bool IsBalanceTree()
	{
		return _IsBalanceTree(_root);
	}

private:
	void _InOrder(Node* root)
	{
		if (root == nullptr)
		{
			return;
		}

		_InOrder(root->_left);
		cout << root->_kv.first << ":" << root->_kv.second << endl;
		_InOrder(root->_right);
	}

	void RotateL(Node* parent)
	{
		// 注:这个sub是子树的意思
		// parent此时为1节点
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		parent->_right = subRL;
		if (subRL)
			subRL->_parent = parent;
		subR->_left = parent;
		Node* grand = parent->_parent;
		parent->_parent = subR;
		if (grand == nullptr)
		{
			_root = subR;
			subR->_parent = nullptr;
		}
		else
		{
			if (grand->_left == parent)
				grand->_left = subR;
			else
				grand->_right = subR;
			subR->_parent = grand;
		}
		parent->_bf = 0;
		subR->_bf = 0;
	}

	void RotateR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;

		Node* Parentparent = parent->_parent;
		subL->_right = parent;
		parent->_parent = subL;
		if(subLR)
			subLR->_parent = parent;
		parent->_left = subLR;
		if (Parentparent)
		{
			if (Parentparent->_left == parent)
				Parentparent->_left = subL;
			else
				Parentparent->_right = subL;
			subL->_parent = Parentparent;
		}
		else
		{
			_root = subL;
			subL->_parent = nullptr;
		}

		subL->_bf = 0;
		parent->_bf = 0;
	}

	void RotateRL(Node* parent)
	{
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		int bf = subRL->_bf;
		RotateR(parent->_right);
		RotateL(parent);

		if (bf == 0)
		{
			parent->_bf = 0;
			subR->_bf = 0;
			subRL->_bf = 0;
		}
		else if (bf == -1)
		{
			parent->_bf = 0;
			subR->_bf = 1;
			subRL->_bf = 0;
		}
		else if (bf == 1)
		{
			parent->_bf = -1;
			subR->_bf = 0;
			subRL->_bf = 0;
		}
		else
		{
			assert(false);
		}
	}

	void RotateLR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		int bf = subLR->_bf;
		RotateL(parent->_left);
		RotateR(parent);

		if (bf == 0)
		{
			parent->_bf = 0;
			subL->_bf = 0;
			subLR->_bf = 0;
		}
		else if (bf == -1)
		{
			parent->_bf = 1;
			subL->_bf = 0;
			subLR->_bf = 0;
		}
		else if (bf == 1)
		{
			parent->_bf = 0;
			subL->_bf = -1;
			subLR->_bf = 0;
		}
		else
		{
			assert(false);
		}
	}

	void Destroy(Node* root)
	{
		if (root == nullptr)
			return;

		Destroy(root->_left);
		Destroy(root->_right);
		delete root;
	}

	Node* Copy(Node* root)
	{
		if (root == nullptr)
			return nullptr;

		Node* newRoot = new Node(root->_kv);
		newRoot->_left = Copy(root->_left);
		newRoot->_right = Copy(root->_right);

		return newRoot;
	}


private:
	Node* _root = nullptr;

	int _Height(Node* root)
	{
		if (root == nullptr)
			return 0;

		int HeightL = _Height(root->_left);
		int HeightR = _Height(root->_right);
		
		return HeightL > HeightR ? HeightL + 1 : HeightR + 1;
	}
	bool _IsBalanceTree(Node* root)
	{
		if (nullptr == root) 
			return true;

		int leftHeight = _Height(root->_left);
		int rightHeight = _Height(root->_right);
		int diff = rightHeight - leftHeight;

		if (diff != root->_bf || (diff > 1 || diff < -1))
			return false;
		return _IsBalanceTree(root->_left) && _IsBalanceTree(root->_right);
	}
};

void TestAVLTree()
{
	AVLTree<int, int> t;
	srand(time(nullptr));
	for (int i = 0; i < 1000; ++i)
	{
		int num = rand() % 10000 + 1;
		t.Insert({ num, num });
	}

	t.InOrder();
	if (t.IsBalanceTree())
		cout << "是平衡二叉树" << endl;
	else
		cout << "不是平衡二叉树" << endl;
	cout << "高度为" << t.Height();
}