#include <iostream>
#include <vector>
#include <xmmintrin.h> // For SSE intrinsics
#include <chrono>

#ifdef _WIN32
static int check_align(size_t align)
{
    for (size_t i = sizeof(void *); i != 0; i *= 2)
    if (align == i)
        return 0;
    return EINVAL;
}

int posix_memalign(void **ptr, size_t align, size_t size)
{
    if (check_align(align))
        return EINVAL;

    int saved_errno = errno;
    void *p = _aligned_malloc(size, align);
    if (p == NULL)
    {
        errno = saved_errno;
        return ENOMEM;
    }

    *ptr = p;
    return 0;
}
#endif

float* aligned_allocate(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, 16, size * sizeof(float)) != 0) {
        throw std::bad_alloc();
    }
    return static_cast<float*>(ptr);
}

void add_vectors_simd(const float* a, const float* b, float* c, size_t size) {
    size_t i = 0;
    // Process 4 elements at a time
    for (; i + 4 <= size; i += 4) {
        __m128 vec_a = _mm_load_ps(&a[i]);
        __m128 vec_b = _mm_load_ps(&b[i]);
        __m128 vec_result = _mm_add_ps(vec_a, vec_b);
        _mm_store_ps(&c[i], vec_result);
    }
    // Process remaining elements
    for (; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

void add_vectors_normal(const float* a, const float* b, float* c, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

int main() {
    size_t size = 1000000;

    float* a = aligned_allocate(size);
    float* b = aligned_allocate(size);
    float* result_simd = aligned_allocate(size);
    float* result_normal = aligned_allocate(size);
    
    // Initialize vectors
    for (size_t i = 0; i < size; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(size - i);
    }

    // Time normal vector addition
    auto start_normal = std::chrono::high_resolution_clock::now();
    add_vectors_normal(a, b, result_normal, size);
    auto end_normal = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_normal = end_normal - start_normal;

    // Time SIMD vector addition
    auto start_simd = std::chrono::high_resolution_clock::now();
    add_vectors_simd(a, b, result_simd, size);
    auto end_simd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_simd = end_simd - start_simd;

    // Verify results
    for (size_t i = 0; i < size; ++i) {
        if (result_normal[i] != result_simd[i]) {
            std::cerr << "Results do not match at index " << i << std::endl;
            break;
        }
    }

    std::cout << "Normal vector addition time: " << duration_normal.count() << " s" << std::endl;
    std::cout << "SIMD vector addition time: " << duration_simd.count() << " s" << std::endl;
    std::cout << "SIMD/Normal ratio: " << duration_simd.count() / duration_normal.count() << std::endl;

#ifdef _WIN32
    _aligned_free(a);
    _aligned_free(b);
    _aligned_free(result_simd);
    _aligned_free(result_normal);
#else
    free(a);
    free(b);
    free(result_simd);
    free(result_normal);
#endif

    return 0;
}

// cd SIMD; g++ vector_add.cpp -o bin/vector_add -std=c++17; ./bin/vector_add