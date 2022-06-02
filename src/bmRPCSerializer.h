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


#ifndef BMRPCSERIALIZER_H
#define BMRPCSERIALIZER_H

namespace bm
{
namespace rpc
{
    /**
     * Serializer
     */

    template <typename D, typename C>
    class Serializer{};

    /**
     * Serializer
     * [text type]
     */

    template <typename C>
    class Serializer<std::string, C>{
    public:

        [[maybe_unused]] explicit Serializer(Streamer<char, C>& s):
                m_streamer(s),
                m_len(0),
                m_p(""),
                m_i(0),
                m_pmsg(nullptr),
                m_tx_phase(IDLE){};


        [[maybe_unused]] void init(const Message<std::string>* pmsg){
            if(pmsg  != nullptr) {
                m_pmsg = pmsg;
                m_len = m_pmsg->getId().size() + 1;
                m_p = m_pmsg->getId().c_str();
                m_i = 0;
                m_tx_phase = ID;
            }
        }

        bool send(){
            switch(m_tx_phase){
                case IDLE:
                case END:
                    break;
                case ID:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_len = m_pmsg->getName().size() + 1;
                        m_p = m_pmsg->getName().c_str();
                        m_i = 0;
                        m_tx_phase = NAME;
                        break;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
                case NAME:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_len = m_pmsg->getValue().size() + 1;
                        m_p = m_pmsg->getValue().c_str();
                        m_i = 0;
                        m_tx_phase = ARGS_VALUE;
                        break;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
                case ARGS_VALUE:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_tx_phase = END;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
            }
            if(m_tx_phase == END){
                m_tx_phase = IDLE;
                return true;
            }
            return false;
        }

    private:
        Streamer<char, C>& m_streamer;
        const Message<std::string>* m_pmsg;
        const char *m_p;
        size_t m_i;
        size_t m_len;
        enum TX_PHASE{
            IDLE = 0,
            ID,
            NAME,
            ARGS_VALUE,
            END
        }m_tx_phase = IDLE;
    };


    /**
    * Serializer
    * [binary type]
    */

    template <typename C>
    class Serializer<std::vector<unsigned char>,C>{
    public:

        [[maybe_unused]] explicit Serializer(Streamer<unsigned char, C>& s):
                m_streamer(s),
                m_len(0),
                m_p(nullptr),
                m_i(0),
                m_id(0),
                m_size(0),
                m_pmsg(nullptr),
                m_tx_phase(IDLE){};

        void init(const Message<std::vector<unsigned char>>* pmsg){
            if(pmsg  != nullptr) {
                m_pmsg = pmsg;
                m_id = m_pmsg->getId();
                m_len = sizeof(m_id);
                m_p = reinterpret_cast<const unsigned char*>(&m_id);
                m_i = 0;
                m_tx_phase = ID;
            }
        }

        bool send(){
            switch(m_tx_phase){
                case IDLE:
                case END:
                    break;
                case ID:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_len = m_pmsg->getName().size() + 1;
                        m_p = reinterpret_cast<const unsigned char*>(m_pmsg->getName().c_str());
                        m_i = 0;
                        m_tx_phase = NAME;
                        break;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
                case NAME:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_size = m_pmsg->getSize();
                        m_len = sizeof(m_size);
                        m_p = reinterpret_cast<const unsigned char*>(&m_size);
                        m_i = 0;
                        m_tx_phase = SIZE;
                        break;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
                case SIZE:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_len = m_pmsg->getValue().size();
                        if(m_len > 0){
                            m_p = m_pmsg->getValue().data();
                            m_i = 0;
                            m_tx_phase = ARGS_VALUE;
                        }
                        else
                            m_tx_phase = END;
                        break;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
                case ARGS_VALUE:
                {
                    size_t count = m_streamer.write(&m_p[m_i], m_len);
                    if(count >= m_len){
                        m_tx_phase = END;
                    }
                    else{
                        m_i += count;
                        m_len -= count;
                    }
                }
                    break;
            }
            if(m_tx_phase == END){
                m_tx_phase = IDLE;
                return true;
            }
            return false;
        }

