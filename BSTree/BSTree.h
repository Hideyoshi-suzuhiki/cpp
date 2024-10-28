#pragma once
#include<string>
#include<iostream>
using namespace std;
template<class K, class V>
class BSTreeNode
{
public:
	K _key;
	V _value;
	BSTreeNode<K, V>* _left;
	BSTreeNode<K, V>* _right;

	BSTreeNode(const K& key, const V& value)
		:_key(key), _value(value), _left(nullptr), _right(nullptr) {}
};
template<class K, class V>
class BSTree
{
	typedef BSTreeNode<K, V> Node;
public:
	bool Insert(const K& key, const V& value)
	{
		if (_root == nullptr)
		{
			_root = new Node(key, value);
			return true;
		}

		Node* cur = _root;
		Node* parent = nullptr;

		//while循环是为了让cur能到达目标节点,同时让parent到达cur的前一位,方便cur被赋予新值的时候,让parent去连接cur
		while (cur)
		{
			if (cur->_key < key)
			{
				parent = cur;
				cur = cur->_right;
			}
			else if (cur->_key > key)
			{
				parent = cur;
				cur = cur->_left;
			}
			else
			{
				return false;
			}
		}

		//这里用传的参数给cur赋值
		cur = new Node(key, value);

		//这里就用到之前的parent,让parent的right/left指向cur
		if (parent->_key > key)
		{
			parent->_left = cur;
		}
		else
		{
			parent->_right = cur;
		}
		return true;
	}
	Node* Find(const K& key)
	{
		if (_root == nullptr)
		{
			return _root;
		}

		Node* cur = _root;
		while (cur)
		{
			if (cur->_key > key)
			{
				cur = cur->_left;
			}
			else if (cur->_key < key)
			{
				cur = cur->_right;
			}
			else
			{
				return cur;
			}
		}

		return nullptr;
	}
	bool Erase(const K& key)
	{
		if (_root == nullptr)
		{
			return false;
		}

		Node* parent = nullptr;
		Node* cur = _root;

		while (cur)
		{
			if (cur->_key > key)
			{
				parent = cur;
				cur = cur->_left;
			}
			else if (cur->_key < key)
			{
				parent = cur;
				cur = cur->_right;
			}
			else //开删
			{
				//如果只有0-1个孩子的话
				if (cur->_left == nullptr)
				{
					if (parent == nullptr)
					{
						_root = cur->_right;
					}
					else
					{
						if (parent->_left == cur)
						{
							cur = cur->_right;
							parent->_left = cur;
						}
						else
						{
							cur = cur->_right;
							parent->_right = cur;
						}
					}

					delete cur;
					return true;
				}
				else if (cur->_right == nullptr)
				{
					if (parent == nullptr)
					{
						_root = cur->_left;
					}
					else
					{
						if (parent->_left == cur)
						{
							cur = cur->_left;
							parent->_left = cur;
						}
						else
						{
							cur = cur->_left;
							parent->_right = cur;
						}
					}

					delete cur;
					return true;
				}
				else //两个孩子的情况
				{
					Node* leftmaxParent = cur;
					Node* leftmax = cur->left;

					while (leftmax->_right)
					{
						leftmaxParent = leftmax;
						leftmax = leftmax->_right;
					}

					cur->_key = leftmax->_key;
					cur->_value = leftmax->_value;

					if (leftmaxParent->_right == leftmax)
					{
						leftmaxParent->_right = leftmax->_left;
					}
					else
					{
						leftmaxParent->_left = leftmax->_left;
					}

					delete leftmax;
					return true;
				}
			}
		}

		return false;

	}
	void _InOrder(Node* root)
	{
		if (root == nullptr)
			return;

		_InOrder(root->_left);
		cout << root->_key << ":" << root->_value << endl;
		_InOrder(root->_right);
	}
	void InOrder()
	{
		_InOrder(_root);
		cout << endl;
	}
private:
	Node* _root = nullptr;
};


void TestBSTree()
{
	BSTree<string, string> dict;
	dict.Insert("insert", "插入");
	dict.Insert("erase", "删除");
	dict.Insert("left", "左边");
	dict.Insert("string", "字符串");


	string str;
	while (cin >> str)
	{
		auto ret = dict.Find(str);
		if (ret)
		{
			cout << str << ":" << ret->_value << endl;
		}
		else
		{
			cout << "单词拼写错误" << endl;
		}
	}


	string strs[] = { "苹果", "西瓜", "苹果", "樱桃", "苹果", "樱桃", "苹果", "樱桃", "苹果" };
	// 统计水果出现的次
	BSTree<string, int> countTree;
	for (auto str : strs)
	{
		auto ret = countTree.Find(str);
		if (ret == NULL)
		{
			countTree.Insert(str, 1);
		}
		else
		{
			ret->_value++;
		}
	}
	countTree.InOrder();
}