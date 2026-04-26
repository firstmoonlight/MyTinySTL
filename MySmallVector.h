#include <cstddef>
#include <type_traits>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cassert>


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
        destroy_all();
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

    void destroy_all() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (T* It = BeginX; It != EndX; ++It) {
                It->~T();
            }
        }

        if (!isSmall()) {
            free(BeginX);
        }
    }

    void destroy_range(T* First, T* Last) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (T* It = First; It != Last; ++It) {
                It->~T();
            }
        }
    }

    void grow(size_t MinSize) {
        const size_t OldSize = size();
        const size_t OldCapacity = capacity();

        // 容量增长策略：至少满足 MinSize，通常按 2 倍扩
        size_t NewCapacity = std::max(OldCapacity ? OldCapacity * 2 : size_t(1), MinSize);
        T* NewBuffer = static_cast<T*>(std::malloc(NewCapacity * sizeof(T)));
        if (!NewBuffer) {
            throw std::bad_alloc();
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(NewBuffer, BeginX, OldSize * sizeof(T));
        } else {
            size_t Constructed = 0;
            try {
                for (; Constructed < OldSize; ++Constructed) {
                    new (NewBuffer + Constructed) T(std::move(BeginX[Constructed]));
                }
            } catch (...) {
                for (size_t i = 0; i < Constructed; ++i) {
                    (NewBuffer + i)->~T();
                }
                std::free(NewBuffer);
                throw;
            }

            // 旧缓冲区元素已经成功搬完，才销毁旧对象
            destroy_range(BeginX, EndX);
        }

        if (!isSmall()) {
            std::free(BeginX);
        }

        BeginX = NewBuffer;
        EndX = NewBuffer + OldSize;
        CapacityX = NewBuffer + NewCapacity;
    }

    void push_back(const T& Value) {
        if (size() == capacity()) {
            T Temp(Value);            // 先复制，避免 Value 引用旧缓冲区时失效
            grow(size() + 1);         // 表达真实最小需求
            new (EndX) T(std::move(Temp));
        } else {
            new (EndX) T(Value);
        }
        ++EndX;
    }

    T& operator[](size_t Index) {
        assert(Index < size());
        return BeginX[Index];
    }

    const T& operator[](size_t Index) const {
        assert(Index < size());
        return BeginX[Index];
    }
};

template <typename T, size_t N>
class MySmallVector : public MySmallVectorImpl<T> {
    alignas(T) unsigned char InlineBuffer[N * sizeof(T)];

public:
    MySmallVector() : MySmallVectorImpl<T>(InlineBuffer, N) {}
};