    private:
        Streamer<unsigned char, C>& m_streamer;
        const Message<std::vector<unsigned char>>* m_pmsg;
        const unsigned char *m_p;
        size_t m_i;
        size_t m_len;
        size_t m_size;
        uint16_t m_id;
        enum TX_PHASE{
            IDLE = 0,
            ID,
            NAME,
            SIZE,
            ARGS_VALUE,
            END
        }m_tx_phase = IDLE;
    };


    /**
     * Deserializer
     */

    template <typename D, typename C>
    class Deserializer{};

    /**
    * Deserializer
    * [text type]
    */

    template <typename C>
    class Deserializer<std::string, C>{
    public:
        //the message is composed of:
        //id
        //rpc name: 0 terminated string.
        //payload
        // args format: 0 terminated string.
        // args values: 0 terminated string.

        [[maybe_unused]] explicit Deserializer(Streamer<char, C>& s):
                m_streamer(s),
                m_len(0),
                m_s(),
                m_pmsg(nullptr),
                m_rx_phase(IDLE){};

        [[maybe_unused]] void init(Message<std::string>* pmsg){
            if(pmsg != nullptr) {
                m_pmsg = pmsg;
                m_len = 0;
                m_s.clear();
                m_rx_phase = ID;
            }
        }

        bool receive(){
            if(m_rx_phase != IDLE && m_rx_phase != END)
            {
                m_len = m_streamer.rx_size();
                if(m_len <= 0) return false;

                char buffer[m_len+1];
                m_streamer.try_read(buffer, m_len);
                buffer[m_len] = 0;
                char* p = buffer;
                size_t ix = 0;

                while(ix < m_len){
                    switch(m_rx_phase){
                        case IDLE:
                        case END:
                            break;
                        case ID:
                            for(size_t i = 0; i < m_len - ix; ++i){
                                if(p[i] == 0){
                                    m_s += std::string(p);
                                    m_pmsg->setId(m_s);
                                    m_s.clear();
                                    m_rx_phase = NAME;
                                    m_streamer.consume_read(i+1);
                                    ix += i+1;
                                    p = &buffer[ix];
                                    break;
                                }
                            }
                            //No string delimiter found. Consumes all the bytes.
                            if(m_rx_phase == ID){
                                m_s += std::string(p);
                                m_streamer.consume_read(m_len);
                                ix = m_len;
                            }
                            break;
                        case NAME:
                            for(size_t i = 0; i < m_len - ix; ++i){
                                if(p[i] == 0){
                                    m_s += std::string(p);
                                    m_pmsg->setName(m_s);
                                    m_s.clear();
                                    m_rx_phase = ARGS_VALUE;
                                    m_streamer.consume_read(i+1);
                                    ix += i+1;
                                    p = &buffer[ix];
                                    break;
                                }
                            }
                            if(m_rx_phase == NAME){
                                m_s += std::string(p);
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                        case ARGS_VALUE:
                            for(size_t i = 0; i < m_len - ix; ++i){
                                if(p[i] == 0){
                                    m_s += std::string(p);
                                    m_pmsg->setValue(m_s);
                                    m_s.clear();
                                    m_rx_phase = END;
                                    m_streamer.consume_read(i+1);
                                    ix = m_len; // exit loop
                                    break;
                                }
                            }
                            if(m_rx_phase == ARGS_VALUE){
                                m_s += std::string(p);
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                    }

                }
                if(m_rx_phase == END){
                    m_rx_phase = IDLE;
                    return true;
                }
            }
            return false;
        }

    private:
        Streamer<char, C>& m_streamer;
        Message<std::string>* m_pmsg;
        std::string m_s;
        size_t m_len;
        enum RX_PHASE{
            IDLE = 0,
            ID,
            NAME,
            ARGS_VALUE,
            END
        }m_rx_phase = IDLE;
    };

    /**
     * Deserializer
     * [binary type]
     */

    template <typename C>
    class Deserializer<std::vector<unsigned char>, C>{
    public:

        [[maybe_unused]] explicit Deserializer(Streamer<unsigned char, C>& s):
                m_streamer(s),
                m_len(0),
                m_size(0),
                m_s(),
                m_pmsg(nullptr),
                m_rx_phase(IDLE){};

        void init(Message<std::vector<unsigned char>>* pmsg){
            if(pmsg != nullptr) {
                m_pmsg = pmsg;
                m_len = 0;
                m_s.clear();
                m_v.clear();
                m_rx_phase = ID;
            }
        }

        bool receive(){
            if(m_rx_phase != IDLE && m_rx_phase != END)
            {
                m_len = m_streamer.rx_size();
                if(m_len <= 0) return false;

                unsigned char buffer[m_len+1];
                m_streamer.try_read(buffer, m_len);
                buffer[m_len] = 0;//needed to terminate incomplete string
                unsigned char* p = buffer;
                size_t ix = 0;

                while(ix < m_len){
                    switch(m_rx_phase){
                        case IDLE:
                        case END:
                            break;
                        case ID:
                            if(m_len - ix + m_v.size() >= sizeof(uint16_t)){
                                size_t n = sizeof(uint16_t) - m_v.size();
                                m_v.insert(m_v.end(), p, p + n);
                                m_pmsg->setId(*reinterpret_cast<uint16_t*>(m_v.data()));
                                m_rx_phase = NAME;
                                m_streamer.consume_read(n);
                                m_v.clear();
                                ix += n;
                                p = &buffer[ix];
                            }
                            else //Not enough read bytes.Consumes read bytes and read more.
                            {
                                m_v.insert(m_v.end(), p, p + (m_len - ix));
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                        case NAME:
                            for(size_t i = 0; i < m_len - ix; ++i){
                                if(p[i] == 0){
                                    m_s += std::string(reinterpret_cast<const char*>(p));
                                    m_pmsg->setName(m_s);
                                    m_rx_phase = SIZE;
                                    m_streamer.consume_read(i+1);
                                    ix += i+1;
                                    p = &buffer[ix];
                                    break;
                                }
                            }
                            if(m_rx_phase == NAME){
                                m_s += std::string(reinterpret_cast<const char*>(p));
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                        case SIZE:
                            if(m_len - ix + m_v.size() >= sizeof(size_t)){
                                size_t n = sizeof(size_t) - m_v.size();
                                m_v.insert(m_v.end(), p, p + n);
                                m_size = *reinterpret_cast<size_t*>(m_v.data());
                                m_streamer.consume_read(n);
                                m_pmsg->resetValue();
                                if(m_size > 0)
                                {
                                    ix += n;
                                    p = &buffer[ix];
                                    m_rx_phase = ARGS_VALUE;
                                }
                                else
                                {
                                    ix = m_len;
                                    m_rx_phase = END;
                                }

                            }
                            else
                            {
                                m_v.insert(m_v.end(), p, p + (m_len - ix));
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                        case ARGS_VALUE:
                            if(m_len - ix + m_pmsg->getSize() >= m_size){
                                size_t n = m_size - m_pmsg->getSize();
                                m_pmsg->writeValue(reinterpret_cast<unsigned char*>(p), n);
                                m_rx_phase = END;
                                m_streamer.consume_read(n);
                                ix = m_len; // exit loop
                            }
                            else
                            {
                                m_pmsg->writeValue(reinterpret_cast<unsigned char*>(p), m_len - ix);
                                m_streamer.consume_read(m_len - ix);
                                ix = m_len;
                            }
                            break;
                    }

                }
                if(m_rx_phase == END){
                    m_rx_phase = IDLE;
                    return true;
                }
            }
            return false;
        }

    private:
        Streamer<unsigned char, C>& m_streamer;
        Message<std::vector<unsigned char>>* m_pmsg;
        std::string m_s;
        std::vector<unsigned char> m_v;
        size_t m_len;
        size_t m_size;
        enum RX_PHASE{
            IDLE = 0,
            ID,
            NAME,
            SIZE,
            ARGS_VALUE,
            END
        }m_rx_phase = IDLE;
    };

    template <typename C>
    using TextSerializer = Serializer<std::string, C>;
    template <typename C>
    using BinarySerializer = Serializer<std::vector<unsigned char>, C>;
    template <typename C>
    using TextDeserializer = Deserializer<std::string, C>;
    template <typename C>
    using BinaryDeserializer = Deserializer<std::vector<unsigned char>, C>;
    template <typename C>
    using DataSerializer = typename std::conditional<is_binary_protocol, BinarySerializer<C>, TextSerializer<C>>::type;
    template <typename C>
    using DataDeserializer = typename std::conditional<is_binary_protocol, BinaryDeserializer<C>, TextDeserializer<C>>::type;

}//namespace rpc
}//namespace bm




#endif // BMRPCSERIALIZER_H




