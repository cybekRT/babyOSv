template<class T>
class OneWayIterator
{
	public:
		virtual T& operator*()=0;

		//virtual const OneWayIterator<T>&& operator+(int) const =0;
		virtual OneWayIterator<T>& operator++()=0;
		//virtual const OneWayIterator<T>&& operator++(int)=0;
		virtual OneWayIterator<T>& operator+=(int)=0;

		virtual bool operator==(const OneWayIterator<T> &)=0;
		virtual bool operator!=(const OneWayIterator<T> &)=0;
};

template<class T>
class TwoWayIterator : public OneWayIterator<T>
{
	public:
		//virtual const TwoWayIterator<T>&& operator-(int) const =0;
		virtual TwoWayIterator<T>& operator--()=0;
		//virtual const TwoWayIterator<T>&& operator--(int)=0;
		virtual TwoWayIterator<T>& operator-=(int)=0;
};
