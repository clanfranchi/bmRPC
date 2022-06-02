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


#ifndef BMRPCANYARG_H
#define BMRPCANYARG_H

namespace bm
{
namespace rpc
{
    namespace stream {
        /**
         * Endianess
         */

        //Static swap
        template<class T, std::size_t... N>
        constexpr T sta_byteswap_impl(T i, std::index_sequence<N...>) {
            return (((i >> N * CHAR_BIT & std::uint8_t(-1)) << (sizeof(T) - 1 - N) * CHAR_BIT) | ...);
        }

        template<class T, class U = std::make_unsigned_t<T>>
        void sta_byteswap(T &i) {
            i = sta_byteswap_impl<U>(i, std::make_index_sequence<sizeof(T)>{});
        }

        //Dynamic swap
        template<typename T>
        void dyn_byteswap(T &value) {
            union U {
                T value;
                unsigned char raw[sizeof(T)];
            } src{}, dst{};
            src.value = value;
            dst.value = 0;
            std::reverse_copy(src.raw, src.raw + sizeof(T), dst.raw);
            value = dst.value;
        }

        /**
         * Write
         */

        template<typename S, typename T>
        [[maybe_unused]] void write(S &s, T data) {
#ifdef LOOP_BACK_TEST
            std::cout << "Error! Should not end here" << std::endl;
#endif
        }

        template<typename T>
        void write(std::ostringstream &s, T data) {
            s << " ";//delimeter
            if constexpr(std::is_pointer_v<T>)
                s << *data;
            else
                s << data;
        }

        template<typename T>
        void write(std::vector<unsigned char> &s, T data) {
            if constexpr(std::is_pointer_v<std::remove_cv_t<T>>) {
                SIZE_T n;
                using t = typename remove_all<T>::type;
                const unsigned char *p_src;
                if constexpr(std::is_same_v<t, std::string> || std::is_same_v<t, std::vector<unsigned char>>) {
                    SIZE_T length = n = data->size();

                    if constexpr(to_swap) { sta_byteswap(length); }

                    p_src = reinterpret_cast<const unsigned char *>(&length);
                    s.push_back(*p_src++);
                    s.push_back(*p_src);
                    if constexpr(std::is_same_v<t, std::string>)
                        p_src = reinterpret_cast<const unsigned char *>(data->c_str());//null terminated string
                    else
                        p_src = reinterpret_cast<const unsigned char *>(data->data());
                    for (int i = 0; i < n; i++, p_src++) {
                        s.push_back(*p_src);
                    }
                } else {
                    n = sizeof(t);
                    p_src = reinterpret_cast<const unsigned char *>(data) + n - 1;
                    for (int i = n; i > 0; --i, --p_src) {
                        s.push_back(*p_src);
                    }
                }
            } else {
                if constexpr(std::is_same_v<T, char> || std::is_same_v<T, bool>) {
                    s.push_back(data);
                } else {
                    if constexpr(to_swap) {
                        if constexpr(std::is_same_v<T, float> || std::is_same_v<T, double>)
                            dyn_byteswap(data);
                        else
                            sta_byteswap(data);
                    }

                    auto p_src = reinterpret_cast<const unsigned char *>(&data);
                    for (int i = 0; i < sizeof(T); ++i, ++p_src) {
                        s.push_back(*p_src);
                    }
                }

            }
        }


        /**
        * Read
        */

        template<typename S, typename T>
        void read(S &s, T &data) {
#ifdef LOOP_BACK_TEST
            std::cout << "Error! Should not end here" << std::endl;
#endif
        }

        template<typename T>
        inline void read(std::string s, T &data) {
            if constexpr(std::is_same_v<T, float>) {
                //stof is used in place of from_chars because the overloads for floating-point types
                //is missing in some libstdcc++ (https://gcc.gnu.org/pipermail/gcc-patches/2020-July/550331.html).
                //To be fixed in the future as stof can be roughly 3x slower than from_chars.
                //std::from_chars(str.data(), str.data() + str.size(), fconv) ;
                data = stof(s);
            } else if constexpr(std::is_same_v<T, double>) {
                //same as stof
                data = stod(s);
            } else if constexpr(std::is_same_v<T, char>) {
                data = s.c_str()[0];
            } else if constexpr(std::is_same_v<T, std::string>) {
                data = s;
            } else
                std::from_chars(s.data(), s.data() + s.size(), data);
        }


