#include <cstddef>
#include <type_traits>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cassert>
#include <new>
#include <memory>

template <typename T>
class MySmallVectorImpl {
protected:
    T* BeginX;
    T* EndX;
    T* CapacityX;
    T* SmallBegin;
    size_t SmallCapacity;

    MySmallVectorImpl(void* Buffer, size_t N) :
        BeginX(reinterpret_cast<T*>(Buffer)),
        EndX(BeginX),
        CapacityX(BeginX + N),
        SmallBegin(BeginX),
        SmallCapacity(N) {}

    virtual ~MySmallVectorImpl() {
        destroy_all();
    }
    
    MySmallVectorImpl(const MySmallVectorImpl&) = delete;
    MySmallVectorImpl(MySmallVectorImpl&&) = delete;

public:

    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() {return BeginX; }
    iterator end() {return EndX; }

    const_iterator begin() const {return BeginX; }
    const_iterator end() const {return EndX; }

    size_t size() const {
        return EndX - BeginX;
    }

    size_t capacity() const {
        return CapacityX - BeginX;
    }

    bool isSmall() const {
        return BeginX == SmallBegin;
    }

    void destroy_range(iterator First, iterator Last) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (auto It = First; It != Last; ++It) {
                It->~T();
            }
        }
    }

    void destroy_all() {
        destroy_range(BeginX, EndX);
        if (!isSmall()) {
            free(BeginX);
        }
    }

    void clear() {
        destroy_range(BeginX, EndX);
        EndX = BeginX;
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

    MySmallVectorImpl<T>& operator=(const MySmallVectorImpl<T>& RHS) {
        if (this == &RHS)
            return *this;

        size_t CurSize = size();
        size_t RHSSize = RHS.size();

        if (CurSize >= RHS.size()) {
            std::copy(RHS.begin(), RHS.end(), begin());
            destroy_range(begin() + RHSSize, end());
            this->EndX = BeginX + RHSSize;
            return *this;
        } 
        
        if (capacity() >= RHSSize) {
            this->clear();
            std::uninitialized_copy(RHS.begin(), RHS.end(), begin());
            this->EndX = BeginX + RHSSize;
            return *this;
        } else {
            clear();
            EndX = BeginX; // 确保 EndX 在 grow 前正确指向小缓冲区
            grow(RHSSize);
            std::uninitialized_copy(RHS.begin(), RHS.end(), begin());
            this->EndX = BeginX + RHSSize;
        }

        return *this;
    }

    MySmallVectorImpl<T>& operator=(MySmallVectorImpl<T>&& RHS) {
        if (this == &RHS)
            return *this;

        size_t CurSize = size();
        size_t RHSSize = RHS.size();

        if (!RHS.isSmall()) {
            destroy_all();
            BeginX = RHS.BeginX;
            EndX = RHS.EndX;
            CapacityX = RHS.CapacityX;

            // 将 RHS 重置为小缓冲区状态
            RHS.BeginX = RHS.SmallBegin;
            RHS.EndX = RHS.SmallBegin;
            RHS.CapacityX = RHS.SmallBegin + RHS.SmallCapacity;
            return *this;
        }

        if (CurSize > RHSSize) {
            std::move(RHS.begin(), RHS.end(), begin());
            destroy_range(begin() + RHSSize, end());
            EndX = BeginX + RHSSize;
            RHS.clear();
            return *this;
        }

        if (CurSize == RHSSize) {
            std::move(RHS.begin(), RHS.end(), begin());
            EndX = BeginX + RHSSize;
            RHS.clear();
            return *this;
        }

        // CurSize < RHSSize
        if (capacity() < RHSSize) {
            clear();
            CurSize = 0;
            grow(RHSSize);
        } else if (CurSize > 0) {
            std::move(RHS.begin(), RHS.begin() + CurSize, begin());
        }

        std::uninitialized_move(RHS.begin() + CurSize, RHS.end(), begin() + CurSize);
        EndX = BeginX + RHSSize;
        RHS.clear();

        return *this;
    }
};

template <typename T, size_t N>
class MySmallVector : public MySmallVectorImpl<T> {
    alignas(T) unsigned char InlineBuffer[N * sizeof(T)];

    using SuperClass = MySmallVectorImpl<T>;
public:
    MySmallVector() : MySmallVectorImpl<T>(InlineBuffer, N) {}
    MySmallVector(size_t Count) : MySmallVectorImpl<T>(InlineBuffer, N) {
        if (Count > N) {
            this->grow(Count);
        }
        std::uninitialized_default_construct_n(this->BeginX, Count);
        this->EndX = this->BeginX + Count;
    }

    MySmallVector(size_t Count, const T& Value) : MySmallVectorImpl<T>(InlineBuffer, N) {
        if (Count > N) {
            this->grow(Count);
        }
        std::uninitialized_fill_n(this->BeginX, Count, Value);
        this->EndX = this->BeginX + Count;
    }

    MySmallVector(const MySmallVector& RHS) : MySmallVectorImpl<T>(InlineBuffer, N) {
        SuperClass::operator=(RHS);
    }

    MySmallVector(MySmallVector&& RHS) : MySmallVectorImpl<T>(InlineBuffer, N) {
        SuperClass::operator=(std::move(RHS));
    }
};
