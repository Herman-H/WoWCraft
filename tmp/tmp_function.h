#ifndef TMP_FUNCTION_H
#define TMP_FUNCTION_H

#include "tmp_types.h"
#include <utility>

namespace tmp
{
namespace fn
{
    namespace detail
    {
        struct is_functor;
        struct is_function;
        struct is_member_function;

        template <typename F>
        struct callable_type_impl
        {
            typedef is_functor type;
            typedef typename callable_type_impl<decltype(&F::operator())>::arguments    arguments;
            typedef typename callable_type_impl<decltype(&F::operator())>::return_type  return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (*)(ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (&)(ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (*&)(ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (&&)(ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename R, typename ... ARGS>
        struct callable_type_impl<R (*&&)(ARGS...)>
        {
            typedef is_function         type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename C, typename R, typename ... ARGS>
        struct callable_type_impl<R (C::*)(ARGS...)>
        {
            typedef is_member_function  type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };
        template <typename C, typename R, typename ... ARGS>
        struct callable_type_impl<R (C::*)(ARGS...) const>
        {
            typedef is_member_function  type;
            typedef tuple_t<ARGS...>    arguments;
            typedef R                   return_type;
        };

        template <typename F>
        struct callable_type
        {
            typedef typename callable_type_impl<F>::type        type;
            typedef typename callable_type_impl<F>::arguments   arguments;
            typedef typename callable_type_impl<F>::return_type return_type;
        };

        template <size_t N, typename ... TS>
        using arg_type = get_element_at<N < sizeof...(TS), no_type, get_element_at<N, TS...>>;

        template <size_t N, typename ... TS>
        struct select_argument_impl
        {
            static constexpr no_type select(const TS&& ...){ return *reinterpret_cast<no_type*>(0); }
        };

        template <size_t N, typename T, typename ... TS>
        struct select_argument_impl<N,T,TS...>
        {
            static constexpr arg_type<N,T,TS...> select(const T&&, const TS&& ... ts)
            {
                return select_argument_impl<N-1,TS...>::select(std::forward<const TS>(ts)...);
            }
        };
        template <typename T, typename ... TS>
        struct select_argument_impl<0,T,TS...>
        {
            static constexpr T select(const T&& t, const TS&& ...)
            {
                return t;
            }
        };

        template <size_t N, typename ARGS, typename ... FS>
        struct select_lambda_impl
        {
            static auto select(FS&& ...){ return [](){}; }
        };

        template <size_t N, typename ARGS, typename F, typename ... FS>
        struct select_lambda_impl<N,ARGS,F,FS...>
        {
            static auto select(F&&, FS&& ... fs)
            {
                typedef typename callable_type<get_element_at<0,FS...>>::arguments next_args;
                return select_lambda_impl<N-1,next_args,FS...>::select(std::forward<FS>(fs)...);
            }
        };
        template <typename ... ARGS, typename F, typename ... FS>
        struct select_lambda_impl<0,tuple_t<ARGS...>,F,FS...>
        {
            static auto select(F&& f, FS&& ...)
            {
                return [&f](ARGS && ... args)
                {
                    return f(std::forward<ARGS>(args)...);
                };
            }
        };

    } // namespace detail

    template <size_t N, typename ... TS>
    constexpr detail::arg_type<N,TS...> select_argument(const TS&& ... args)
    {
        static_assert(sizeof...(args) > N, "Index is out of range!\n");
        return detail::select_argument_impl<N,TS...>::select(std::forward<const TS>(args)...);
    }

    template <size_t N, typename ... FS>
    auto select_lambda(FS && ... fs)
    {
        typedef typename detail::callable_type<get_element_at<0,FS...>>::arguments next_args;
        return detail::select_lambda_impl<N,next_args,FS...>::select(std::forward<FS>(fs)...);
    }

    template <size_t N>
    auto select_argument_lambda()
    {
        return [](const auto && ... args)
        {
            return select_argument<N>(std::forward<decltype(args)>(args)...);
        };
    }

    template <size_t ... NS, typename F, typename ... ARGS>
    typename detail::callable_type<F>::return_type select_and_forward_arguments(F && functor, ARGS && ... args)
    {
        return functor(select_argument<NS>(std::forward<ARGS>(args)...)...);
    }

    template <size_t ... NS>
    auto select_and_forward_arguments_lambda()
    {
        return [] (auto fn, auto && ... args)
        {
            return select_and_forward_arguments<NS...>(fn,std::forward<decltype(args)>(args)...);
        };
    }

    /*
    template <typename,typename ... FS>
    auto compose_function_tree(FS... fs)


    template <typename ... CS,typename...FS>
    auto compose_function_tree(FS ... fs)

    CS... ---> select_arg<A1>, select_arg<A2>, select_fun<F1>::with_args<CS2...>

    */
    template <size_t I>
    struct arg
    {
        enum { index = I };
    };
    template <size_t I>
    struct fn
    {
        template <typename ... CS>
        struct with_args
        {
            enum { index = I };
            using args = tuple_t<CS...>;
        };
    };
    namespace detail
    {
        template <typename A>
        using get_sel_args = typename A::args;
        template <typename A>
        constexpr auto get_sel_index = A::index;

        template <size_t I, typename T>
        struct arg_index_type;

        template <size_t I, typename T, typename S>
        struct add_to_argument_set
        {
            typedef typename ::tmp::detail::set_of_impl<tuple_t<arg_index_type<I,T>>,S>::type type;
        };
        template <typename S1, typename S2>
        struct merge_argument_sets
        {
            typedef typename ::tmp::detail::set_of_impl<S2,S1>::type type;
        };

        template <typename T>
        struct arg_index_type_to_int_type;
        template <size_t I, typename T>
        struct arg_index_type_to_int_type<arg_index_type<I,T>>
        {
            typedef int_type<I> type;
        };

        template <size_t N,typename T>
        struct arg_index_type_is_index;
        template <size_t N, size_t M, typename T>
        struct arg_index_type_is_index<N,arg_index_type<M,T>>
        {
            enum { value = N == M };
        };

        template <size_t N>
        struct argument_type_at_index
        {
            template <typename T>
            struct matches
            {
                enum { value = arg_index_type_is_index<N,T>::value };
            };
        };

        template <typename>
        struct arg_index_to_type;
        template <>
        struct arg_index_to_type<no_type>
        {
            typedef no_type type;
        };
        template <size_t N, typename T>
        struct arg_index_to_type<arg_index_type<N,T>>
        {
            typedef T type;
        };

        template <typename S, typename SEQ>
        struct argument_set_finalize;
        template <typename S, size_t ... SEQ>
        struct argument_set_finalize<S,tuple_i<SEQ...>>
        {
            enum { value = (((cardinality< filter<   arg_index_type_to_int_type,
                                                     int_type<SEQ>,
                                                     S
                                                 >
                                       >)
                                   < 2) && ... && true) };
            typedef tuple_t< typename arg_index_to_type<
                                typename get_element_of<S>:: template that_satisfies<
                                    argument_type_at_index<SEQ>:: template matches > >::type ... > type;
        };

        template <size_t F, size_t FA, size_t MAXI, typename S, typename CTS, typename ... FS>
        struct deduce_argument_set_helper;
        template <size_t F, size_t FA, size_t MAXI, typename S, size_t I, typename ... CTS, typename ... FS>
        struct deduce_argument_set_helper<F, FA, MAXI, S, tuple_t<arg<I>, CTS...>, FS...>
        {
            typedef typename callable_type< get_element_at<F,FS...> >::arguments current_args;
            enum { current_args_size = cardinality<current_args> };
            typedef get_element_in<FA,current_args> current_arg_type;

            static_assert(current_args_size - FA == sizeof...(CTS)+1, "Function argument description does not match its arity.\n");

            typedef deduce_argument_set_helper< F,
                                                FA+1,
                                                MAXI < I ? I : MAXI,
                                                typename add_to_argument_set<I, current_arg_type, S>::type,
                                                tuple_t<CTS...>,
                                                FS...>
                next;

            typedef typename next::type type;
            enum { max = next::max };
        };
        template <size_t F, size_t FA, size_t MAXI, typename S, typename CT, typename ... CTS, typename ... FS>
        struct deduce_argument_set_helper<F, FA, MAXI, S, tuple_t<CT, CTS...>, FS...>
        {
            typedef typename callable_type< get_element_at<F,FS...> >::arguments current_args;
            enum { current_args_size = cardinality<current_args> };
            typedef get_element_in<FA,current_args> current_arg_type;
            typedef typename callable_type< get_element_at<get_sel_index<CT>, FS...> >::return_type f_return_type;

            static_assert(current_args_size - FA == sizeof...(CTS)+1, "Function argument description does not match its arity.\n");
            static_assert(types_equal<f_return_type,current_arg_type>, "The inner function's return type must match the the outer functions argument type at this index.\n");

            typedef deduce_argument_set_helper<get_sel_index<CT>,
                                               0,
                                               0,
                                               tuple_t<>,
                                               get_sel_args<CT>,
                                               FS...>
                f;

            typedef typename f::type ftype;
            enum { fmax = f::max };

            typedef deduce_argument_set_helper<F,
                                               FA+1,
                                               MAXI < fmax ? fmax : MAXI,
                                               typename merge_argument_sets<S,ftype>::type,
                                               tuple_t<CTS...>,
                                               FS...>
                next;

            typedef typename next::type type;
            enum { max = next::max };
        };
        template <size_t F, size_t FA, size_t MAXI, typename S, typename ... FS>
        struct deduce_argument_set_helper<F, FA, MAXI, S, tuple_t<>, FS...>
        {
            enum { max = MAXI };
            typedef S type;
        };

        template <typename CT, typename ... FS>
        struct deduce_argument_set
        {
            using return_type = typename callable_type<get_element_at<get_sel_index<CT>,FS...>>::return_type;
            typedef deduce_argument_set_helper<get_sel_index<CT>,
                                                0,
                                                0,
                                                tuple_t<>,
                                                get_sel_args<CT>,
                                                FS...> f;
            typedef typename f::type type;
            enum { arity = f::max };

            static_assert(argument_set_finalize<type,sequence<arity+1>>::value, "Argument expected to be only one distinct type.\n");
            typedef typename argument_set_finalize<type,sequence<arity+1>>::type arguments;
        };

        template <typename CT, typename ARGS>
        struct composition_tree_impl;
        template <size_t N, typename ... ARGS>
        struct composition_tree_impl<arg<N>,tuple_t<ARGS...>>
        {
            template <typename ... FS>
            static auto lambda(FS && ...)
            {
                return [](ARGS && ... args)
                {
                    return select_argument<N>(std::forward<ARGS>(args)...);
                };
            }
        };
        template <template <typename ...> typename SELFUN, typename ... CTS, typename ... ARGS>
        struct composition_tree_impl<SELFUN<CTS...>,tuple_t<ARGS...>>
        {
            template <typename ... FS>
            static auto lambda(FS && ... fs)
            {
                enum { index = SELFUN<CTS...>::index };
                return [&fs...](ARGS && ... args)
                {
                    return select_lambda<index>(std::forward<FS>(fs)...)
                        (composition_tree_impl<CTS,tuple_t<ARGS...>>::lambda(std::forward<FS>(fs)...)
                            (std::forward<ARGS>(args)...)
                             ...
                        );
                };
            }
        };

        template <typename R, typename ARGS, typename CT>
        struct compose_function_tree_impl;
        template <typename R, typename ... ARGS, typename CT>
        struct compose_function_tree_impl<R, tuple_t<ARGS...>, CT>
        {
            template <typename ... FS>
            static auto lambda(FS && ... fs)
            {
                return [&fs...](ARGS && ... args)
                {
                    return composition_tree_impl<CT,tuple_t<ARGS...>>::lambda(std::forward<FS>(fs)...)
                        (std::forward<ARGS>(args)...);
                };
            }
        };
    } // namespace detail

    template <typename CT, typename...FS>
    auto compose_function_tree_lambda(FS && ... fs)
    {
        typedef typename detail::deduce_argument_set<CT,FS...>::return_type return_type;
        typedef typename detail::deduce_argument_set<CT,FS...>::arguments arguments;
        return detail::compose_function_tree_impl<
                return_type,
                arguments,
                CT
              >::lambda(std::forward<FS>(fs)...);
    }
} // namespace fn
} // namespace tmp

#endif /* TMP_FUNCTION_H */
