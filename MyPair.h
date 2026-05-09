#include <cstddef>
#include <utility>

template <typename, typename> class MyPair;

template <size_t N, typename Pair>
struct Mypair_element_type;

template <typename T1, typename T2>
struct Mypair_element_type<0, MyPair<T1, T2>> {
    using type = T1;
};

template <typename T1, typename T2>
struct Mypair_element_type<1, MyPair<T1, T2>> {
    using type = T2;
};


template <typename T1, typename T2>
class MyPair {
public:
    MyPair(const T1& first, const T2& second) : first_(first), second_(second) {}
    template<typename U1, typename U2,
         typename = std::enable_if_t<
             std::is_constructible_v<T1, U1&&> && std::is_constructible_v<T2, U2&&>
         >>
    MyPair(U1&& u1, U2&& u2)
        : first_(std::forward<U1>(u1)), second_(std::forward<U2>(u2)) {}

    T1 getFirst() const { return first_; }
    T2 getSecond() const { return second_; }

    template <size_t  N>
    typename Mypair_element_type<N, MyPair>::type& get() {
        if constexpr (N == 0) {
            return first_;
        } else if constexpr (N == 1) {
            return second_;
        }
    }

    template <size_t  N>
    const typename Mypair_element_type<N, MyPair>::type& get() const {
        if constexpr (N == 0) {
            return first_;
        } else if constexpr (N == 1) {
            return second_;
        }
    }

private:
    T1 first_;
    T2 second_;
};


namespace std {
    // 告诉编译器大小
    template <typename T1, typename T2>
    struct tuple_size<MyPair<T1, T2>> : integral_constant<size_t, 2> {};

    // 告诉编译器每个位置的类型
    template <std::size_t I, typename T1, typename T2>
    struct tuple_element<I, MyPair<T1, T2>> {
        using type = typename Mypair_element_type<I, MyPair<T1, T2>>::type;
    };
}