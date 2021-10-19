namespace Container
{
	template<class T>
	class IContainer
	{
		public:
			virtual ~IContainer() {}

			virtual u32 Size() = 0;
			virtual bool IsEmpty() = 0;
			virtual void Clear() = 0;

			virtual void PushFront(const T& v) = 0;
			virtual void PushBack(const T& v) = 0;
			virtual T PopFront() = 0;
			virtual T PopBack() = 0;
	};
}
