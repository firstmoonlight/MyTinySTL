
#include <type_traits>
#include <cstddef>
#include <utility>

template <typename... Args> class MyTuple;

template <size_t N, typename Tuple>
struct Mytuple_element_type;

template <typename T, typename... Rest>
struct Mytuple_element_type<0, MyTuple<T, Rest...>> {
    using type = T;
};

template <size_t N, typename T, typename... Rest>
struct Mytuple_element_type<N, MyTuple<T, Rest...>> {
    using type = typename Mytuple_element_type<N - 1, MyTuple<Rest...>>::type;
};

template <typename T> 
class MyTuple<T> {
public:
    using type = T;

    MyTuple(const T& value) : value_(value) {}
    template<typename U,
         typename = std::enable_if_t<
             std::is_constructible_v<T, U&&>
         >>
    MyTuple(U&& u)
        : value_(std::forward<U>(u)) {}

    MyTuple() = default;
    MyTuple(const MyTuple&) = default;
    MyTuple(MyTuple&&) = default;

    T value_;
};

template <typename T, typename... Rest>
class MyTuple<T, Rest...> : public MyTuple<Rest...> {
public:
    using Parent = MyTuple<Rest...>;
    using type = T;

    MyTuple(const T& value, const Rest&... rest) : MyTuple<Rest...>(rest...), value_(value) {}
    template<typename U, typename... URest,
         typename = std::enable_if_t<
             std::is_constructible_v<T, U&&> && (std::is_constructible_v<Rest, URest&&> && ...)
         >>
    MyTuple(U&& u, URest&&... urest)
        : MyTuple<Rest...>(std::forward<URest>(urest)...), value_(std::forward<U>(u)) {}

    T value_;
};

template <size_t N, typename T, typename... Rest>
typename Mytuple_element_type<N, MyTuple<T, Rest...>>::type& get(MyTuple<T, Rest...>& tuple) {
    if constexpr (N == 0) {
        return tuple.value_;
    } else {
        return get<N - 1>(static_cast<MyTuple<Rest...>&>(tuple));   
    }
}


template <size_t N, typename T, typename... Rest>
const typename Mytuple_element_type<N, MyTuple<T, Rest...>>::type& get(const MyTuple<T, Rest...>& tuple) {
    if constexpr (N == 0) {
        return tuple.value_;
    } else {
        return get<N - 1>(static_cast<const MyTuple<Rest...>&>(tuple));   
    }
}

template <size_t N, typename T, typename... Rest>
typename Mytuple_element_type<N, MyTuple<T, Rest...>>::type&& get(MyTuple<T, Rest...>&& tuple) {
    if constexpr (N == 0) {
        return std::move(tuple.value_);
    } else {
        return get<N - 1>(static_cast<MyTuple<Rest...>&&>(tuple));   
    }
}

namespace std {
    // 告诉编译器大小
    template <typename... Args>
    struct tuple_size<MyTuple<Args...>> : integral_constant<size_t, sizeof...(Args)> {};

    // 告诉编译器类型
    template <size_t N, typename... Args>
    struct tuple_element<N, MyTuple<Args...>> {
        using type = typename Mytuple_element_type<N, MyTuple<Args...>>::type;
    };
}
