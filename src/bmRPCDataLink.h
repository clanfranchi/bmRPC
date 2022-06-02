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


#ifndef BMRPCDATALINK_H
#define BMRPCDATALINK_H


namespace bm
{
namespace rpc
{

    /**
    *  Com Interface
    */

    //Use curiously recurring template pattern (CRTP) for static polymorphism
    //This technique achieves a similar effect to the use of virtual functions,
    // without the costs (and some flexibility) of dynamic polymorphism.
    template <typename T, typename C>
    class Comm{
    public:
        INLINE int open(){
            return static_cast<C*>(this)->open_impl();
        }

        [[maybe_unused]] INLINE void close(){
            return static_cast<C*>(this)->close_impl();
        }

        INLINE bool is_open() {
            return static_cast<C*>(this)->is_open_impl();
        }

        INLINE size_t get_packet_size(){
            return static_cast<C*>(this)->get_packet_size_impl();
        }

        INLINE size_t write(const T buf[], size_t len) {
            return static_cast<C*>(this)->write_impl(buf,len);
        }

        INLINE size_t read(T buf[], size_t len) {
            return static_cast<C*>(this)->read_impl(buf,len);
        }
    };

    /**
    *  SharedBuffer
    */

    template <typename T>
    class SharedBuffer{
    public:

        #define MAX_BUFFER_SIZE 256
        SharedBuffer() {
            in_queue = new std::array<T,MAX_BUFFER_SIZE>();
            out_queue = new std::array<T,MAX_BUFFER_SIZE>();
        }

        ~SharedBuffer() {
            delete in_queue;
            delete out_queue;
        }

        [[maybe_unused]] bool srv_can_read(){
            return (in_queue->empty() == false);
        }

        size_t srv_read(T b[], size_t len){
            if(in_lock || in_len <= 0 || len <=0 ){
                return 0;
            }
            if(len > in_len) len = in_len;
            in_lock = true;
            std::copy(in_queue->begin(), in_queue->begin() + len, b);
            if(len != in_len)//move remaining elements at the beginning of the array
                std::move(in_queue->begin() + len,in_queue->begin() + in_len, in_queue->begin());
            in_len -= len;
            in_lock = false;
            return len;
        }

        size_t srv_write(const T b[], size_t len){
            if(!out_lock && len > 0){
                size_t capacity = out_queue->size() - out_len;
                if(len > capacity){
                    len = capacity;
                }
                out_lock = true;
                std::copy(b, b + len, out_queue->begin()+out_len);
                out_len += len;
                out_lock = false;
                return len;
            }
            return 0;
        }

        [[maybe_unused]] bool cln_can_read(){
            return (out_queue->empty() == false);
        }

        size_t cln_read(T b[], size_t len){
            if(out_lock || out_len <= 0 || len <= 0){
                return 0;
            }
            if(len > out_len) len = out_len;
            out_lock = true;
            std::copy(out_queue->begin(), out_queue->begin() + len, b);
            if(len != out_len)//move remaining elements at the beginning of the array
                std::move(out_queue->begin() + len,out_queue->begin() + out_len, out_queue->begin());
            out_len -= len;
            out_lock = false;
            return len;
        }

        size_t cln_write(const T b[], size_t len){
            if(!in_lock && len > 0){
                size_t capacity = in_queue->size() - in_len;
                if(len > capacity){
                    len = capacity;
                }
                in_lock = true;
                std::copy(b, b + len, in_queue->begin() + in_len);
                in_len += len;
                in_lock = false;
                return len;
            }
            return 0;
        }

    private:
        //Queues directions are relative to the server side
        std::array<T,MAX_BUFFER_SIZE>* in_queue;
        size_t in_len = 0;
        bool in_lock = false;
        std::array<T,MAX_BUFFER_SIZE>* out_queue;
        size_t out_len = 0;
        bool out_lock = false;
    };


    /**
    *  ServerCom
    */

    template <typename T>
    class ServerCom: public Comm<T,ServerCom<T>> {
    public:

        explicit ServerCom(SharedBuffer<T>& sb):sb(sb),m_open(false){};

        int open_impl() {
            if(m_open) return -1;
            m_open = true;
            return 0;
        }

        void close_impl() {
            m_open = false;
        }

        bool is_open_impl() {
            return m_open;
        }

        size_t get_packet_size_impl() {
            return m_max_packet_size;
        }

        size_t write_impl(const T buf[], size_t len) {
            return sb.srv_write(buf, len);
        }

        size_t read_impl(T buf[], size_t len) {
            return sb.srv_read(buf, len);
        }

    private:
        SharedBuffer<T>& sb;
        const size_t m_max_packet_size = 8;
        bool m_open;
    };

    /**
    *  ClientCom
    */

    template <typename T>
    class ClientCom: public Comm<T,ClientCom<T>> {
    public:

        explicit ClientCom(SharedBuffer<T>& sb):sb(sb),m_open(false){};

        int open_impl() {
            if(m_open) return -1;
            m_open = true;
            return 0;
        }

        void close_impl() {
            m_open = false;
        }

        bool is_open_impl() {
            return m_open;
        }

        size_t get_packet_size_impl() {
            return m_max_packet_size;
        }

        size_t write_impl(const T buf[], size_t len) {
            return sb.cln_write(buf, len);
        }

        size_t read_impl(T buf[], size_t len) {
            return sb.cln_read(buf, len);
        }

    private:
        SharedBuffer<T>& sb;
        const size_t m_max_packet_size = 8;
        bool m_open;
    };

    /**
    *  EmptyCom
    *  Empty com example to be filled with your implementation
    */

    template <typename T>
    class EmptyCom: public Comm<T,EmptyCom<T>> {
    public:

        EmptyCom()=default;

        int open_impl() {
            return 0;
        }

        void close_impl() {
        }

        bool is_open_impl() {
            return 0;
        }

        size_t get_packet_size_impl() {
            return m_max_packet_size;
        }

        size_t write_impl(const T buf[], size_t len) {
            return 0;
        }

        size_t read_impl(T buf[], size_t len) {
            return 0;
        }

    private:
        const size_t m_max_packet_size = 8;
    };

}//namespace rpc
}//namespace bm


#endif // BMRPCDATALINK_H




