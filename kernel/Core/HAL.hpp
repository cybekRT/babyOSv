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

	class RegisterBase
	{
		protected:
			u16 port;

			RegisterBase(u16 port) : port(port) {}
	};

	template<class T>
	class RegisterRO : public RegisterBase
	{
		protected:
			//T value;

		public:
			RegisterRO() {}

			RegisterRO(u16 port) : RegisterBase(port)
			{

			}

			T Read()
			{
				if constexpr (sizeof(T) == 1)
				{
					//*(u8*)&value = In8(port);
					u8 v = In8(port);

					return *(T*)&v;
					//return reinterpret_cast<T>(In8(port));
				}
				else if constexpr (sizeof(T) == 2)
					return (T)In16(port);
				else
					//static_assert(dependent_false<T>::value, "Invalid register size");
					ASSERT(false, "Invalid register size");
			}
	};

	template<class T>
	class RegisterWO : public RegisterBase
	{
		public:
			RegisterWO() {}

			RegisterWO(u16 port) : RegisterBase(port)
			{

			}

			void Write(T value)
			{
				if constexpr (sizeof(T) == 1)
				{
					/*Print("Vptr: %x\n", &value);
					Print("V   : %x\n", value);
					Print("V2  : %x\n", *(&value));
					Print("V3  : %x\n", *(u8*)&value);
					Print("Port: %x\n", port);*/
					Out8(port, *(u8*)&value);
				}
				else if constexpr (sizeof(T) == 2)
					Out16(port, *(u16*)&value);
				else
					//static_assert(dependent_false<T>::value, "Invalid register size");
					ASSERT(false, "Invalid register size");
			}
	};

	template<class T>
	class RegisterRW : public RegisterBase
	{
		public:
			RegisterRW(u16 port) : RegisterBase(port)
			{

			}

			inline T Read()
			{
				//return (T)In8(port);
				if constexpr (sizeof(T) == 1)
				{
					//Print("%d -> 8b\n", port);
					return (T)In8(port);
				}
				else if constexpr (sizeof(T) == 2)
				{
					//Print("%d -> 16b\n", port);
					return (T)In16(port);
				}
				else
				{
					//Print("%d -> inv\n", port);
					//static_assert(dependent_false<T>::value, "Invalid register size");
					ASSERT(false, "Invalid register size");
				}
			}

			inline void Write(T value)
			{
				if constexpr (sizeof(T) == 1)
					Out8(port, *(u8*)&value);
				else if constexpr (sizeof(T) == 2)
					Out16(port, *(u16*)&value);
				else
					//static_assert(dependent_false<T>::value, "Invalid register size");
					ASSERT(false, "Invalid register size");
			}
	};
}
