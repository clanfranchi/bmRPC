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



#include <iostream>
#include <unordered_map>
#include <vector>
#include <charconv>     //from_chars and to_chars
#include <algorithm>// for godbolt
#include <chrono> // for godbolt
#include "bmRPCUtilities.h"

namespace bm
{
namespace rpc
{
    std::string ltrim(const std::string &str) {
        std::string s(str);
        s.erase(
                s.begin(),
                find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);})
        );

        return s;
    }

    [[maybe_unused]] std::string rtrim(const std::string &str) {
        std::string s(str);

        s.erase(
                find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c);}).base(),
                s.end()
        );

        return s;
    }

    std::vector<std::string> split(const std::string &str) {
        std::vector<std::string> tokens;

        std::string::size_type start = 0;
        std::string::size_type end;

        while ((end = str.find(' ', start)) != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + 1;
        }
        tokens.push_back(str.substr(start));
        return tokens;
    }

    //String version of split
    [[maybe_unused]] std::vector<std::string> splitST(const std::string& str, const std::string& delims)
    {
        std::vector<std::string> output;
        auto first = std::cbegin(str);

        while (first != std::cend(str))
        {
            const auto second = std::find_first_of(first, std::cend(str),
                                                   std::cbegin(delims), std::cend(delims));

            if (first != second)
                output.emplace_back(first, second);

            if (second == std::cend(str))
                break;

            first = std::next(second);
        }

        return output;
    }

    //StringView version of Split
    [[maybe_unused]] std::vector<std::string_view> splitSV(std::string_view strv, std::string_view delims)
    {
        std::vector<std::string_view> output;
        size_t first = 0;

        while (first < strv.size())
        {
            const auto second = strv.find_first_of(delims, first);

            if (first != second)
                output.emplace_back(strv.substr(first, second-first));

            if (second == std::string_view::npos)
                break;

            first = second + 1;
        }

        return output;
    }

    //adjust size to the closest, but minor, power of two.
    [[maybe_unused]] size_t adjust_power_2(size_t size){
        size_t r = 1;
        while (size >>= 1) {
            r<<=1;
        }
        return r;
    }

}//namespace rpc
}//namespace bm
