/*
 *
 * Copyright 2022 Claudio Lanfranchi.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef BMRPCMARSHALLER_H
#define BMRPCMARSHALLER_H

namespace bm
{
namespace rpc
{

    /**
     * Server Marshaller
     */
    template <typename D, typename E>
    inline void unmarshall_args_in(const D& data, E& in_args){
        if constexpr(std::is_same_v<D, std::string>) {
            in_args = split(data);
        }
        else {
            in_args = data;
        }
    }


    /**
     * Server side: from stream to args
     */
    template <typename D>
    [[maybe_unused]] static std::vector<AnyArg> deserialize_in_args(std::vector<InArgTypeId>& format, D& data){
        std::vector<AnyArg> vec;
        return vec;
    }

    template <>
    [[maybe_unused]] std::vector<AnyArg> deserialize_in_args(std::vector<InArgTypeId>& format,std::vector<std::string>& data){
        std::vector<AnyArg> vec;
        vec.reserve(format.size());
        for(int i = 0; i < format.size(); ++i){
            AnyArg val;
            switch(format[i]){
                case InArgTypeId::BOOL:
                case InArgTypeId::UCHAR:
                case InArgTypeId::CHAR:
                {
                    char cconv;
                    stream::read(data[i], cconv);
                    val = AnyArg(cconv);
                }
                    break;
                case InArgTypeId::SHORT:
                {
                    short sconv;
                    stream::read(data[i], sconv);
                    val = AnyArg(sconv);
                }
                break;
                case InArgTypeId::USHORT:
                {
                    unsigned short usconv;
                    stream::read(data[i], usconv);
                    val = AnyArg(usconv);
                }
                    break;
                case InArgTypeId::INT:
                case InArgTypeId::INT_REF:
                {
                    int iconv;
                    stream::read(data[i], iconv);
                    val = AnyArg(iconv);
                }
                    break;
                case InArgTypeId::UINT:
                {
                    unsigned int uiconv;
                    stream::read(data[i], uiconv);
                    val = AnyArg(uiconv);
                }
                    break;
                case InArgTypeId::LONG:
                case InArgTypeId::LONG_REF:
                {
                    long lconv;
                    stream::read(data[i], lconv);
                    val = AnyArg(lconv);
                }
                    break;
                case InArgTypeId::ULONG:
                {
                    unsigned long ulconv;
                    stream::read(data[i], ulconv);
                    val = AnyArg(ulconv);
                }
                    break;
                case InArgTypeId::FLOAT:
                case InArgTypeId::FLOAT_REF:
                {
                    float fconv;
                    stream::read(data[i], fconv);
                    val = AnyArg(fconv);
                }
                    break;
                case InArgTypeId::CONST_STRING_REF:
                case InArgTypeId::STRING_REF:
                {
                    auto const s = new std::string(std::move(data[i]));//deleted in serialize_out_args
                    val = AnyArg(s);
                }
                    break;
#if P64
                case InArgTypeId::LONGLONG:
                case InArgTypeId::LONGLONG_REF:
                {
                    long long llconv;
                    stream::read(data[i], llconv);
                    val = AnyArg(llconv);
                }
                    break;
                case InArgTypeId::ULONGLONG:
                {
                    unsigned long long ullconv;
                    stream::read(data[i], ullconv);
                    val = AnyArg(ullconv);
                }
                    break;
                case InArgTypeId::DOUBLE:
                case InArgTypeId::DOUBLE_REF:
                {
                    double dconv;
                    stream::read(data[i], dconv);
                    val = AnyArg(dconv);
                }
                    break;
#endif
                case InArgTypeId::WRONG:
                case InArgTypeId::VOID:
                default:
                    AnyArg();
                    break;
            }
            vec.push_back(val);
        }
        return vec;
    }

    template <>
    [[maybe_unused]] std::vector<AnyArg> deserialize_in_args(std::vector<InArgTypeId>& format, std::vector<unsigned char>& data){
        std::vector<AnyArg> vec;
        std::vector<unsigned char>::difference_type ix = 0;
        vec.reserve(format.size());
        for(auto f: format){
            AnyArg val;
            switch(f){
                case InArgTypeId::BOOL:
                case InArgTypeId::UCHAR:
                case InArgTypeId::CHAR:
                {   char cconv;
                    stream::read(data.begin()+ix, cconv);
                    ix += sizeof(char);
                    val = AnyArg(cconv);
                }
                    break;
                case InArgTypeId::USHORT:
                case InArgTypeId::SHORT:
                {   short sconv;
                    stream::read(data.begin()+ix, sconv);
                    ix += sizeof(short);
                    val = AnyArg(sconv);
                }
                    break;
                case InArgTypeId::INT:
                case InArgTypeId::UINT:
                case InArgTypeId::INT_REF:
                {   int iconv;
                    stream::read(data.begin()+ix, iconv);
                    ix += sizeof(int);
                    val = AnyArg(iconv);
                }
                    break;
                case InArgTypeId::LONG:
                case InArgTypeId::ULONG:
                case InArgTypeId::LONG_REF:
                {   long lconv;
                    stream::read(data.begin()+ix, lconv);
                    ix += sizeof(long);
                    val = AnyArg(lconv);
                }
                    break;
                case InArgTypeId::FLOAT:
                case InArgTypeId::FLOAT_REF:
                {   float fconv;
                    stream::read(data.begin()+ix, fconv);
                    ix +=  sizeof(float);
                    val = AnyArg(fconv);
                }
                    break;
                case InArgTypeId::CONST_STRING_REF:
                case InArgTypeId::STRING_REF:
                {
                    SIZE_T length;
                    stream::read(data.begin()+ix, length);
                    ix += sizeof(SIZE_T);
                    auto const s = new std::string(data.begin()+ix, data.begin()+ix+length);//deleted in serialize_out_args
                    ix += length;
                    val = AnyArg(s);
                }
                    break;
#if BINARY_BASED_PROTOCOL
                case InArgTypeId::CONST_BLOB_REF:
                case InArgTypeId::BLOB_REF:
                {
                    SIZE_T length;
                    stream::read(data.begin()+ix, length);
                    ix += sizeof(SIZE_T);
                    blob* const s = new blob(data.begin()+ix, data.begin()+ix+length);//deleted in serialize_out_args
                    ix += length;
                    val = AnyArg(s);
                }
                    break;
#endif
#if P64
                case InArgTypeId::LONGLONG:
                case InArgTypeId::ULONGLONG:
                case InArgTypeId::LONGLONG_REF:
                {   long long llconv;
                    stream::read(data.begin()+ix, llconv);
                    ix += sizeof(long long);
                    val = AnyArg(llconv);
                }
                    break;
                case InArgTypeId::DOUBLE:
                case InArgTypeId::DOUBLE_REF:
                {
                    double dconv;
                    stream::read(data.begin()+ix, dconv);
                    ix += sizeof(double);
                    val = AnyArg(dconv);
                }
                    break;
#endif
                case InArgTypeId::WRONG:
                case InArgTypeId::VOID:
                    AnyArg();
                    break;
            }
            vec.push_back(val);
        }
        return vec;
    }

    /**
     * Server side: from function call to stream
     */

    template <typename T>
    void serialize_r(RArgTypeId format, AnyArg arg, T& buffer)
    {
        switch(format){
            case RArgTypeId::INT:
                arg.write<int>(buffer);
                break;
            case RArgTypeId::LONG:
                arg.write<long>(buffer);
                break;
            case RArgTypeId::FLOAT:
                arg.write<float>(buffer);
                break;
#if P64
            case RArgTypeId::LONGLONG:
                arg.write<long long>(buffer);
                break;
            case RArgTypeId::DOUBLE:
                arg.write<double>(buffer);
                break;
#endif
            case RArgTypeId::VOID:
            case RArgTypeId::WRONG:
                break;
        }
    }

    template <typename T>
    void serialize_out_args(std::vector<InArgTypeId>& format, std::vector<AnyArg>& arr, T& buffer){
        size_t i = 0;
        for(auto f: format){
            switch(f){
                case InArgTypeId::BOOL:
                case InArgTypeId::CHAR:
                case InArgTypeId::UCHAR:
                case InArgTypeId::SHORT:
                case InArgTypeId::USHORT:
                case InArgTypeId::INT:
                case InArgTypeId::UINT:
                case InArgTypeId::LONG:
                case InArgTypeId::ULONG:
                case InArgTypeId::FLOAT:
                    ++i;
                    break;
                case InArgTypeId::CONST_STRING_REF:
                    delete arr[i].getAs<const std::string*>();//allocated by deserialize_in_args
                    ++i;
                    break;
#if BINARY_BASED_PROTOCOL
                case InArgTypeId::CONST_BLOB_REF:
                    delete arr[i].getAs<const blob*>();//allocated by deserialize_in_args
                    ++i;
                    break;
#endif
                case InArgTypeId::INT_REF:
                    arr[i++].write<int>(buffer);
                    break;
                case InArgTypeId::LONG_REF:
                    arr[i++].write<long>(buffer);
                    break;
                case InArgTypeId::FLOAT_REF:
                    arr[i++].write<float>(buffer);
                    break;
                case InArgTypeId::STRING_REF:
                {
                    arr[i].write<std::string*>(buffer);
                    delete arr[i].getAs<std::string*>();//allocated by deserialize_in_args
                    ++i;
                }
                    break;
#if BINARY_BASED_PROTOCOL
                case InArgTypeId::BLOB_REF:
                {
                    arr[i].write<blob*>(buffer);
                    delete arr[i].getAs<blob*>();//allocated by deserialize_in_args
                    ++i;
                }
                    break;
#endif
#if P64
                case InArgTypeId::DOUBLE_REF:
                    arr[i++].write<double>(buffer);
                    break;
                case InArgTypeId::LONGLONG_REF:
                    arr[i++].write<long long>(buffer);
                    break;
                case InArgTypeId::LONGLONG:
                case InArgTypeId::ULONGLONG:
                case InArgTypeId::DOUBLE:
                    i++;
                    break;
#endif
                case InArgTypeId::VOID:
                case InArgTypeId::WRONG:
                    ++i;
                    break;
            }
        }
    }

    //Client side: first element is the return value when its format differs from void
    template <typename D, typename E, typename F>
    void unmarshall_out_args(const RArgTypeId format, const D& data, E& r_arg, F& out_args){
        using TData = typename std::conditional<std::is_same_v<D,std::string>, std::vector<std::string>, std::vector<unsigned char>>::type;
        TData sdata;
        out_args.clear();
        r_arg.clear();
        if constexpr(std::is_same_v<D, std::string>)
        {
            sdata = split(data);
            size_t ix = 0;
            if(format != RArgTypeId::VOID) {
                r_arg = sdata[0];
                ix = 1;
            }
            size_t n = sdata.size();
            out_args.reserve(n);
            for (size_t i = ix; i < n; i++)
                out_args.push_back(sdata[i]);
        }
        else
        {
            if (format == RArgTypeId::VOID)
            {
                out_args = data;
            }
            else
            {
                size_t size;
                switch (format) {
                    case RArgTypeId::INT:
                        size = sizeof(int);
                        break;
                    case RArgTypeId::LONG:
                        size = sizeof(long);
                        break;
                    case RArgTypeId::FLOAT:
                        size = sizeof(float);
                        break;
#if P64
                    case RArgTypeId::LONGLONG:
                        size = sizeof(long long);
                        break;
                    case RArgTypeId::DOUBLE:
                        size = sizeof(double);
                        break;
#endif
                    default:
                        size = 0;
                        break;
                }
                if(size > 0){
                    std::copy(data.begin(), data.begin()+size, std::back_inserter(r_arg));
                    std::copy(data.begin() + size, data.end(), std::back_inserter(out_args));
                }
            }
        }
    }

    //Client side: from stream to return value
    template <typename D>
    ReturnValue deserialize_r(RArgTypeId format, D& data){
        AnyArg val;
        bool valid = true;
        switch(format){
            case RArgTypeId::INT:
            {
                int iconv;
                stream::read(data, iconv);
                val = AnyArg(iconv);
            }
                break;
            case RArgTypeId::LONG:
            {
                long lconv;
                stream::read(data, lconv);
                val = AnyArg(lconv);
            }
                break;
            case RArgTypeId::FLOAT:
            {
                float fconv;
                stream::read(data, fconv);
                val = AnyArg(fconv);
            }
                break;
#if P64
            case RArgTypeId::LONGLONG:
            {
                long long llconv;
                stream::read(data, llconv);
                val = AnyArg(llconv);
            }
                break;
            case RArgTypeId::DOUBLE:
            {
                double dconv;
                stream::read(data, dconv);
                val = AnyArg(dconv);
            }
                break;
#endif
            default:
            {
                valid = false;
                val = AnyArg();
            }
                break;
        }
        return ReturnValue(valid,format,val);
    }

    template<typename R, typename... Args, std::size_t ... Is>
    auto callFuncWithArgs(R (*function)(Args...), std::vector<AnyArg> & vArgs, std::index_sequence<Is...> const &) {
        return function(vArgs[Is].getAs<Args>()...);
    }

    template<typename... Args, std::size_t ... Is>
    void callProcWithArgs(void (*function)(Args...), std::vector<AnyArg>& vArgs, std::index_sequence<Is...> const &) {
        function(vArgs[Is].getAs<Args>()...);
    }

    /**
    * Client Marshaller
    */

    //Client side: from assign data from stream to args_out addresses
    template<typename D>
    [[maybe_unused]] static void deserialize_out_args(const std::vector<OutArgTypeId>& format, D& data, const std::vector<void*>& addresses){}

    template<>
    [[maybe_unused]] void deserialize_out_args(const std::vector<OutArgTypeId>& format, std::vector<std::string>& data, const std::vector<void*>& addresses){
        for(int i = 0; i < format.size(); ++i) {
            switch (format[i])
            {
                case OutArgTypeId::INT_REF:
                {
                    int iconv;
                    stream::read(data[i], iconv);
                    int *p = reinterpret_cast<int*>(addresses[i]);
                    *p = iconv;
                }
                    break;
                case OutArgTypeId::LONG_REF:
                {
                    long lconv;
                    stream::read(data[i], lconv);
                    long *p = reinterpret_cast<long*>(addresses[i]);
                    *p = lconv;
                }
                    break;
                case OutArgTypeId::FLOAT_REF:
                {
                    float fconv;
                    stream::read(data[i], fconv);
                    auto p = reinterpret_cast<float*>(addresses[i]);
                    *p = fconv;
                }
                    break;
                case OutArgTypeId::STRING_REF:
                {
                    std::string s;
                    stream::read(data[i], s);
                    auto p = reinterpret_cast<std::string*>(addresses[i]);
                    *p = s;
                }
                    break;
#if P64
                case OutArgTypeId::LONGLONG_REF:
                {
                    long long llconv;
                    stream::read(data[i], llconv);
                    auto p = reinterpret_cast<long long*>(addresses[i]);
                    *p = llconv;
                }
                    break;
                case OutArgTypeId::DOUBLE_REF:
                {
                    double dconv;
                    stream::read(data[i], dconv);
                    auto p = reinterpret_cast<double*>(addresses[i]);
                    *p = dconv;
                }
                    break;
#endif
                default:
                    break;
            }
        }
    }

    template<>
    [[maybe_unused]] void deserialize_out_args(const std::vector<OutArgTypeId>& format, std::vector<unsigned char>& data, const std::vector<void*>& addresses){
        using DIFFERENCE_TYPE = std::vector<unsigned char>::difference_type;
        DIFFERENCE_TYPE ix = 0;
        for(int i = 0; i < format.size(); ++i) {
            switch (format[i])
            {
                case OutArgTypeId::INT_REF:
                {   int iconv;
                    stream::read(data.begin()+ix, iconv);
                    ix += sizeof(int);
                    int *p = reinterpret_cast<int*>(addresses[i]);
                    *p = iconv;
                }
                    break;
                case OutArgTypeId::LONG_REF:
                {
                    long lconv;
                    stream::read(data.begin()+ix, lconv);
                    ix += sizeof(long);
                    long *p = reinterpret_cast<long*>(addresses[i]);
                    *p = lconv;
                }
                    break;
                case OutArgTypeId::FLOAT_REF:
                {
                    float fconv;
                    stream::read(data.begin()+ix, fconv);
                    ix += sizeof(float);
                    auto p = reinterpret_cast<float*>(addresses[i]);
                    *p = fconv;
                }
                    break;
                case OutArgTypeId::STRING_REF:
                {
                    std::string s;
                    stream::read(data.begin()+ix, s);
                    ix += (DIFFERENCE_TYPE)(sizeof(SIZE_T) + s.size());
                    auto *p = reinterpret_cast<std::string*>(addresses[i]);
                    *p = s;
                }
                    break;
#if BINARY_BASED_PROTOCOL
                case OutArgTypeId::BLOB_REF:
                {
                    blob v;
                    stream::read(data.begin()+ix, v);
                    ix += (DIFFERENCE_TYPE)(sizeof(SIZE_T) + v.size());
                    blob* p = reinterpret_cast<blob*>(addresses[i]);
                    *p = v;
                }
                    break;
#endif
#if P64
                case OutArgTypeId::LONGLONG_REF:
                {
                    long long llconv;
                    stream::read(data.begin()+ix, llconv);
                    ix += sizeof(long long);
                    auto p = reinterpret_cast<long long*>(addresses[i]);
                    *p = llconv;
                }
                    break;
                case OutArgTypeId::DOUBLE_REF:
                {
                    double dconv;
                    stream::read(data.begin()+ix, dconv);
                    ix += sizeof(double);
                    auto p = reinterpret_cast<double*>(addresses[i]);
                    *p = dconv;
                }
                    break;
#endif
                default:
                    break;
            }
        }
    }

    /**
    * Codification
    */

    static std::string string_composer (const std::initializer_list<std::string>& strings) {
        std::stringstream ss_composition;

        std::initializer_list<std::string>::iterator it = strings.begin();
        ss_composition << it[0];//first string without separator
        for(int i = 1; i < strings.size(); ++i){
            ss_composition << " " << it[i];
        }
        return ss_composition.str();
    }

    template <typename ...Args>
    std::string stringify(){
        return string_composer({ParamType<Args>::name...});
    }


    static auto codify_in_args_impl (const std::initializer_list<InArgTypeId>& ids) {
        std::vector<InArgTypeId> args;
        std::initializer_list<InArgTypeId>::iterator it = ids.begin();
        for(int i = 0; i < ids.size(); ++i){
            if(it[i] != InArgTypeId::WRONG)
                args.push_back(it[i]);
        }
        return args;
    }

    template <typename ...Args>
    static auto codify_in_args(){
        return codify_in_args_impl({ParamType<Args>::in_id...});
    }

    static auto codify_out_args_impl (const std::initializer_list<OutArgTypeId>& ids) {
        std::vector<OutArgTypeId> args;
        std::initializer_list<OutArgTypeId>::iterator it = ids.begin();
        for(int i = 0; i < ids.size(); ++i){
            if(it[i] != OutArgTypeId::WRONG)
                args.push_back(it[i]);
        }
        return args;
    }

    template <typename ...Args>
    static auto codify_out_args(){
        return codify_out_args_impl({ParamType<Args>::out_id...});
    }

    /**
    * ParamTraits
    */

    template<class F>
    struct ParamTraits {};

    // function pointer
    template<class R, class... Args>
    struct ParamTraits<R(*)(Args...)> : public ParamTraits<R(Args...)>
    {};

    template<class R, class... Args>
    struct ParamTraits<R(Args...)>{
        //Number of the parameters of the function signature
        static constexpr std::size_t arity = sizeof...(Args);
        //Validity of the parameters of the function signature
        [[maybe_unused]] static constexpr bool valid = ReturnType<R>::valid && ((ParamType<Args>::valid) && ...);
        template <std::size_t N>
        struct parameter{
            static_assert(N < arity, "error: invalid parameter index.");
            using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
        };
    };

    /**
    * Argument Traits
    */

    template<typename F, int N>
    struct Arguments
    {
        template<typename V, typename S>
        static void serialize_impl(V& v,S&) {}
        template<typename V, typename S, typename First, typename... Rest>
        [[maybe_unused]] static void serialize_impl(V& v, S& s, First&& first, Rest&& ... rest)
        {
            using ParamTrait = typename ParamTraits<F>::template parameter<N>::type;
            using arg_base_type = typename remove_all<ParamTrait>::type;
            using par_base_type = typename remove_all<First>::type;
            static constexpr bool same_base = std::is_same<arg_base_type,par_base_type>::value;

            //arithmetics with conversions allowed
            static constexpr bool invalid = (std::is_arithmetic_v<ParamTrait> && !std::is_arithmetic_v<std::remove_reference_t<First>>)
                                            //References with implicit conversions
                                            || (std::is_reference_v<ParamTrait> && !std::is_const_v<std::remove_reference_t<ParamTrait>> && std::is_const_v<std::remove_reference_t<First>>)
                                            || (std::is_reference_v<ParamTrait> && std::is_pointer_v<std::remove_reference_t<First>>)
                                            //Pointers
                                            || (std::is_pointer_v<ParamTrait> && std::is_pointer_v<std::remove_reference_t<First>> && std::is_const_v<std::remove_reference_t<First>>)
                                            || (std::is_pointer_v<ParamTrait> && std::is_reference_v<First>)
                                            //different base
                                            || !same_base;
            static_assert(!invalid, "Argument mismatch!");

            if constexpr(std::is_reference_v<ParamTrait>
                         && (std::is_same_v<std::string, par_base_type>
                             || std::is_same_v<std::vector<unsigned char>, par_base_type>)){
                auto p = std::addressof(first);
                stream::write(s,p);
            }
            else
                stream::write(s,first);

            if constexpr(std::is_reference_v<ParamTrait> && !std::is_const_v<std::remove_reference_t<ParamTrait>>){
                v.push_back(std::addressof(first));
            }
            else if constexpr(std::is_pointer_v<ParamTrait> && !std::is_const_v<std::remove_reference_t<ParamTrait>>){
                v.push_back(first);
            }

            Arguments<F, N+1>::serialize_impl(v,s, std::forward<Rest>(rest)...);
        }
    };

    template<typename F, typename V, typename S, typename... Args>
    static void serialize_args(V& v,S& s, Args&& ... args)
    {
        static_assert(ParamTraits<F>::arity == sizeof...(Args), "Invalid number of parameters for RPC call.");
        Arguments<F, 0>::serialize_impl(v,s, std::forward<Args>(args)...);
    }

}//namespace rpc
}//namespace bm



#endif // BMRPCMARSHALLER_H




