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


#ifndef BMRPCMESSAGE_H
#define BMRPCMESSAGE_H

namespace bm
{
namespace rpc
{
    template <typename D>
    class Message {};

    /**
     * Message
     * [text type]
     */

    template <>
    class Message<std::string> {
    public:

        //the message is composed of:
        //id: null terminated string.
        //rpc name: null terminated string.
        //payload (args values): null terminated string.

        Message() = default;

        [[nodiscard]] const std::string & getName() const {
            return m_name;
        }

        void setName(std::string& name){
            m_name = name;
        }

        [[nodiscard]] const std::string& getValue() const {
            return m_value;
        }

        void setValue(std::string& value){
            m_value = std::move(value);
        }

        [[nodiscard]] const std::string& getId() const {
            return m_id;
        }

        void getId(uint16_t& id) const {
            std::from_chars(m_id.data(), m_id.data() + m_id.size(), id);
        }

        void setId(uint16_t& id) {
            m_id = std::to_string(id);
        }

        void setId(std::string& id) {
            m_id = id;
        }

    private:
        std::string m_name;
        std::string m_id;
        std::string m_value;
    };


    /**
    * Message
    * [binary type]
    */

    template <>
    class Message<std::vector<unsigned char>> {
    public:

        //the message is composed of:
        //id: word (uint6_t)
        //rpc name: 0 terminated string.
        //payload (args values): vector<unsigned char>.

        Message():m_id(0){};

        [[nodiscard]] const std::string& getName() const {
            return m_name;
        }

        void setName(std::string& name){
            m_name = name;
        }

        [[nodiscard]] const std::vector<unsigned char>& getValue() const {
            return m_value;
        }

        void setValue(std::vector<unsigned char>& value){
            m_value = value;
        }

        void resetValue(){
            m_value.clear();
        }

        void writeValue(const unsigned char* pvalue, size_t size){
            for (int i = 0; i < size; ++i,++pvalue) {
                m_value.push_back(*pvalue);
            }
        }

        [[nodiscard]] size_t getSize() const {
            return m_value.size();
        }

        [[nodiscard]] const uint16_t& getId() const {
            return m_id;
        }

        void setId(uint16_t& value) {
            m_id = value;
        }

    private:
        std::string m_name;
        uint16_t m_id{};
        std::vector<unsigned char> m_value;
    };

}//namespace rpc
}//namespace bm


#endif // BMRPCMESSAGE_H