        template<typename T>
        void read(std::vector<unsigned char>::iterator s, T &data) {
            if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<unsigned char>>) {
                SIZE_T size;
                auto p = reinterpret_cast<unsigned char *>(&size);
                *p++ = *s++;
                *p = *s++;

#ifdef LOOP_BACK_TEST
                if constexpr(to_swap) { sta_byteswap(size); }
#endif

                std::copy(s, s + size, std::back_inserter(data));
            } else {
                if constexpr(std::is_same_v<T, char> || std::is_same_v<T, bool>) {
                    data = *s;
                } else {
                    auto p_data = reinterpret_cast<unsigned char *>(&data);
                    for (int i = 0; i < sizeof(T); i++, p_data++) {
                        *p_data = s[i];
                    }

#ifdef LOOP_BACK_TEST
                    if constexpr(to_swap) {
                        if constexpr(std::is_same_v<T, float> || std::is_same_v<T, double>) { dyn_byteswap(data); }
                        else { sta_byteswap(data); }
                    }
#endif
                }
            }
        }

        template<typename T>
        void read(std::vector<unsigned char> s, T &data) {
            auto p_data = reinterpret_cast<unsigned char *>(&data);
            auto p = s.data();
            for (int i = 0; i < sizeof(T); i++, p_data++) {
                *p_data = p[i];
            }

#ifdef LOOP_BACK_TEST
            if constexpr(to_swap) {
                if constexpr(std::is_same_v<T, float> || std::is_same_v<T, double>) { dyn_byteswap(data); }
                else { sta_byteswap(data); }
            }
#endif
        }
    }//namespace stream

    /**
     * AnyArg
     */

    union Any_P32 {
        [[maybe_unused]] char c;
        [[maybe_unused]] bool b;
        [[maybe_unused]] short s;
        [[maybe_unused]] int i;
        [[maybe_unused]] long l;
        [[maybe_unused]] float f;
        [[maybe_unused]] void *p;
        [[maybe_unused]] unsigned char a[sizeof(float)];
    };

    union Any_P64 {
        [[maybe_unused]] char c;
        [[maybe_unused]] bool b;
        [[maybe_unused]] short s;
        [[maybe_unused]] int i;
        [[maybe_unused]] long l;
        [[maybe_unused]] long long ll;
        [[maybe_unused]] float f;
        [[maybe_unused]] double d;
        [[maybe_unused]] void *p;
        [[maybe_unused]] unsigned char a[sizeof(double)];
    };

    using U = typename std::conditional<is_p64, Any_P64, Any_P32>::type;

    //Input argument types
    enum class [[maybe_unused]] In_ArgTypeId_P32 {
        WRONG = -1,
        BOOL = 0,//bool by value
        CHAR,//char by value
        UCHAR,//unsigned char by value
        SHORT,//short by value
        USHORT,//unsigned short by value
        INT,//int by value
        UINT,//unsigned int by value
        LONG,//long by value
        ULONG,//unsigned long by value
        FLOAT,//float by value
        CONST_STRING_REF,//const string& by reference
#if BINARY_BASED_PROTOCOL
        CONST_BLOB_REF,//const vector<unsigned char>& by reference
#endif
        INT_REF,//int by reference
        LONG_REF,//long by reference
        FLOAT_REF,//float by reference
        STRING_REF,//string& by reference
#if BINARY_BASED_PROTOCOL
        BLOB_REF,//vector<unsigned char>& by reference
#endif
        VOID// function with no return value
    };


    //Output Argument types
    enum class [[maybe_unused]] Out_ArgTypeId_P32 {
        WRONG = -1,
        INT_REF = 0,//int by reference
        LONG_REF,//long by reference
        FLOAT_REF,//float by reference
        STRING_REF,//string& by reference
#if BINARY_BASED_PROTOCOL
        BLOB_REF,//vector<unsigned char>& by reference
#endif
        VOID// function with no return value
    };


