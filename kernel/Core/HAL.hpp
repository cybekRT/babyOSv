#pragma once

namespace HAL
{
	inline void WaitForInterrupt() { __asm("hlt"); }

	u8 In8(u16 port);
	void Out8(u16 port, u8 data);

	u16 In16(u16 port);
	void Out16(u16 port, u16 data);

	enum class RegisterType
	{
		RO,
		WO,
		RW
	};

	template<class T>
	class RegisterBase
	{
		protected:
			u16 port;

			RegisterBase(u16 port) : port(port) {}

		public:
			T value;
	};

	template<class T>
	class RegisterRO : public RegisterBase<T>
	{
		public:
			RegisterRO() {}

			RegisterRO(u16 port) : RegisterBase<T>(port)
			{

			}

			T& Read()
			{
				if constexpr (sizeof(T) == 1)
				{
					*(u8*)&this->value = In8(this->port);
					return this->value;
				}
				else if constexpr (sizeof(T) == 2)
				{
					*(u16*)&this->value = In16(this->port);
					return this->value;
				}
				else
					ASSERT(false, "Invalid register size");
			}
	};

	template<class T>
	class RegisterWO : public RegisterBase<T>
	{
		public:
			RegisterWO() {}

			RegisterWO(u16 port) : RegisterBase<T>(port)
			{

			}

			void Write()
			{
				if constexpr (sizeof(T) == 1)
					Out8(this->port, *(u8*)&this->value);
				else if constexpr (sizeof(T) == 2)
					Out16(this->port, *(u16*)&this->value);
				else
					ASSERT(false, "Invalid register size");
			}

			void Write(const T& value)
			{
				this->value = value;
				Write();
			}
	};

	template<class T>
	class RegisterRW : public RegisterBase<T>
	{
		public:
			RegisterRW(u16 port) : RegisterBase<T>(port)
			{

			}

			T& Read()
			{
				if constexpr (sizeof(T) == 1)
				{
					*(u8*)&this->value = In8(this->port);
					return this->value;
				}
				else if constexpr (sizeof(T) == 2)
				{
					*(u16*)&this->value = In16(this->port);
					return this->value;
				}
				else
					ASSERT(false, "Invalid register size");
			}

			void Write()
			{
				if constexpr (sizeof(T) == 1)
					Out8(this->port, *(u8*)&this->value);
				else if constexpr (sizeof(T) == 2)
					Out16(this->port, *(u16*)&this->value);
				else
					ASSERT(false, "Invalid register size");
			}

			void Write(const T& value)
			{
				this->value = value;
				Write();
			}
	};
}
