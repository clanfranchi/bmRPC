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


#ifndef BMRPCSTREAMER_H
#define BMRPCSTREAMER_H

namespace bm
{
namespace rpc
{
    template <class T, class C>
    class Streamer {
    public:

       explicit Streamer(Comm<T,C>* com) :
                m_com(com),
                m_packet_size(com->get_packet_size()){
            m_tx_buffer = new T[STREAMER_BUFFER_SIZE];
            m_rx_buffer = new T[STREAMER_BUFFER_SIZE];
        };

        virtual ~Streamer() {
            delete[] m_tx_buffer;
            delete[] m_rx_buffer;
        }

        typedef std::make_signed_t<std::size_t> s_size_t;

        size_t write(const T items[], size_t size){
            if(!tx_full())//to avoid overflow
            {
                size_t available_size;
                if(tx_empty()) available_size = m_max_size;
                else  available_size = (size_t)((m_tx_out-m_tx_in)&(m_max_size-1));//requires the buffer size of a power of two
                if(size > available_size) size = available_size;

                if(m_tx_out > m_tx_in)
                    memcpy(&m_tx_buffer[m_tx_in], items, size);
                else
                {
                    s_size_t count;
                    if((count = m_tx_in + size - m_max_size) > 0)
                    {
                        memcpy(&m_tx_buffer[m_tx_in], items, size - count);
                        memcpy(&m_tx_buffer[0], &items[size - count], count);
                    }
                    else
                        memcpy(&m_tx_buffer[m_tx_in], items, size);
                }
                inc(m_tx_in,size);
                m_tx_full = (m_tx_in == m_tx_out);
            }
            else
                size = 0;

            flush();

            return size;
        }

        void flush(){
            if(m_com->is_open() && !tx_empty()){
                size_t used_size;
                size_t n_written;
                if(tx_full())
                    used_size = m_max_size;
                else
                    used_size = (size_t)((m_tx_in-m_tx_out)&(m_max_size-1));
                do{
                    size_t size;
                    if(used_size >= m_packet_size) size = m_packet_size;
                    else size = used_size;
                    s_size_t count;
                    if((count = m_tx_out + size - m_max_size) > 0){
                        n_written = m_com->write(&m_tx_buffer[m_tx_out], size - count);
                        if(n_written > 0) {
                            inc(m_tx_out, n_written);
                            m_tx_full = false;
                            used_size -= n_written;
                            n_written = m_com->write(m_tx_buffer, count);
                            if(n_written > 0) {
                                inc(m_tx_out, n_written);
                                m_tx_full = false;
                                used_size -= n_written;
                            }
                        }
                    }
                    else{
                        n_written = m_com->write(&m_tx_buffer[m_tx_out], size);
                        if(n_written > 0) {
                            inc(m_tx_out, n_written);
                            m_tx_full = false;
                            used_size -= n_written;
                        }
                    }
                }while(used_size > 0 && n_written > 0);
            }
        }

        size_t read(T items[], size_t size){
            if(rx_empty()){
                return 0;
            }

            size_t available_size;
            if(rx_full()) available_size = m_max_size;
            else  available_size = (size_t)((m_rx_in-m_rx_out)&(m_max_size-1));
            if(size > available_size) size = available_size;

            if(m_rx_in > m_rx_out)
                memcpy(items, &m_rx_buffer[m_rx_out], size);
            else
            {
                s_size_t count;
                if((count = m_rx_out + size - m_max_size) > 0)
                {
                    memcpy(items, &m_rx_buffer[m_rx_out], size - count);
                    memcpy(&items[size - count],&m_rx_buffer[0], count);
                }
                else
                    memcpy(items, &m_rx_buffer[m_rx_out], size);
            }
            m_rx_full = false;
            inc(m_rx_out, size);
            return size;
        }