    //Return Values types
    enum class [[maybe_unused]] R_ArgTypeId_P32 {
        WRONG = -1,
        INT,//int by value
        LONG,//long by value
        FLOAT,//float by value
        VOID// function with no return value
    };

    //Input argument types
    enum class [[maybe_unused]] In_ArgTypeId_P64 {
        WRONG = -1,
        BOOL = 0,//bool by value
        CHAR,//char by value
        UCHAR,//unsigned char by value
        SHORT,//short by value
        USHORT,//unsigned short by value
        INT,//int by value
        UINT,//unsigned int by value
        LONG,//long by value
        ULONG,//unsigned long by value
        FLOAT,//float by value
        CONST_STRING_REF,//const string& by reference
#if BINARY_BASED_PROTOCOL
        CONST_BLOB_REF,//const vector<unsigned char>& by reference
#endif
        INT_REF,//int by reference
        LONG_REF,//long by reference
        FLOAT_REF,//float by reference
        STRING_REF,//string& by reference
#if BINARY_BASED_PROTOCOL
        BLOB_REF,//vector<unsigned char>& by reference
#endif
        VOID, // function with no return value
        LONGLONG,//long long by value
        ULONGLONG,//unsigned long long by value
        LONGLONG_REF,//unsigned long long by reference
        DOUBLE,//double by value
        DOUBLE_REF//double by reference
    };

    //Output Argument types
    enum class  [[maybe_unused]] Out_ArgTypeId_P64 {
        WRONG = -1,
        INT_REF = 0,//int by reference
        LONG_REF,//long by reference
        FLOAT_REF,//float by reference
        STRING_REF,//string& by reference
#if BINARY_BASED_PROTOCOL
        BLOB_REF,//vector<unsigned char>& by reference
#endif
        VOID,// function with no return value
        LONGLONG_REF,//long long by reference
        DOUBLE_REF//double by reference
    };

    //Return Values types
    enum class [[maybe_unused]] R_ArgTypeId_P64 {
        WRONG = -1,
        INT,//int by value
        LONG,//long by value
        FLOAT,//float by value
        LONGLONG,//long long by value
        DOUBLE,//double by value
        VOID// function with no return value
    };

    using InArgTypeId = typename std::conditional<is_p64, In_ArgTypeId_P64, In_ArgTypeId_P32>::type;
    using OutArgTypeId = typename std::conditional<is_p64, Out_ArgTypeId_P64, Out_ArgTypeId_P32>::type;
    using RArgTypeId = typename std::conditional<is_p64, R_ArgTypeId_P64, R_ArgTypeId_P32>::type;

    typedef std::vector<unsigned char> blob;

    /**
    * AnyArg
    */

    class AnyArg {
    public:

        explicit AnyArg() {
            val.p = nullptr;
        }

        template<typename T>
        explicit AnyArg(T &value) {
            static_assert(sizeof(val) >= sizeof(T), "Argument type bigger than storage type!");
            val.p = nullptr;
            std::memcpy(val.a, &value, sizeof(T));
        }

        template<typename T>
        inline T getAs() {
            if constexpr(std::is_same<T, bool>::value)
                return val.b;
            else if constexpr(std::is_same<T, char>::value || std::is_same<T, unsigned char>::value)
                return val.c;
            else if constexpr(std::is_same<T, short>::value || std::is_same<T, unsigned short>::value)
                return val.s;
            else if constexpr(std::is_same<T, int>::value || std::is_same<T, unsigned int>::value ||
                              std::is_same<T, int &>::value)
                return val.i;
            else if constexpr(std::is_same<T, long>::value || std::is_same<T, unsigned long>::value ||
                              std::is_same<T, long &>::value)
                return val.l;
            else if constexpr(std::is_same<T, float>::value || std::is_same<T, float &>::value)
                return val.f;
            else if constexpr(std::is_same<T, std::string &>::value)
                return *(std::string *) val.p;
            else if constexpr(std::is_same<T, const std::string &>::value)
                return *(const std::string *) val.p;
            else if constexpr(std::is_same<T, std::string *>::value)
                return (std::string *) val.p;
            else if constexpr(std::is_same<T, const std::string *>::value)
                return (const std::string *) val.p;
#if BINARY_BASED_PROTOCOL
            else if constexpr(std::is_same<T, blob &>::value)
                return *(blob *) val.p;
            else if constexpr(std::is_same<T, const blob &>::value)
                return *(const blob *) val.p;
            else if constexpr(std::is_same<T, blob *>::value)
                return (blob *) val.p;
            else if constexpr(std::is_same<T, const blob *>::value)
                return (const blob *) val.p;
#endif
            else if constexpr(std::is_same<T, void *>::value)
                return (void *) val.p;
#if P64
            else if constexpr(std::is_same<T, long long>::value || std::is_same<T, unsigned long long>::value ||
                              std::is_same<T, long long &>::value)
                return val.ll;
            else if constexpr(std::is_same<T, double>::value || std::is_same<T, double &>::value)
                return val.d;
#endif
            else return val;
        }

