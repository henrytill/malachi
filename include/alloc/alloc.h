#pragma once

#include <stddef.h>

///
/// Allocates or dies.
///
/// @param size The size in bytes to allocate.
/// @return A pointer to the allocated memory.
///
void *emalloc(size_t size);

///
/// Allocates and zeroes or dies.
///
/// @param nmemb The number of elements to allocate.
/// @param size The size in bytes of each element.
/// @return A pointer to the allocated memory.
///
void *ecalloc(size_t nmemb, size_t size);
