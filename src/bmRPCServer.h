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


#ifndef BMRPCSERVER_H
#define BMRPCSERVER_H


namespace bm
{
namespace rpc
{

    template <typename T, typename D, typename C>
    class RpcServer{

    public:

        explicit RpcServer(Comm<T,C>* com):
                m_com(com),
                m_streamer(Streamer<T, C>(m_com)),
                m_deserializer(DataDeserializer<C>(m_streamer)),
                m_serializer(DataSerializer<C>(m_streamer)),
                rx_msg_buffer(),
                tx_msg_buffer(),
                m_init_serializer(false),
                m_init_deserializer(false)
        {};


        template<typename R, typename... Args>
        Skeleton<D>* connect(const std::string& func_name, R(*func_address)(Args...)){
            auto f_lambda = [func_address](Skeleton<D>* p_rpc) {
                const size_t nargs = sizeof...(Args);
                std::vector<AnyArg> vec;
                vec = deserialize_in_args(p_rpc->in_args_format, p_rpc->in_args);
                if constexpr (std::is_same<R,void>::value)//constexpr is required here
                {
                    callProcWithArgs(func_address, vec, std::make_index_sequence<nargs>{});
                    if constexpr (std::is_same<D,std::string>::value){
                        std::ostringstream ss;
                        serialize_out_args<std::ostringstream>(p_rpc->in_args_format, vec,ss);
                        p_rpc->out_args =  ltrim(ss.str());
                    }
                    else
                    {
                        p_rpc->out_args.clear();
                        serialize_out_args<std::vector<unsigned char>>(p_rpc->in_args_format, vec, p_rpc->out_args);
                    }
                }
                else
                {
                    auto returned_value = callFuncWithArgs(func_address, vec, std::make_index_sequence<nargs>{});
                    auto val = AnyArg(returned_value);
                    if constexpr (std::is_same<D,std::string>::value){
                        std::ostringstream ss;
                        serialize_r<std::ostringstream>(p_rpc->r_format, val,ss);
                        serialize_out_args<std::ostringstream>(p_rpc->in_args_format, vec,ss);
                        p_rpc->out_args =  ltrim(ss.str());
                    }
                    else
                    {
                        std::vector<unsigned char> ss;
                        serialize_r<std::vector<unsigned char>>(p_rpc->r_format, val,ss);
                        serialize_out_args<std::vector<unsigned char>>(p_rpc->in_args_format, vec,ss);
                        p_rpc->out_args = ss;
                    }
                }
            };
            Skeleton<D> rpc;
            rpc.func = f_lambda;
            rpc.in_args_format = codify_in_args<Args...>();
            rpc.out_args_format = codify_out_args<Args...>();
            rpc.r_format = ParamType<R>::r_id;
            std::string sargs = stringify<Args...>();
            rpc.id = std::string(ParamType<R>::name) + " " + func_name + " " + sargs;
            Skeleton<D>* p_rpc = registry.insert(rpc);
            return p_rpc;
        }

        [[maybe_unused]] void disconnect( Skeleton<D>* rpc){
            registry.remove(rpc);
        }

        void initLoop(){
            m_init_serializer = true;
            m_init_deserializer = true;
        }


        void doLoop(){
            TimeOut_t rx_msg_tout;
            static Message<D> tx_msg;
            static Message<D> rx_msg;

            rx_msg_tout.preset(SERVER_LOOP_TOUT_MS);
            rx_msg_tout.start();
            while(!rx_msg_tout.expired()){
                if(m_init_deserializer)
                {
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
                Skeleton<D>* rpc = registry.get(msg.getName());
                if(rpc!= nullptr) {
                    rpc->unmarshall(msg);
                    rpc->dispatch();
                    msg = rpc->marshall();
                    tx_msg_buffer.push(msg);
                }
                rx_msg_buffer.pop();
            }

            TimeOut_t tx_msg_tout;
            tx_msg_tout.preset(SERVER_LOOP_TOUT_MS);
            tx_msg_tout.start();
            while(!tx_msg_buffer.empty() && !tx_msg_tout.expired()){
                if(m_init_serializer){
                    tx_msg  = tx_msg_buffer.front();
                    m_serializer.init(&tx_msg);
                    m_init_serializer = false;
                }
                if(m_serializer.send()){
                    tx_msg_buffer.pop();
                    m_init_serializer = true;
                }
            }
            m_streamer.flush();
        }

    private:
        std::queue<Message<D>> tx_msg_buffer;
        std::queue<Message<D>> rx_msg_buffer;
        bool m_init_deserializer;
        bool m_init_serializer;
        Comm<T,C>* m_com;
        FunctionsRegistry<Skeleton<D>> registry;
        Streamer<T, C> m_streamer;
        DataDeserializer<C> m_deserializer;
        DataSerializer<C> m_serializer;
    };

}//namespace rpc
}//namespace bm



#endif // BMRPCSERVER_H