        size_t try_read(T items[], size_t size){
            if(rx_empty()){
                return 0;
            }

            size_t available_size;
            if(rx_full()) available_size = m_max_size;
            else  available_size = (size_t)((m_rx_in-m_rx_out)&(m_max_size-1));
            if(size > available_size) size = available_size;

            if(m_rx_in > m_rx_out)
                memcpy(items, &m_rx_buffer[m_rx_out], size);
            else
            {
                s_size_t count;
                if((count = m_rx_out + size - m_max_size) > 0)
                {
                    memcpy(items, &m_rx_buffer[m_rx_out], size - count);
                    memcpy(&items[size - count],&m_rx_buffer[0], count);
                }
                else
                    memcpy(items, &m_rx_buffer[m_rx_out], size);
            }
            return size;
        }

        size_t consume_read(size_t size){
            if(rx_empty()){
                return 0;
            }

            size_t available_size;
            if(rx_full()) available_size = m_max_size;
            else  available_size = (size_t)((m_rx_in-m_rx_out)&(m_max_size-1));
            if(size > available_size) size = available_size;

            m_rx_full = false;
            inc(m_rx_out, size);
            return size;
        }

        [[maybe_unused]] void reset(){
            m_tx_in = m_tx_out;
            m_tx_full = false;
            m_rx_in = m_rx_out;
            m_rx_full = false;
        }

        [[nodiscard]] bool tx_empty() const {
            //if m_in and m_out are equal the buffer is empty
            return (!m_tx_full && (m_tx_in == m_tx_out));
        }

        [[nodiscard]] bool rx_empty() {
            com_read();//read from com
            return (!m_rx_full && (m_rx_in == m_rx_out));
        }

        [[nodiscard]] INLINE bool tx_full() const {
            return m_tx_full;
        }

        [[nodiscard]] INLINE bool rx_full() const {
            return m_rx_full;
        }

        [[maybe_unused]] [[nodiscard]] INLINE size_t capacity() const {
            return m_max_size;
        }

        [[maybe_unused]] [[nodiscard]] size_t tx_size() const {
            size_t size = m_max_size;
            if(!m_tx_full)
            {
                if(m_tx_in >= m_tx_out)
                    size = m_tx_in - m_tx_out;
                else
                    size = m_max_size + m_tx_in - m_tx_out;
            }
            return size;
        }

        [[nodiscard]] size_t rx_size() {
            com_read();
            size_t size = m_max_size;
            if(!m_rx_full)
            {
                if(m_rx_in >= m_rx_out)
                    size = m_rx_in - m_rx_out;
                else
                    size = m_max_size + m_rx_in - m_rx_out;
            }
            return size;
        }


    private:

        void com_read(){
            size_t count, len;
            if(!rx_full()){//to avoid overflow
                if(m_rx_in >= m_rx_out)
                {
                    len = m_max_size - m_rx_in;
                    count = m_com->read(&m_rx_buffer[m_rx_in], len);
                    if(count > 0){
                        inc(m_rx_in,count);
                        m_rx_full = (m_rx_in == m_rx_out);//dbg
                        if(count == len){
                            len = m_rx_out;
                            count = m_com->read(m_rx_buffer, len);
                            if(count>0){
                                inc(m_rx_in,count);
                                m_rx_full = (m_rx_in == m_rx_out);
                            }
                        }
                    }
                }
                else
                {
                    len = m_rx_out - m_rx_in;
                    count = m_com->read(&m_rx_buffer[m_rx_in], len);
                    if(count > 0){
                        inc(m_rx_in,count);
                        m_rx_full = (m_rx_in >= m_rx_out);
                    }
                }
            }
        }

        inline void inc(size_t& p, size_t v)
        {
            p += v;
            if(p >= m_max_size)
                p -= m_max_size;
        }
        [[maybe_unused]] inline void dec(size_t& p, size_t v)
        {
            p -= v;
            if(p < 0)
                p += m_max_size;
        }

        //tx
        T* m_tx_buffer;
        size_t m_tx_in = 0;
        size_t m_tx_out = 0;
        bool m_tx_full = false;

        //rx
        T* m_rx_buffer;
        size_t m_rx_in = 0;
        size_t m_rx_out = 0;
        bool m_rx_full = false;

        //common
        const size_t m_max_size = STREAMER_BUFFER_SIZE;
        Comm<T,C>* m_com;
        size_t m_packet_size;
    };

}//namespace rpc
}//namespace bm

#endif // BMRPCSTREAMER_H



