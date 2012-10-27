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
		 aligned_allocator() {}
		 aligned_allocator& operator=(const aligned_allocator &rhs)
		 {
			 std::allocator<T>::operator=(rhs);
			 return *this;
		 }

		 pointer allocate(size_type n, allocator<void>::const_pointer *hint)
		 {
			 size_type count = sizeof(T) * n;
			 if ( hint != nullptr )
			 {
				 return reinterpret_cast<pointer>(aligned_malloc(count,Alignment));
			 }
			 else
			 {
				 return reinterpret_cast<pointer>(aligned_realloc(const_cast<void*>(hint),count,Alignment));
			 }
		 }

		 void deallocate(pointer p, size_type n)
		 {
			 aligned_free(p);
		 }

		 void construct(pointer p, const T &val)
		 {
			 new(p) T(val);
		 }

		 void destroy(pointer p)
		 {
			 p->~T();
		 }
	};

};
#endif