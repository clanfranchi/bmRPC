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


#ifndef BMRPCCLIENT_H
#define BMRPCCLIENT_H

namespace bm
{
namespace rpc
{
    template <typename T, typename D, typename C>
    class RpcClient{

    public:
        explicit RpcClient(Comm<T,C>* com):
                m_com(com),
                m_streamer(Streamer<T, C>(m_com)),
                m_deserializer(DataDeserializer<C>(m_streamer)),
                m_serializer(DataSerializer<C>(m_streamer)),
                rx_msg_buffer(),
                tx_msg_buffer(),
                m_init_serializer(false),
                m_init_deserializer(false)
        {};

        template <typename R, typename...Args>
        [[maybe_unused]] decltype(auto) connect(const char* name, R(*func)(Args...)){
            //Check the validity of the function signature
            using Traits = ParamTraits<decltype(func)>;
            const size_t nargs = sizeof...(Args);
            static_assert(Traits::arity == nargs, "Wrong parameters number!");
            static_assert(Traits::valid, "Not supported function signature!");

            Stub<D>* p_rpc;
            if((p_rpc = registry.find(name)) == nullptr){
                //new rpc
                Stub<D> rpc;
                rpc.r_format = ParamType<R>::r_id;
                rpc.in_args_format = codify_in_args<Args...>();
                rpc.out_args_format = codify_out_args<Args...>();
                std::string sargs = stringify<Args...>();
                rpc.id = std::string(ParamType<R>::name) + " " + name + " " + sargs;
                rpc.n_handles = 1;
                p_rpc = registry.insert(rpc);//insert rpc or get its pointer if already present
            }
            else
                p_rpc->n_handles++;

            auto handle = RpcHandle<Stub<D>>(p_rpc);
            return handle;
        }

        void disconnect(RpcHandle<Stub<D>>& handle){
            auto  p = handle.getStub();
            if(p != nullptr){
                p->n_handles--;
                if(p->n_handles <= 0){
                    registry.remove(p);
                }
                handle.setStub(nullptr);
            }
        }

        template <typename F, typename...Args>
        bool asyncRPC(RpcHandle<Stub<D>>& handle, std::function<void(ReturnValue)>&& callback, Args&&... args){
            const size_t nargs = sizeof...(Args);
            using Traits = ParamTraits<F>;

            //Checks the validity of the function signature
            static_assert(Traits::arity == nargs, "Wrong parameters number!");
            static_assert(Traits::valid, "Not supported function signature!");

            //Checks the validity of the submitted arguments
            static constexpr bool is_supported = ((ArgType<typename remove_cvref<Args>::type>::valid) && ...);
            static_assert(is_supported, "Wrong arguments types!");

            Stub<D>* rpc = handle.getStub();
            if(rpc != nullptr && !tx_msg_full()){
                rpc->invokations++;
                rpc->invokation_id = rpc->invokations;
                invokation_data data;
                data.id = rpc->invokation_id;

                if constexpr(std::is_same_v<D,std::string>)
                {
                    std::ostringstream ss;
                    serialize_args<F>(data.out_args_addresses,ss,std::forward<Args>(args)...);
                    rpc->in_args = ltrim(ss.str());
                }
                else//std::vector<unsigned char>
                {
                    rpc->in_args.clear();
                    serialize_args<F>(data.out_args_addresses,rpc->in_args,std::forward<Args>(args)...);
                }

                data.callback = move(callback);
                auto before_end = rpc->invokation_list.before_begin();
                for (auto& _ : rpc->invokation_list)
                    ++before_end;
                rpc->invokation_list.insert_after(before_end, data);

                Message<D> msg = rpc->marshall();
                tx_msg_buffer.push(msg);
                return true;
            }
            return false;
        }

        #define EMPTY_CB std::function<void(ReturnValue)>()
        #define ASYNC_RPC_WITH_CB(f, handle,callback,args...) asyncRPC<decltype(f)>(handle, callback, args)
        #define ASYNC_RPC(f, handle,args...) asyncRPC<decltype(f)>(handle, EMPTY_CB, args)

        void initLoop(){
            m_init_serializer = true;
            m_init_deserializer = true;
        }

        void doLoop(){
            TimeOut_t tx_msg_tout;
            static Message<D> tx_msg;
            static Message<D> rx_msg;

            tx_msg_tout.preset(CLIENT_LOOP_TOUT_MS);
            tx_msg_tout.start();
            while(!tx_msg_buffer.empty() && !tx_msg_tout.expired()){
                if(m_init_serializer){
                    tx_msg = tx_msg_buffer.front();
                    m_serializer.init(&tx_msg);
                    m_init_serializer = false;
                }
                if(m_serializer.send()){
                    tx_msg_buffer.pop();
                    m_init_serializer = true;
                }
            }
            m_streamer.flush();

            TimeOut_t rx_msg_tout;
            rx_msg_tout.preset(CLIENT_LOOP_TOUT_MS);
            rx_msg_tout.start();
            while(!rx_msg_tout.expired()){
                if(m_init_deserializer) {
                    m_deserializer.init(&rx_msg);
                    m_init_deserializer = false;
                }
                if(m_deserializer.receive()) {
                    rx_msg_buffer.push(rx_msg);
                    m_init_deserializer = true;
                }
            }

            while(!rx_msg_buffer.empty()){
                Message<D> msg = rx_msg_buffer.front();
                Stub<D>* rpc = registry.get(msg.getName());
                rpc->unmarshall_and_dispatch(msg);
                rx_msg_buffer.pop();
            }
        }

    private:
        FunctionsRegistry<Stub<D>> registry;
        std::queue<Message<D>> tx_msg_buffer;
        std::queue<Message<D>> rx_msg_buffer;
        bool m_init_deserializer;
        bool m_init_serializer;
        Comm<T,C>* m_com;
        Streamer<T, C> m_streamer;
        DataDeserializer<C> m_deserializer;
        DataSerializer<C> m_serializer;
        bool tx_msg_full(){
            return tx_msg_buffer.size() > MAX_CLIENT_MSG_BUFFER_SIZE;
        }
    };

}//namespace rpc
}//namespace bm

#endif // BMRPCCLIENT_H




