#pragma once

#include "Ref.h"

#include  <memory>

namespace VoxelicousEngine
{
	void InitializeCore();
	void ShutdownCore();

}

#if !defined(VE_PLATFORM_WINDOWS) && !defined(VE_PLATFORM_LINUX)
	#error Unknown platform.
#endif

#define BIT(x) (1u << (x))

#if defined(__GNUC__)
	#if defined(__clang__)
		#define VE_COMPILER_CLANG
	#else
		#define VE_COMPILER_GCC
	#endif
#elif defined(_MSC_VER)
	#define VE_COMPILER_MSVC
#endif

#define VE_BIND_EVENT_FN(fn) std::bind(&(fn), this, std::placeholders::_1)

#ifdef VE_COMPILER_MSVC
	#define VE_FORCE_INLINE __forceinline
	#define VE_EXPLICIT_STATIC static
#elif defined(__GNUC__)
	#define VE_FORCE_INLINE __attribute__((always_inline)) inline
	#define VE_EXPLICIT_STATIC
#else
	#define VE_FORCE_INLINE inline
	#define VE_EXPLICIT_STATIC
#endif

namespace VoxelicousEngine
{
	// Pointer wrappers
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	using Byte = uint8_t;

	/** A simple wrapper for std::atomic_flag to avoid confusing
		function names usage. The object owning it can still be
		default copyable, but the copied flag is going to be reset.
	*/
	struct AtomicFlag
	{
		VE_FORCE_INLINE void SetDirty() { m_Flag.clear(); }
		VE_FORCE_INLINE bool CheckAndResetIfDirty() { return !m_Flag.test_and_set(); }

		explicit AtomicFlag() noexcept { m_Flag.test_and_set(); }
		AtomicFlag(const AtomicFlag&) noexcept {}
		AtomicFlag& operator=(const AtomicFlag&) noexcept { return *this; }
		AtomicFlag(AtomicFlag&&) noexcept {}
		AtomicFlag& operator=(AtomicFlag&&) noexcept { return *this; }

	private:
		std::atomic_flag m_Flag;
	};

	struct Flag
	{
		VE_FORCE_INLINE void SetDirty() noexcept { m_Flag = true; }
		VE_FORCE_INLINE bool CheckAndResetIfDirty() noexcept
		{
			if (m_Flag)
			{
				m_Flag = false;
				return true;
			}
			
			return false;
		}

		VE_FORCE_INLINE bool IsDirty() const noexcept { return m_Flag; }

	private:
		bool m_Flag = false;
	};

}
