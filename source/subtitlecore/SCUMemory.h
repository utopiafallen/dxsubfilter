// Based off this article: http://www.codeproject.com/Articles/4795/C-Standard-Allocator-An-Introduction-and-Implement

#ifndef SCUMEMORY_H
#define SCUMEMORY_H
#pragma once

#include <memory>

namespace SCU
{
	// Custom allocator for STL structures to allow for alignment.
	template <typename T, size_t Alignment=16>
	class aligned_allocator : public std::allocator<T>
	{
	public:
		//    convert an allocator<T> to allocator<U>
		template<typename U>
		struct rebind {
			typedef aligned_allocator<U> other;
		};

	public:
		 inline explicit aligned_allocator() {}
		 inline ~aligned_allocator() {}
		 inline aligned_allocator(const aligned_allocator&) {}
		 template <typename U>
		 inline explicit aligned_allocator(const aligned_allocator<U>&) {}

		 aligned_allocator& operator=(const aligned_allocator &rhs)
		 {
			 std::allocator<T>::operator=(rhs);
			 return *this;
		 }

		 pointer allocate(size_type n, allocator<void>::const_pointer *hint = nullptr)
		 {
			 size_type count = sizeof(T) * n;
			 if ( hint != nullptr )
			 {
				 return reinterpret_cast<pointer>(_aligned_malloc(count,Alignment));
			 }
			 else
			 {
				 return reinterpret_cast<pointer>(_aligned_realloc(hint,count,Alignment));
			 }
		 }

		 void deallocate(pointer p, size_type n)
		 {
			 UNREFERENCED_PARAMETER(n);
			 _aligned_free(p);
		 }

		 void construct(pointer p, const T &val)
		 {
			 new(p) T(val);
		 }

		 void destroy(pointer p)
		 {
			 UNREFERENCED_PARAMETER(p);
			 p->~T();
		 }
	};

};
#endif