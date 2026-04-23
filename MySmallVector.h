#include <cstddef>
#include <type_traits>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>


template <typename T>
class MySmallVectorImpl {
protected:
    T* BeginX;
    T* EndX;
    T* CapacityX;

    MySmallVectorImpl(void* Buffer, size_t N) :
        BeginX(reinterpret_cast<T*>(Buffer)),
        EndX(BeginX),
        CapacityX(BeginX + N) {}

    virtual ~MySmallVectorImpl() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (T* It = BeginX; It != EndX; ++It) {
                It->~T();
            }
        }

        if (!isSmall()) {
            free(BeginX);
        }
    }
    
    MySmallVectorImpl(const MySmallVectorImpl&) = delete;
    MySmallVectorImpl(MySmallVectorImpl&&) = delete;

public:
    size_t size() const {
        return EndX - BeginX;
    }

    size_t capacity() const {
        return CapacityX - BeginX;
    }

    bool isSmall() const {
        return BeginX == reinterpret_cast<const T*>(this + 1);
    }

    void grow(size_t MinSize) {
        // Implementation of grow logic to allocate new memory and move elements
        size_t NewCapacity = std::max(capacity() * 2, MinSize);
        T* NewBufffer = (T*)malloc(NewCapacity * sizeof(T));
        if (!NewBufffer) {
            throw std::bad_alloc();
        }
        // Move existing elements to the new buffer
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(NewBufffer, BeginX, size() * sizeof(T));
        } else {
            for (size_t i = 0; i < size(); ++i) {
                new (NewBufffer + i) T(std::move(BeginX[i]));
                BeginX[i].~T();
            }
        }

        if (!isSmall()) {
            free(BeginX);
        }
        size_t OldSize = size();
        BeginX = NewBufffer;
        EndX = BeginX + OldSize;
        CapacityX = BeginX + NewCapacity;
    }
};

template <typename T, size_t N>
class MySmallVector : public MySmallVectorImpl<T> {
    alignas(T) unsigned char InlineBuffer[N * sizeof(T)];

public:
    MySmallVector() : MySmallVectorImpl<T>(InlineBuffer, N) {}
};