        template<typename T, typename S>
        inline void write(S &s) {
            if constexpr(std::is_same<T, bool>::value)
                stream::write(s, val.b);
            else if constexpr(std::is_same<T, char>::value || std::is_same<T, unsigned char>::value)
                stream::write(s, val.c);
            else if constexpr(std::is_same<T, short>::value || std::is_same<T, unsigned short>::value)
                stream::write(s, val.s);
            else if constexpr(std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
                stream::write(s, val.i);
            else if constexpr(std::is_same<T, long>::value || std::is_same<T, unsigned long>::value)
                stream::write(s, val.l);
            else if constexpr(std::is_same<T, float>::value)
                stream::write(s, val.f);
            else if constexpr(std::is_same<T, std::string *>::value) {
                auto p_str = reinterpret_cast<T>(val.p);
                stream::write(s, p_str);
            }
#if BINARY_BASED_PROTOCOL
            else if constexpr(std::is_same<T, blob *>::value) {
                blob *p_str = reinterpret_cast<T>(val.p);
                stream::write(s, p_str);
            }
#endif
#if P64
            else if constexpr(std::is_same<T, long long>::value || std::is_same<T, unsigned long long>::value)
                stream::write(s, val.ll);
            else if constexpr(std::is_same<T, double>::value)
                stream::write(s, val.d);
#endif
            else stream::write(s, *reinterpret_cast<T>(val.p));//handles all the other pointers
        }

    private:
        U val{};
    };

    class ReturnValue {
    public:

        explicit ReturnValue() :
                m_valid(false),
                m_type(RArgTypeId::WRONG),
                m_value(AnyArg()) {};

        explicit ReturnValue(bool valid, RArgTypeId type, AnyArg value) :
                m_valid(valid),
                m_type(type),
                m_value(value) {};


        [[nodiscard]] INLINE bool valid() const { return m_valid; }

        template<typename T>
        INLINE auto get_value() { return m_value.getAs<T>(); }

        [[maybe_unused]] INLINE auto get_type() { return m_type; }

    private:
        bool m_valid;
        RArgTypeId m_type;
        AnyArg m_value;
    };


    /**
     * ArgType
     */

    //RPC ARGUMENTS FORMAT
    //List of the accepted argument types without considering cv and reference.
    //Template arguments to string using class template and pack folding
    template<typename... Args>
    struct ArgType {
        static constexpr bool valid = false;
    };

    template<>
    struct ArgType<bool> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<char> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<unsigned char> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<short> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<unsigned short> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<int> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<int *> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<unsigned int> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<long *> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<unsigned long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<long long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<long long *> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<unsigned long long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<float> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<float *> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<double> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<double *> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<std::string> {
        static constexpr bool valid = true;
    };

    template<>
    struct ArgType<std::string *> {
        static constexpr bool valid = true;
    };

#if BINARY_BASED_PROTOCOL

    template<>
    struct ArgType<blob> {
        static constexpr bool valid = true;
    };


    template<>
    struct ArgType<blob *> {
        static constexpr bool valid = true;
    };
#endif

    template<>
    struct ArgType<void> {
        static constexpr bool valid = true;
    };


    /**
     * ParamType
     */

