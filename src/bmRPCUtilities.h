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


#ifndef BMRPCUTILITIES_H
#define BMRPCUTILITIES_H

namespace bm
{
namespace rpc
{
    template<class T, class U=
    typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<typename std::remove_extent<T>::type>::type>::type>::type>
    struct remove_all : remove_all<U> {};
    template<class T>
    struct remove_all<T, T> { typedef T type; };

    template< class G >
    struct remove_cvref {
        typedef std::remove_cv_t<std::remove_reference_t<G>> type;
    };

    std::string ltrim(const std::string &str);

    [[maybe_unused]] std::string rtrim(const std::string &str);

    std::vector<std::string> split(const std::string &str);

    //String version of split
    [[maybe_unused]] std::vector<std::string> splitST(const std::string& str, const std::string& delims = " ");

    //StringView version of Split
    [[maybe_unused]] std::vector<std::string_view> splitSV(std::string_view strv, std::string_view delims = " ");

    //adjust size to the closest, but minor, power of two.
    [[maybe_unused]] size_t adjust_power_2(size_t size);

}//namespace rpc
}// namespace bm



#endif // BMRPCUTILITIES_H




