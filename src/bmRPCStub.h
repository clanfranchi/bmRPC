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


#ifndef BMRPCSTUB_H
#define BMRPCSTUB_H

namespace bm
{
namespace rpc
{
    /**
     * Skeleton
     * Server side
     */

    template <typename D>
    class Skeleton{
    public:

        Skeleton():r_format(RArgTypeId::VOID),invocation_id(0){};

        void dispatch(){
            invoke(func, this);
        }

        std::string& getName(){
            return id;
        }

        void unmarshall(Message<D>& msg){
            id = msg.getName();
            using TData = typename std::conditional<std::is_same_v<D,std::string>, std::string, std::vector<unsigned char>>::type;
            const TData& data = msg.getValue();
            unmarshall_args_in(data, in_args);
            if constexpr(std::is_same_v<D,std::string>)
                msg.getId(invocation_id);
            else
                invocation_id = msg.getId();
        }

        Message<D> marshall(){
            Message<D> msg;
            msg.setName(id);
            msg.setValue(out_args);
            msg.setId(invocation_id);
            return msg;
        }

    protected:
        template <typename T, typename E, typename C>
        friend class RpcServer;
        std::string id;
        std::vector<InArgTypeId> in_args_format;
        std::vector<OutArgTypeId> out_args_format;
        RArgTypeId r_format;
        using In_TData = typename std::conditional<std::is_same_v<D,std::string>, std::vector<std::string>, std::vector<unsigned char>>::type;
        In_TData in_args;
        using Out_TData = typename std::conditional<std::is_same_v<D,std::string>, std::string, std::vector<unsigned char>>::type;
        Out_TData out_args;
        uint16_t invocation_id{};
        std::function<void(Skeleton*)> func;
    };


    /**
     * Stub
     * Client side
     */

    struct invokation_data{
        uint16_t id;
        std::function<void(ReturnValue)> callback;
        std::vector<void*> out_args_addresses;
    };

    template <typename D>
    class Stub{
    public:

        Stub():r_format(RArgTypeId::WRONG),invokation_id(0){};

        Message<D> marshall(){
            Message<D> msg;
            msg.setName(id);
            msg.setValue(in_args);
            msg.setId(invokation_id);
            return msg;
        }

        void unmarshall_and_dispatch(Message<D>& msg){
            //unmarshall
            id = msg.getName();
            using TData = typename std::conditional<std::is_same_v<D,std::string>, std::string, std::vector<unsigned char>>::type;
            const TData& data = msg.getValue();
            unmarshall_out_args(r_format, data, r_arg, out_args);
            if constexpr(std::is_same_v<D,std::string>)
                msg.getId(invokation_id);
            else
                invokation_id = msg.getId();

            //dispatch
            auto pdata = invokation_list.begin();
            auto pre_pdata = invokation_list.before_begin();
            for (; pdata != invokation_list.end();) {
                if (pdata->id == invokation_id) {
                    break;
                }
                else{
                    pre_pdata = pdata;
                    ++pdata;
                }
            }
            ReturnValue r = ReturnValue();
            if(r_format != RArgTypeId::VOID)
                r = deserialize_r(r_format,r_arg);
            deserialize_out_args(out_args_format,out_args,pdata->out_args_addresses);
            if(pdata->callback)
                pdata->callback(r);
            invokation_list.erase_after(pre_pdata);
        }

        std::string& getName(){
            return id;
        }

    protected:
        template <typename T, typename E, typename C>
        friend class RpcClient;
        std::string id;
        std::vector<InArgTypeId> in_args_format;
        std::vector<OutArgTypeId> out_args_format;
        RArgTypeId r_format;
        using In_TData = typename std::conditional<std::is_same_v<D,std::string>, std::string, std::vector<unsigned char>>::type;
        In_TData in_args;
        using Out_TData = typename std::conditional<std::is_same_v<D,std::string>, std::vector<std::string>, std::vector<unsigned char>>::type;
        Out_TData out_args;
        using TDataItem = typename std::conditional<std::is_same_v<D,std::string>,std::string, std::vector<unsigned char>>::type;
        TDataItem r_arg;
        uint16_t invokation_id{};
        uint16_t invokations = 0;
        int n_handles = 0;
        std::forward_list<struct invokation_data> invokation_list;
    };


    /**
     * RpcHandle
     */

    template <typename T>
    class RpcHandle{
    public:

        explicit RpcHandle(T* stub): m_stub(stub){};

        T* getStub( ){
            return m_stub;
        }

        void setStub(T* handle){
            m_stub = handle;
        }

    private:
        T* m_stub;
    };

}//namespace rpc
}//namespace bm

#endif // BMRPCSTUB_H




