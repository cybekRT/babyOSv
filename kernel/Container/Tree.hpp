#include"Memory.h"
#include"LinkedList.h"

template<class T>
class TreeNode
{
	public:
		T							value;
		TreeNode<T>*				parent;
		LinkedList<TreeNode<T>*>	children;

		void Clear()
		{
			
		}
};

template<class T>
class Tree
{
	public:
		TreeNode<T>	root;

		void Add(TreeNode<T>& parent, const T& value)
		{
			auto node = Memory::Alloc<TreeNode<T>>();
			node.value = value;
			node.parent = parent;
			
			parent.children.PushBack(node);
		}

		void Remove(TreeNode<T>& node)
		{

		}
};
