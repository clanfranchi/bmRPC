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



#include "bmRPC.h"
#include "bmRPCTest.h"
#include <thread>// required for sleep_for

using namespace std;
using namespace bm;
using namespace rpc;


#if BMRPC_SERVER
    int fsm(int& state, long actuator, float& sensor, string& error_description){
        //Finite State Machine implementation
        int fsm_errno = 0;
        return fsm_errno;
    }
#endif

#if BMRPC_CLIENT
    static int state;
    static float sensor;
    static string error_description;
#if BMRPC_SERVER == false
        [[maybe_unused]]  int (*fsm)(int& state, long actuator, float& sensor, string& error_description);
#endif
#endif



int main() {

    /****
    * Server
    */
    #ifdef LOOP_BACK_TEST
        //Server side
        SharedBuffer<DataItem> shared_buffer = SharedBuffer<DataItem>();
    #endif

    #if BMRPC_SERVER
        #ifdef LOOP_BACK_TEST
            ServerCom<DataItem> server_com = ServerCom(shared_buffer);
            RpcServer server = CREATE_SERVER(ServerCom<DataItem>, server_com);
        #else
            //define your server_com
            EmptyCom<DataItem> server_com = EmptyCom<DataItem>();
            RpcServer my_server = CREATE_SERVER(EmptyCom<DataItem>, server_com);
        #endif
        server_com.open();
    #endif

    #if BMRPC_CLIENT
        #ifdef LOOP_BACK_TEST
            ClientCom<DataItem> client_com = ClientCom(shared_buffer);
            RpcClient client = CREATE_CLIENT(ClientCom<DataItem>, client_com);
        #else
            //define your client_com
            EmptyCom<DataItem> client_com = EmptyCom<DataItem>();
            RpcClient my_client = CREATE_CLIENT(EmptyCom<DataItem>, client_com);
        #endif
        client_com.open();
    #endif


#if BMRPC_CLIENT
    client.initLoop();
#endif
#if BMRPC_SERVER
    server.initLoop();
#endif

    #if BMRPC_CLIENT
        RpcHandle<Stub<Data>> fsm_stub = client.CONNECT(fsm);
    #endif
    #if BMRPC_SERVER
        Skeleton<Data>* fsm_skeleton = server.CONNECT(fsm);
    #endif

#if BMRPC_CLIENT

    client.ASYNC_RPC_WITH_CB(fsm, fsm_stub, [&](ReturnValue r) {
        if (r.valid())
            cout << "fsm errorno: " << r.get_value<int>() << endl;
        //implement callback logic
    }, state, (long)0xAA556432, sensor, error_description);

#endif

    TimeOutChrono tout;
    tout.preset(50); //50 ms
    tout.start();
    while (!tout.expired())
    {
        #if BMRPC_SERVER
            server.doLoop();
        #endif
        #if BMRPC_CLIENT
            client.doLoop();
        #endif
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    #if BMRPC_CLIENT
        client.disconnect(fsm_stub);
    #endif

    #if BMRPC_SERVER
        server.disconnect(fsm_skeleton);
    #endif

    test();//executes bmRPCTest cases

}//end main
