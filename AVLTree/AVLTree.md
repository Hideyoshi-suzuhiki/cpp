# AVL树的底层实现

## AVL树

AVL树是一种自平衡的二叉搜索树，它通过旋转操作来保持树的平衡。AVL树的每个节点都有一个平衡因子，平衡因子是左子树的高度减去右子树的高度。AVL树的平衡因子可以是-1、0或1。如果平衡因子的绝对值大于1，则树是不平衡的，需要进行旋转操作来恢复平衡。

## AVL树除了旋转部分的代码实现

### AVL树除了旋转的部分,与二叉搜索树基本相同,所以在这里不再赘述
    #include<iostream>
    #include<assert.h>
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


            return true;
        }

        Node* Find(const K& key)
        {
            Node* cur = _root;
            while (cur)
            {
                if (cur->_key < key)
                {
                    cur = cur->_right;
                }
                else if (cur->_key > key)
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

            Node* newRoot = new Node(root->_key, root->_value);
            newRoot->_left = Copy(root->_left);
            newRoot->_right = Copy(root->_right);

            return newRoot;
        }

    private:
        Node* _root = nullptr;
    };


## 平衡因子

### 这里用bf(balance factor)来表示平衡因子,这里我们用右子树的高度减去左子树的高度来表示平衡因子,所以就有了下面这段代码

    if (cur == parent->_left)
        parent->_bf--;
    else
        parent->_bf++;

在插入节点的时候,如果插入的节点是左子树,那么父节点的平衡因子就减一,如果插入的节点是右子树,那么父节点的平衡因子就加一,可是,我们不能只判断子节点的父节点,如图所示:

这是一个满二叉树,所有节点的bf都是0:

![alt text](image.png)

现在插入一个节点9:

![alt text](image-1.png)

可以看到,节点8的左子树(nullptr)与右子树(9)的高度差为1,可是这里又要引入新的问题,7的左子树(6)和右子树(8)的高度差也是1,同理5的左子树和右子树高度差也是1,这里我们就要通过回溯来更新所有节点的平衡因子

    if (parent->_bf == 1 || parent->_bf == -1)
    {
        // 往上更新
        cur = parent;
        parent = parent->_parent;
    }

目的就是让这棵树上的所有父辈节点的平衡因子都更新到正确的值

通过while循环重复执行这段代码

    if (cur == parent->_left)
        parent->_bf--;
    else
        parent->_bf++;

例如这个:

![alt text](image-2.png)

此时8为parent,9为cur,所以8->bf++,然后向上更新,让cur指向8,parent指向7,最终让所有节点都更新到正确的bf值

## 问题来了!
### 8的左边在插入一个节点呢?此时8的bf应该变为0,而别的不变,所以当一个parent的bf为0的时候,直接break,不再更新祖辈节点,这样就解决了这个问题
所以就有了这段代码:

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
            // TODO:旋转
        }
        else
        {
            assert(false);
        }
    }

所以前半段代码如下:

    #include<iostream>
    #include<assert.h>
    using namespace std;
    template<class K, class V>
    // 为节省篇幅先省略结构体定义

    template<class K, class V>
    class AVLTree
    {
        typedef AVLTreeNode<K, V> Node;
    public:

        // 为节省篇幅先省略构造析构

        bool Insert(const pair<K, V>& kv)
        {
            // 为节省篇幅先省略插入部分

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
                    // TODO:旋转
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
            // 为节省篇幅先省略
        }

        void InOrder()
        {
            _InOrder(_root);
            cout << endl;
        }


    private:
        void _InOrder(Node* root)
        {
            // 为节省篇幅先省略
        }

        void Destroy(Node* root)
        {
            // 为节省篇幅先省略
        }

        Node* Copy(Node* root)
        {
            // 为节省篇幅先省略
        }

    private:
        Node* _root = nullptr;
    };

# AVL树的失衡和旋转
## 失衡
### 什么时候需要AVL树进行旋转呢?当平衡因子的绝对值大于1就说明已经失衡,需要进行旋转
#### 失衡分为四种情况:分别是LL,RR,LR,RL,第一个字母表示失衡节点的左子树或右子树,第二个字母表示最后插入的节点是在失衡节点的子树(这个子树方向根据第一个字母)的左子树还是右子树

### RL:如图所示,我们用圆形来表示节点,长方形来表示子树

![alt text](image-3.png)

#### 这就是常见的RL失衡,这个时候在中间的长方形下面插入新的节点就会导致失衡,其他的失衡情况也是类似的,只是插入的节点位置不同,在这里我们只需要考虑RL失衡的情况,其他的失衡情况可以类推

## 旋转
### 既然因为插入操作导致失衡,那么我们就可以通过旋转来恢复平衡

#### 这里我们使用RR失衡来演示:

![alt text](image-6.png)

(为了方便标了个号)
#### 我们可以观察到,RR失衡是因为在C子树插入了3节点导致的,那么我们可以通过将1作为失衡节点,这个时候我们进行旋转,把2作为新的根,然后让2的左指向1,1的右指向B

### 成果图应该是这样的:

![alt text](image-7.png)

#### 判断一下,这个时候树仍然是二叉搜索树,而且平衡因子也是符合要求,说明已经恢复平衡了

### 代码实现:

一开始我是这么想的:让2->left=1,1->right=B,再让2当根节点,于是我尝试实现了代码

    void RotateL(Node* parent)
	{
        // 注:这个sub是子树的意思
        // parent此时为1节点
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		parent->_right = subRL;
        subR->_left = parent;
	}

但是这个只是让图示的关系成立了,但是此时B的parent还是指向2,所以我们要改过来,于是代码就变成了这样:

    void RotateL(Node* parent)
	{
        // 注:这个sub是子树的意思
        // parent此时为1节点
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		parent->_right = subRL;
        subRL->_parent = parent;
        subR->_left = parent;
	}

然后又又要考虑到,这个subRL有没有可能不存在呢,要加个判断:

    if(subRL)
        subRL->_parent = parent;

然后因为parent有了新的父节点,所以又要加上代码:

    parent->_parent = subR;

然后又又又考虑到,我们的1节点也只是一个子树,所以要让1节点的父节点指向新的根节点,于是代码又要加上:

    Node* grand = parent->_parent;
    if(grand == nullptr)
    {
        _root = subR;
        subR->_parent = nullptr;
    }
    else
    {
        if(grand->_left == parent)
            grand->_left = subR;
        else
            grand->_right = subR;
        subR->_parent = grand;
    }

最后把平衡因子改成0,就完成了左旋操作

    parent->_bf = 0;
    subR->_bf = 0;

### 最后附上完整代码:

    void RotateL(Node* parent)
	{
        // 注:这个sub是子树的意思
        // parent此时为1节点
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		parent->_right = subRL;
        if(subRL)
            subRL->_parent = parent;
        subR->_left = parent;
        Node* grand = parent->_parent;
        parent->_parent = subR;
        if(grand == nullptr)
        {
            _root = subR;
            subR->_parent = nullptr;
        }
        else
        {
            if(grand->_left == parent)
                grand->_left = subR;
            else
                grand->_right = subR;
            subR->_parent = grand;
        }
        parent->_bf = 0;
        subR->_bf = 0;
	}

至此,我们写完了左旋的代码,其他的旋转操作同理,只是旋转方向不同而已,不做赘述

# AVL树的完整代码:
