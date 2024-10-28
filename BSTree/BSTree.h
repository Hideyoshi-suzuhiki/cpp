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

		//whileѭ����Ϊ����cur�ܵ���Ŀ��ڵ�,ͬʱ��parent����cur��ǰһλ,����cur��������ֵ��ʱ��,��parentȥ����cur
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

		//�����ô��Ĳ�����cur��ֵ
		cur = new Node(key, value);

		//������õ�֮ǰ��parent,��parent��right/leftָ��cur
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
			else //��ɾ
			{
				//���ֻ��0-1�����ӵĻ�
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
				else //�������ӵ����
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
	dict.Insert("insert", "����");
	dict.Insert("erase", "ɾ��");
	dict.Insert("left", "���");
	dict.Insert("string", "�ַ���");


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
			cout << "����ƴд����" << endl;
		}
	}


	string strs[] = { "ƻ��", "����", "ƻ��", "ӣ��", "ƻ��", "ӣ��", "ƻ��", "ӣ��", "ƻ��" };
	// ͳ��ˮ�����ֵĴ�
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