    //RPC PARAMETERS FORMAT
    //List of the accepted parameter types.
    //Template arguments to string using class template and pack folding
    template<class Enable = void, typename... Args>
    struct ParamType {
        static constexpr const char *name = "";
        static constexpr bool valid = false;
        static constexpr InArgTypeId in_id = InArgTypeId::WRONG;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<bool> {
        static constexpr const char *name = "bool";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::BOOL;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<char> {
        static constexpr const char *name = "char";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::CHAR;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<unsigned char> {
        static constexpr const char *name = "uchar";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::UCHAR;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<short> {
        static constexpr const char *name = "short";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::SHORT;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<unsigned short> {
        static constexpr const char *name = "ushort";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::USHORT;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<int> {
        static constexpr const char *name = "int";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::INT;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::INT;
    };

    template<>
    struct ParamType<int &> {
        static constexpr const char *name = "int&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::INT_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::INT_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<unsigned int> {
        static constexpr const char *name = "uint";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::UINT;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<long> {
        static constexpr const char *name = "long";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::LONG;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::LONG;
    };

    template<>
    struct ParamType<long &> {
        static constexpr const char *name = "long&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::LONG_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::LONG_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<unsigned long> {
        static constexpr const char *name = "ulong";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::ULONG;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<float> {
        constexpr static const char *name = "float";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::FLOAT;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::FLOAT;
    };

    template<>
    struct ParamType<float &> {
        static constexpr const char *name = "float&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::FLOAT_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::FLOAT_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

#if P64
    template<>
    struct ParamType<long long> {
        static constexpr const char *name = "longlong";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::LONGLONG;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::LONGLONG;
    };

    template<>
    struct ParamType<long long &> {
        static constexpr const char *name = "longlong&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::LONGLONG_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::LONGLONG_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<unsigned long long> {
        static constexpr const char *name = "ulonglong";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::ULONGLONG;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<double> {
        static constexpr const char *name = "double";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::DOUBLE;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::DOUBLE;
    };

    template<>
    struct ParamType<double &> {
        static constexpr const char *name = "double&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::DOUBLE_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::DOUBLE_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };
#endif

    template<>
    struct ParamType<std::string &> {
        static constexpr const char *name = "string&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::STRING_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::STRING_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<const std::string &> {
        static constexpr const char *name = "conststring&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::CONST_STRING_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<std::string *> {
        static constexpr const char *name = "string*";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::STRING_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::STRING_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<const std::string *> {
        static constexpr const char *name = "conststring*";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::CONST_STRING_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

#if BINARY_BASED_PROTOCOL
    template<>
    struct ParamType<blob &> {
        static constexpr const char *name = "vector<unsigned char>&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::BLOB_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::BLOB_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<const blob &> {
        static constexpr const char *name = "constvector<unsigned char>&";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::CONST_BLOB_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<blob *> {
        static constexpr const char *name = "vector<unsigned char>*";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::BLOB_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::BLOB_REF;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };

    template<>
    struct ParamType<const blob *> {
        static constexpr const char *name = "constvector<unsigned char>*";
        static constexpr bool valid = true;
        static constexpr InArgTypeId in_id = InArgTypeId::CONST_BLOB_REF;
        static constexpr OutArgTypeId out_id = OutArgTypeId::WRONG;
        static constexpr RArgTypeId r_id = RArgTypeId::WRONG;
    };
#endif

    template<>
    struct ParamType<void> {
        static constexpr const char *name = "void";
        static constexpr bool valid = false;
        static constexpr InArgTypeId in_id = InArgTypeId::VOID;
        static constexpr OutArgTypeId out_id = OutArgTypeId::VOID;
        static constexpr RArgTypeId r_id = RArgTypeId::VOID;
    };

    /**
     * ReturnType
     */

    template<typename R>
    struct ReturnType {
        static constexpr bool valid = false;
    };

    template<>
    struct ReturnType<int> {
        static constexpr bool valid = true;
    };

    template<>
    struct ReturnType<long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ReturnType<float> {
        static constexpr bool valid = true;
    };

    template<>
    struct ReturnType<long long> {
        static constexpr bool valid = true;
    };

    template<>
    struct ReturnType<double> {
        static constexpr bool valid = true;
    };

    template<>
    struct ReturnType<void> {
        static constexpr bool valid = true;
    };

}//namespace rpc
}//namespace bm

#endif // BMRPCANYARG_H




