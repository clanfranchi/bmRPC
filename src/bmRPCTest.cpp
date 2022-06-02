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
#include <thread>// required for sleep_for
#include "bmRPCTest.h"

using namespace std;
using namespace bm;
using namespace rpc;


#if BMRPC_SERVER
    #ifdef LOOP_BACK_TEST
        static RpcServer<DataItem,Data, ServerCom<DataItem>>* server;
    #else
        //define your server_com
        static RpcServer<DataItem,Data, EmptyCom<DataItem>>* server;
    #endif
#endif

#if BMRPC_CLIENT
    #ifdef LOOP_BACK_TEST
        static RpcClient<DataItem,Data,ClientCom<DataItem>>* client;
    #else
        //define your client_com
        static RpcClient<DataItem,Data,EmptyCom<DataItem>>* client;
    #endif
#endif

static int total_passed_test = 0;

#ifdef  TEST_F0
    #if BMRPC_SERVER
        #if P64
            void f0_srv(int a, double b, float& c) { //unsigned long c = stol(*s);
        #else
            void f0_srv(int a, float b, float& c) { //unsigned long c = stol(*s);
        #endif
            cout << "f0_srv before: " << a << " " << b << " " << c << endl;
            c = (float)(a + b + c);
            cout << "f0_srv after: " << a << " " << b << " " << c << endl;
        }
        #if P64
            void (*f0)(int a, double b, float& c) = f0_srv;
        #else
            void (*f0)(int a, float b, float& c) = f0_srv;
        #endif
            Skeleton<Data>* pf0rpc;
            void test_f0_srv(){
                //server->connect("f0",f0);
                pf0rpc = server->CONNECT(f0);
            }
            void end_test_f0_srv() {
                server->disconnect(pf0rpc);
            }
    #endif//BMRPC_SERVER

#if BMRPC_CLIENT
    #if BMRPC_SERVER == false
        #if P64
            [[maybe_unused]]  void (*f0)(int a, double b, float& c);
        #else
            [[maybe_unused]]  void (*f0)(int a, float b, float& c);
        #endif
    #endif
    RpcHandle<Stub<Data>>* pf0h;
    #define F0_a 64
    #define F0_b 12355.1
    #define F0_c 55.5
    void init_test_f0_cln() {
        static RpcHandle<Stub<Data>> f0h = client->connect("f0", f0);
        pf0h = &f0h;//needed for the disconnect
    }
    void test_f0_cln(){
        static int a = F0_a;
    #if P64
        static double b = F0_b;
    #else
        static float b = F0_b;
    #endif
        static float c = F0_c;
        client->ASYNC_RPC_WITH_CB(f0, *pf0h,[&]( ReturnValue r){
            if(r.valid())
                cout << "f0_cln RV: " << r.get_value<int>() << endl;
            else
                cout << "f0_cln: invalid RPC return type" << endl;
            cout << "f0_cln: " << a << " " << b << " " << c << " " << endl;
            if(c == (float)a+(float)b+F0_c) {
                cout << "Test f0_cln: PASSED!" << endl;
                total_passed_test++;
            }
            else
                cout << "Test f0_cln: FAILED!" << endl;
        }, a, b, c);
    }
    void end_test_f0_cln() {
        client->disconnect(*pf0h);
    }
    #endif//BMRPC_CLIENT
#endif//TEST_F0

#ifdef TEST_F1
    #define F1_a 64
    #if BMRPC_SERVER
        void f1_srv(int a){
            cout << "f1_srv: " << a << endl;
            if(a == F1_a) {
                cout << "Test f1_srv: PASSED!" << endl;
                total_passed_test++;
            }
            else
                cout << "Test f1_srv: FAILED!" << endl;
        }
        void (*f1)(int a) = f1_srv;
        Skeleton<Data>* pf1rpc;
        void test_f1_srv(){
            pf1rpc = server->CONNECT(f1);
        }
        void end_test_f1_srv() {
            server->disconnect(pf1rpc);
        }
    #endif
    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            [[maybe_unused]]   void (*f1)(int a);
        #endif
        RpcHandle<Stub<Data>>* pf1h;
        void init_test_f1_cln(){
            static RpcHandle<Stub<Data>> f1h = client->connect("f1",f1);
            pf1h = &f1h;
        }
        void test_f1_cln(){
            static int a = F1_a;
            client->ASYNC_RPC(f1, *pf1h, a);
        }
        void end_test_f1_cln() {
            client->disconnect(*pf1h);
        }
    #endif
#endif//TEST_F1


#ifdef TEST_F2
    #if BMRPC_SERVER
        #if P64
            long f2_srv(int a, double b, double& c){
        #else
            long f2_srv(int a, float b, float& c){
        #endif
            cout << "f2_srv before: " << a << " " << b  << " " << c << endl;
        #if P64
            c = (double)(a+b+c);
        #else
            c = (float)(a+b+c);
        #endif
            a = 123;
            cout << "f2_srv after: " << a << " " << b  << " " << c << endl;
            return (long)c*2;
        }
        #if P64
            long (*f2)(int a, double b, double& c) = f2_srv;
        #else
            long (*f2)(int a, float b, float& c) = f2_srv;
        #endif
            Skeleton<Data>* pf2rpc;
            void test_f2_srv(){
                pf2rpc = server->CONNECT(f2);
            }
            void end_test_f2_srv() {
                server->disconnect(pf2rpc);
            }
    #endif//BMRPC_SERVER

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
        #if P64
            [[maybe_unused]]  long (*f2)(int a, double b, double& c);
        #else
             [[maybe_unused]]  long (*f2)(int a, float b, float& c);
        #endif
    #endif
        RpcHandle<Stub<Data>>* pf2h;
        #define F2_a 33
        #define F2_b 12345.1
        #define F2_c 55.3
        void init_test_f2_cln() {
            static RpcHandle<Stub<Data>> f2h = client->connect("f2", f2);
            pf2h = &f2h;
        }
        void test_f2_cln(){
            static int a = F2_a;
        #if P64
            static double b = F2_b;
            static double c = F2_c;
        #else
            static float b = F2_b;
            static float c = F2_c;
        #endif
            client->ASYNC_RPC_WITH_CB(f2, *pf2h,[&]( ReturnValue r){
                if(r.valid())
                    cout << "f2_cln RV: " << r.get_value<long>() << endl;
                else
                    cout << "f2_cln: invalid RPC return type" << endl;
                cout << "f2_cln: " << a << " " << b <<  " " << c << endl;
                if((c >= a+b+F2_c-0.1) && (c <= a+b+F2_c+0.1) && (r.get_value<long>() == 2*(long)c) && a == F2_a) {
                    cout << "Test f2_cln: PASSED!" << endl;
                    total_passed_test++;
                }
                else
                    cout << "Test f2_cln: FAILED!" << endl;
            }, a,b,c);
        }
        void end_test_f2_cln() {
            client->disconnect(*pf2h);
        }
    #endif
#endif//TEST_F2

#ifdef TEST_F3
    #if BMRPC_SERVER
        #if P64
            double f3_srv(char c, bool b, short s, int i, long l, long long ll, float f, double d){
                cout << "f3_srv: " << c << " " << b << endl;
                cout << "f3_srv: " << s << " " << i << endl;
                cout << "f3_srv: " << l << " " << ll << endl;
                cout << "f3_srv: " << f << " " << d << endl;
                return (double)(c+b+s+i+l+ll+f+d);
            }
            double (*f3)(char c, bool b, short s, int i, long l, long long ll, float f, double d) = f3_srv;
        #else
            float f3_srv(char c, bool b, short s, int i, long l, float f){
                    cout << "f3_srv: " << c << " " << b << endl;
                    cout << "f3_srv: " << s << " " << i << endl;
                    cout << "f3_srv: " << l << " " << f << endl;
                    return (float)(c+b+s+i+l+f);
                }
                float (*f3)(char c, bool b, short s, int i, long l, float f) = f3_srv;
        #endif
            Skeleton<Data>* pf3rpc;
            void test_f3_srv(){
                pf3rpc = server->CONNECT(f3);
            }
            void end_test_f3_srv() {
                server->disconnect(pf3rpc);
            }
    #endif

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            #if P64
                [[maybe_unused]]  double (*f3)(char c, bool b, short s, int i, long l, long long ll, float f, double d);
            #else
                [[maybe_unused]]  float (*f3)(char c, bool b, short s, int i, long l, float f);
            #endif
        #endif
        RpcHandle<Stub<Data>>* pf3h;
        #define F3_c 'a'
        #define F3_b 1
        #define F3_s 11
        #define F3_i 22
        #define F3_l 33
        #define F3_ll 44
        #define F3_f 55.6
        #define F3_d 66.5
        void init_test_f3_cln() {
            static RpcHandle<Stub<Data>> f3h = client->connect("f3", f3);
            pf3h = &f3h;
        }
        #if P64
        void test_f3_cln(){
            static char c = F3_c;
            static bool b = F3_b;
            static short s = F3_s;
            static int i = F3_i;
            static long l = F3_l;
            static long long ll = F3_ll;
            static float f = F3_f;
            static double d = F3_d;

            client->ASYNC_RPC_WITH_CB(f3, *pf3h,[&]( ReturnValue r){
                if(r.valid())
                    cout << "f3_cln RV: " << r.get_value<double>() << endl;
                else
                    cout << "f3_cln: invalid RPC return type" << endl;
                cout << "f3_cln: " << c << " " << b << endl;
                cout << "f3_cln: " << s << " " << i << endl;
                cout << "f3_cln: " << l << " " << ll << endl;
                cout << "f3_cln: " << f << " " << d << endl;
                if((r.get_value<double>() >= (double)(c+b+s+i+l+ll+f+d-0.1)) && (r.get_value<double>() <= (double)(c+b+s+i+l+ll+f+d+0.1))) {
                    cout << "Test f3_cln: PASSED!" << endl;
                    total_passed_test++;
                }
                else
                    cout << "Test f3_cln: FAILED!" << endl;
            },c,b,s,i,l,ll,f,d);
        }
        #else
        void test_f3_cln(){
                static char c = F3_c;
                static bool b = F3_b;
                static short s = F3_s;
                static int i = F3_i;
                static long l = F3_l;
                static float f = F3_f;

                client->ASYNC_RPC_WITH_CB(f3, *pf3h,[&]( ReturnValue r){
                    if(r.valid())
                        cout << "f3_cln RV: " << r.get_value<float>() << endl;
                    else
                        cout << "f3_cln: invalid RPC return type" << endl;
                    cout << "f3_cln: " << c << " " << b << endl;
                    cout << "f3_cln: " << s << " " << i << endl;
                    cout << "f3_cln: " << l << " " << f << endl;
                    if((r.get_value<float>() >= (float)(c+b+s+i+l+f-0.1)) && (r.get_value<float>() <= (float)(c+b+s+i+l+f+0.1))){
                        cout << "Test f3_cln: PASSED!" << endl;
                        total_passed_test++;
                    }
                    else
                        cout << "Test f3_cln: FAILED!" << endl;
                },c,b,s,i,l,f);
            }
        #endif
        void end_test_f3_cln() {
            client->disconnect(*pf3h);
        }
    #endif
#endif//TEST_F3

#ifdef TEST_F4
    #define F4_i 22
    #define F4_l 33
    #define F4_ll 44
    #define F4_f 55.6
    #define F4_d 66.5
    #if BINARY_BASED_PROTOCOL
        #define F4_s1  "hello string"
        #define F4_s2  "F4: hello string"
    #else
        #define F4_s1  "hellostring"
        #define F4_s2  "F4:hellostring"
    #endif
    #if BMRPC_SERVER
        #if BINARY_BASED_PROTOCOL

            #if P64
            float f4_srv(int& i, long& l, long long& ll, float& f, double& d, string& s, vector<unsigned char>& v){
                cout << "f4_srv before: " << i << " " << l << endl;
                cout << "f4_srv: " << ll << " " << f << endl;
                cout << "f4_srv: " << d << " " << s << endl;
                cout << "f4_srv blob: " << endl;
                for(auto a: v)
                    cout << std::hex << +a << " ";
                cout << std::dec;
                cout << endl;
                i++;
                l++;
                ll++;
                f++;
                d++;
                s = F4_s2;
                int k = 0;
                for(int j = 0; j < v.size(); ++j){
                    v[j]++;
                    k+=v[j];
                }
                cout << "f4_srv after: " << i << " " << l << endl;
                cout << "f4_srv: " << ll << " " << f << endl;
                cout << "f4_srv: " << d << " " << s << endl;
                cout << "f4_srv blob: " << endl;
                for(auto a: v)
                    cout << std::hex << +a << " ";
                cout << std::dec;
                cout << endl;
                return (float)(i+l+ll+f+d+k);
            }
            float (*f4)(int& i, long& l, long long& ll, float& f, double& d, string& s, vector<unsigned char>& v) = f4_srv;
            #else//P64
            float f4_srv(int& i, long& l, float& f, string& s, vector<unsigned char>& v){
                    cout << "f4_srv before: " << i << " " << l << endl;
                    cout << "f4_srv: " << f << " " << s << endl;
                    cout << "f4_srv blob: " << endl;
                    for(auto a: v)
                        cout << std::hex << +a << " ";
                    cout << std::dec;
                    cout << endl;
                    i++;
                    l++;
                    f++;
                    s = F4_s2;
                    int k = 0;
                    for(int j = 0; j < v.size(); ++j){
                        v[j]++;
                        k+=v[j];
                    }
                    cout << "f4_srv after: " << i << " " << l << endl;
                    cout << "f4_srv: " << f << " " << s << endl;
                    cout << "f4_srv blob: " << endl;
                    for(auto a: v)
                        cout << std::hex << +a << " ";
                    cout << std::dec;
                    cout << endl;
                    return (float)(i+l+f+k);
                }
                float (*f4)(int& i, long& l, float& f, string& s, vector<unsigned char>& v) = f4_srv;
            #endif//P64
        #else//BINARY_BASED_PROTOCOL
            #if P64
                float f4_srv(int& i, long& l, long long& ll, float& f, double& d, string& s){
                    cout << "f4_srv before: " << i << " " << l << endl;
                    cout << "f4_srv: " << ll << " " << f << endl;
                    cout << "f4_srv: " << d << " " << s << endl;
                    i++;
                    l++;
                    ll++;
                    f++;
                    d++;
                    s = F4_s2;
                    cout << "f4_srv after: " << i << " " << l << endl;
                    cout << "f4_srv: " << ll << " " << f << endl;
                    cout << "f4_srv: " << d << " " << s << endl;
                    return (float)(i+l+ll+f+d);
                }
                float (*f4)(int& i, long& l, long long& ll, float& f, double& d, string& s) = f4_srv;
            #else
                 float f4_srv(int& i, long& l, float& f, string& s){
                    cout << "f4_srv before: " << i << " " << l << endl;
                    cout << "f4_srv: " << f << " " << s << endl;
                    i++;
                    l++;
                    f++;
                    s = F4_s2;
                    cout << "f4_srv after: " << i << " " << l << endl;
                    cout << "f4_srv: " << f << " " << s << endl;
                    return (float)(i+l+f);
                }
                float (*f4)(int& i, long& l, float& f, string& s) = f4_srv;
            #endif//P64
        #endif//BINARY_BASED_PROTOCOL

            Skeleton<Data>* pf4rpc;
            void test_f4_srv(){
                pf4rpc = server->CONNECT(f4);
            }
            void end_test_f4_srv() {
                server->disconnect(pf4rpc);
            }
    #endif//BMRPC_SERVER

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            #ifdef BINARY_BASED_PROTOCOL
                #if P64
                    [[maybe_unused]]  double (*f4)(int& i, long& l, long long& ll, float& f, double& d, string& s, vector<unsigned char>& v);
                #else
                    [[maybe_unused]]  float (*f4)(int& i, long& l, float& f, string& s, vector<unsigned char>& v);
                #endif
            #else
                #if P64
                    [[maybe_unused]]  double (*f4)(int& i, long& l, long long& ll, float& f, double& d, string& s);
                #else
                    [[maybe_unused]]  float (*f4)(int& i, long& l, float& f, string& s);
                #endif
            #endif
        #endif

    //#if BMRPC_CLIENT
        RpcHandle<Stub<Data>>* pf4h;
        void init_test_f4_cln() {
            static RpcHandle<Stub<Data>> f4h = client->connect("f4", f4);
            pf4h = &f4h;
        }
        #if P64
            void test_f4_cln() {
                static int i = F4_i;
                static long l = F4_l;
                static long long ll = F4_ll;
                static float f = F4_f;
                static double d = F4_d;
                static string s = F4_s1;
            #if BINARY_BASED_PROTOCOL
                static vector<unsigned char> v = {0x55, 0xaa, 0x64, 0x32};

                client->ASYNC_RPC_WITH_CB(f4, *pf4h, [&](ReturnValue r) {
                    if (r.valid())
                        cout << "f4_cln RV: " << r.get_value<float>() << endl;
                    else
                        cout << "f4_cln: invalid RPC return type" << endl;
                    cout << "f4_cln: " << i << " " << l << endl;
                    cout << "f4_cln: " << ll << " " << f << endl;
                    cout << "f4_cln: " << d << " " << s << endl;
                    cout << "f3_cln blob: " << endl;
                    int k = 0;
                    for (auto a: v) {
                        cout << std::hex << +a << " ";
                        k += a;
                    }
                    cout << std::dec;
                    cout << endl;

                    if ((r.get_value<float>() >= (float) (i + l + ll + f + d + k - 0.1)) &&
                        (r.get_value<float>() <= (float) (i + l + ll + f + d + k + 0.1))
                        && (s == F4_s2)
                        && ((float) (F4_i + 1 + F4_l + 1 + F4_ll + 1 + F4_d + 1 + F4_f + 1) >=
                            (float) (i + l + ll + f + d - 0.1)) &&
                        ((float) (F4_i + 1 + F4_l + 1 + F4_ll + 1 + F4_d + 1 + F4_f + 1) <= (float) (i + l + ll + f + d + 0.1))) {
                        cout << "Test f4_cln: PASSED!" << endl;
                        total_passed_test++;
                    }
                    else
                        cout << "Test f4_cln: FAILED!" << endl;
                }, i, l, ll, f, d, s, v);
            }
            #else
                client->ASYNC_RPC_WITH_CB(f4, *pf4h,[&]( ReturnValue r){
                    if(r.valid())
                        cout << "f4_cln RV: " << r.get_value<float>() << endl;
                    else
                        cout << "f4_cln: invalid RPC return type" << endl;
                    cout << "f4_cln: " << i << " " << l << endl;
                    cout << "f4_cln: " << ll << " " << f << endl;
                    cout << "f4_cln: " << d << " " << s << endl;

                    if( (r.get_value<float>() >= (float)(i+l+ll+f+d-0.1)) && (r.get_value<float>() <= (float)(i+l+ll+f+d+0.1))
                    && (s ==  F4_s2)
                    && ((float)(F4_i+1+F4_l+1+F4_ll+1+F4_d+1+F4_f+1) >= (float)(i+l+ll+f+d-0.1)) && ((float)(F4_i+1+F4_l+1+F4_ll+1+F4_d+1+F4_f+1) <= (float)(i+l+ll+f+d+0.1))){
                        cout << "Test f4_cln: PASSED!" << endl;
                        total_passed_test++;
                    }
                    else
                        cout << "Test f4_cln: FAILED!" << endl;

                    },i,l,ll,f,d,s);
                }
            #endif//BINARY_BASED_PROTOCOL
        #else//P64
            void test_f4_cln(){
                    static int i = F4_i;
                    static long l = F4_l;
                    static float f = F4_f;
                    static string s = F4_s1;

                #if BINARY_BASED_PROTOCOL
                    static vector<unsigned char> v = {0x55,0xaa, 0x64, 0x32};

                    client->ASYNC_RPC_WITH_CB(f4, *pf4h,[&]( ReturnValue r){
                        if(r.valid())
                            cout << "f4_cln RV: " << r.get_value<float>() << endl;
                        else
                            cout << "f4_cln: invalid RPC return type" << endl;
                        cout << "f4_cln: " << i << " " << l << endl;
                        cout << "f4_cln: " << f << " " << s << endl;
                        cout << "f3_cln blob: " << endl;
                        int k = 0;
                        for(auto a: v){
                            cout << std::hex << +a << " ";
                            k+=a;
                        }
                        cout << std::dec;
                        cout << endl;

                        if( (r.get_value<float>() >= (float)(i+l+f+k-0.1)) && (r.get_value<float>() <= (float)(i+l+f+k+0.1))
                            && (s ==  F4_s2)
                            && ((float)(F4_i+1+F4_l+1+F4_f+1) >= (float)(i+l+f-0.1)) && ((float)(F4_i+1+F4_l+1+F4_f+1) <= (float)(i+l+f+0.1))){
                            cout << "Test f4_cln: PASSED!" << endl;
                            total_passed_test++;
                        }
                        else
                            cout << "Test f4_cln: FAILED!" << endl;
                    },i,l,f,s,v);
                #else
                    client->ASYNC_RPC_WITH_CB(f4, *pf4h,[&]( ReturnValue r){
                        if(r.valid())
                            cout << "f4_cln RV: " << r.get_value<float>() << endl;
                        else
                            cout << "f4_cln: invalid RPC return type" << endl;
                        cout << "f4_cln: " << i << " " << l << endl;
                        cout << "f4_cln: " << f << " " << s << endl;

                        if( (r.get_value<float>() >= (float)(i+l+f-0.1)) && (r.get_value<float>() <= (float)(i+l+f+0.1))
                        && (s ==  F4_s2)
                        && ((float)(F4_i+1+F4_l+1+F4_f+1) >= (float)(i+l+f-0.1)) && ((float)(F4_i+1+F4_l+1+F4_f+1) <= (float)(i+l+f+0.1))){
                            cout << "Test f4_cln: PASSED!" << endl;
                            total_passed_test++;
                        }
                        else
                            cout << "Test f4_cln: FAILED!" << endl;

                    },i,l,f,s);
                #endif
                }
        #endif//P64
        void end_test_f4_cln() {
            client->disconnect(*pf4h);
        }
    #endif//BMRPC_CLIENT
#endif//TEST_F4

#ifdef TEST_F5
    #define F5_i 64;
    #define F5_d 33.55;
    #define F5_s  "789"
    #if BMRPC_SERVER
        #if P64
            float f5_srv(int a, double& b, const string& c){
        #else
            float f5_srv(int a, float& b, const string& c){
        #endif
        cout << "f5_srv before: " << a << " " << b << " " << c << endl;
        b = a+b;
        a++;
        cout << "f5_srv after: " << a << " " << b << " " << c << endl;
        return stoi(c)+a+b;
        }
        #if P64
            float (*f5)(int a, double& b, const string& c) = f5_srv;
        #else
            float (*f5)(int a, float& b, const string& c) = f5_srv;
        #endif
            Skeleton<Data>* pf5rpc;
            void test_f5_srv(){
                pf5rpc = server->CONNECT(f5);
            }
            void end_test_f5_srv() {
                server->disconnect(pf5rpc);
            }
    #endif//BMRPC_SERVER

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            #if P64
                [[maybe_unused]]  float (*f5)(int a, double& b, const string& c);
            #else
                 [[maybe_unused]]  float (*f5)(int a, float& b, const string& c);
            #endif
        #endif
        RpcHandle<Stub<Data>>* pf5h;
        void init_test_f5_cln() {
            static RpcHandle<Stub<Data>> f5h = client->connect("f5", f5);
            pf5h = &f5h;
        }
        void test_f5_cln(){
            static int a = F5_i;
        #if P64
            static double b = F5_d;
        #else
            static float b = F5_d;
        #endif
            static string c = F5_s;
            client->ASYNC_RPC_WITH_CB(f5, *pf5h,[&]( ReturnValue r){
                if(r.valid())
                    cout << "f5_cln RV: " << r.get_value<float>() << endl;
                else
                    cout << "f5_cln: invalid RPC return type" << endl;
                cout << "f5_cln: " << a << " " << b <<  " " << c << endl;
                if((r.get_value<float>() >= (float)(stoi(c)+a+1+b-0.1)) && (r.get_value<float>() <= (float)(stoi(c)+a+1+b+0.1))) {
                    cout << "Test f5_cln: PASSED!" << endl;
                    total_passed_test++;
                }
                else
                    cout << "Test f5_cln: FAILED!" << endl;
            }, a,b,c);
        }
        void end_test_f5_cln() {
            client->disconnect(*pf5h);
        }
    #endif//BMRPC_CLIENT
#endif//TEST_F5

#ifdef TEST_F6
    #define F6_i 64;
    #define F6_f 33.55;
    #define F6_s1  "789"
    #define F6_s2  "123"
    #if BMRPC_SERVER
        int f6_srv(int a, float& b, string& c){
            cout << "f6_srv before: " << a << " " << b << " " << c << endl;
            b = (float)a+b;
            a++;
            c = F6_s2;
            cout << "f6_srv after: " << a << " " << b << " " << c << endl;
            return (int)(stoi(c)+a+b);
        }
        int (*f6)(int a, float& b, string& c) = f6_srv;
        Skeleton<Data>* pf6rpc;
        void test_f6_srv(){
            pf6rpc = server->CONNECT(f6);
        }
        void end_test_f6_srv() {
            server->disconnect(pf6rpc);
        }
    #endif

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            [[maybe_unused]]  int (*f6)(int a, float& b, string& c);
        #endif
        RpcHandle<Stub<Data>>* pf6h;
        void init_test_f6_cln() {
            static RpcHandle<Stub<Data>> f6h = client->connect("f6", f6);
            pf6h = &f6h;
        }
        void test_f6_cln(){
            static int a = F6_i;
            static float b = F6_f;
            static string c = F6_s1;
            client->ASYNC_RPC_WITH_CB(f6, *pf6h,[&]( ReturnValue r){
                if(r.valid())
                    cout << "f6_cln RV: " << r.get_value<int>() << endl;
                else
                    cout << "f6_cln: invalid RPC return type" << endl;
                cout << "f6_cln: " << a << " " << b <<  " " << c << endl;
                if((r.get_value<int>() >= (int)(stoi(c)+a+1+b-0.1)) && (r.get_value<int>() <= (int)(stoi(c)+a+1+b+0.1))) {
                    cout << "Test f6_cln: PASSED!" << endl;
                    total_passed_test++;
                }
                else
                    cout << "Test f6_cln: FAILED!" << endl;
            }, a,b,c);
        }
        void end_test_f6_cln() {
            client->disconnect(*pf6h);
        }
    #endif
#endif//TEST_F6

#ifdef TEST_F7
    #define F7_c 0x8F
    #define F7_b 1
    #define F7_s 0x8FFF
    #define F7_i 0x8FFFFFFF
    #define F7_l 0x8FFFFFFFFFFFFFFF
    #define F7_ll 0x8FFFFFFFFFFFFFFF
    #define F7_f 55.5
    #define F7_d 66.5
    #if BMRPC_SERVER
        #if P64
            double f7_srv(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, unsigned long long ll, float f, double d){
                cout << "f7_srv: " << std::hex << +c << " " << std::hex << +b << endl;
                cout << "f7_srv: " << std::hex << +s << " " << std::hex << i << endl;
                cout << "f7_srv: " << std::hex << l << " " << std::hex << ll << endl;
                cout << "f7_srv: " << f << " " << d << endl;
                cout << std::dec;
                int j = 0;
                if(c == F7_c) j++;
                if(b == F7_b) j++;
                if(s == F7_s) j++;
                if(i == F7_i) j++;
                if(l == F7_l) j++;
                if(ll == F7_ll) j++;
                if(f == F7_f) j++;
                if(d == F7_d) j++;
                return (double)(j);
            }
            double (*f7)(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, unsigned long long ll, float f, double d) = f7_srv;
        #else
            float f7_srv(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, float f){
                    cout << "f7_srv: " << std::hex << +c << " " << std::hex << +b << endl;
                    cout << "f7_srv: " << std::hex << +s << " " << std::hex << i << endl;
                    cout << "f7_srv: " << std::hex << l << " " << std::hex << f << endl;
                    cout << std::dec;
                    int j = 0;
                    if(c == F7_c) j++;
                    if(b == F7_b) j++;
                    if(s == F7_s) j++;
                    if(i == F7_i) j++;
                    if(l == F7_l) j++;
                    if(f == F7_f) j++;

                    return (double)(j);
                }
                float (*f7)(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, float f) = f7_srv;
        #endif
        Skeleton<Data>* pf7rpc;
        void test_f7_srv(){
            pf7rpc = server->CONNECT(f7);
        }
        void end_test_f7_srv() {
            server->disconnect(pf7rpc);
        }
    #endif//BMRPC_SERVER

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            #if P64
                [[maybe_unused]]  double (*f7)(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, unsigned long long ll, float f, double d);
            #else
                [[maybe_unused]]  float (*f7)(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, float f);
            #endif
        #endif
        RpcHandle<Stub<Data>>* pf7h;
        void init_test_f7_cln() {
            static RpcHandle<Stub<Data>> f7h = client->connect("f7", f7);
            pf7h = &f7h;
        }
        #if P64
            void test_f7_cln(){
                static unsigned char c = F7_c;
                static bool b = F7_b;
                static unsigned short s = F7_s;
                static unsigned int i = F7_i;
                static unsigned long l = F7_l;
                static unsigned long long ll = F7_ll;
                static float f = F7_f;
                static double d = F7_d;
                client->ASYNC_RPC_WITH_CB(f7, *pf7h,[&]( ReturnValue r){
                    if(r.valid())
                        cout << "f7_cln RV: " << r.get_value<double>() << endl;
                    else
                        cout << "f7_cln: invalid RPC return type" << endl;
                    cout << "f7_cln: " << std::hex << +c << " " << std::hex << +b << endl;
                    cout << "f7_cln: " << std::hex << +s << " " << std::hex << i << endl;
                    cout << "f7_cln: " << std::hex << l << " " << std::hex << ll << endl;
                    cout << "f7_cln: " << f << " " << d << endl;
                    cout << std::dec;

                    if(r.get_value<double>() == 8.0) {
                        cout << "Test f7_cln: PASSED!" << endl;
                        total_passed_test++;
                    }
                    else
                        cout << "Test f7_cln: FAILED!" << endl;
                }, c,b,s,i,l,ll,f,d);
            }
        #else
            void test_f7_cln(){
                static unsigned char c = F7_c;
                static bool b = F7_b;
                static unsigned short s = F7_s;
                static unsigned int i = F7_i;
                static unsigned long l = F7_l;
                static float f = F7_f;
                client->ASYNC_RPC_WITH_CB(f7, *pf7h,[&]( ReturnValue r){
                    if(r.valid())
                        cout << "f7_cln RV: " << r.get_value<float>() << endl;
                    else
                        cout << "f7_cln: invalid RPC return type" << endl;
                    cout << "f7_cln: " << std::hex << +c << " " << std::hex << +b << endl;
                    cout << "f7_cln: " << std::hex << +s << " " << std::hex << i << endl;
                    cout << "f7_cln: " << std::hex << l << " " << std::hex << f << endl;
                    cout << std::dec;
                    if(r.get_value<float>() == 6.0){
                        cout << "Test f7_cln: PASSED!" << endl;
                        total_passed_test++;
                    }
                    else
                        cout << "Test f7_cln: FAILED!" << endl;
                }, c,b,s,i,l,f);
            }
        #endif
        void end_test_f7_cln() {
            client->disconnect(*pf7h);
        }
    #endif//BMRPC_CLIENT
#endif//TEST_F7

#ifdef TEST_F8
    #define F8_i 64
    #define F8_f 33.55
    #define F8_s1  "789"
    #define F8_s2  "123"
    static int a_f8 = F8_i;
    static float b_f8 = F8_f;
    static string c_f8 = F8_s1;
    #if BMRPC_SERVER
        void f8_srv(int& a, float& b, string& c){
            cout << "f8_srv before: " << a << " " << b << " " << c << endl;
            b = (float)a+b;
            a = (int)(stoi(c)+a+b);
            c = F8_s2;
            cout << "f8_srv after: " << a << " " << b << " " << c << endl;
        }
        void (*f8)(int& a, float& b, string& c) = f8_srv;
        Skeleton<Data>* pf8rpc;
        void test_f8_srv(){
            pf8rpc = server->CONNECT(f8);
        }
        void end_test_f8_srv() {
            server->disconnect(pf8rpc);
        }
    #endif

    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            [[maybe_unused]]  int (*f8)(int a, float& b, string& c);
        #endif
        RpcHandle<Stub<Data>>* pf8h;
        void init_test_f8_cln() {
            static RpcHandle<Stub<Data>> f8h = client->connect("f8", f8);
            pf8h = &f8h;
        }
        void test_f8_cln(){
            client->ASYNC_RPC(f8, *pf8h, a_f8, b_f8, c_f8);
        }
        void end_test_f8_cln() {
            client->disconnect(*pf8h);
        }
    #endif
#endif//TEST_F8

#ifdef TEST_F9
    #if BMRPC_SERVER
        int f9(bool b, long& c){
            if(b){
                c++;
                return c;
            }
            return 0;
        }
    #endif
    #if BMRPC_CLIENT
        #if BMRPC_SERVER == false
            [[maybe_unused]]  int (*f9)(bool b, long& c);
        #endif
    #endif
#endif//TEST_F9


void test() {

#ifdef TEST_STREAMER

    /****
    * Server
    */
    #ifdef LOOP_BACK_TEST
        SharedBuffer<DataItem> shared_buffer = SharedBuffer<DataItem>();
        ServerCom<DataItem> server_com = ServerCom(shared_buffer);
    #else
        //define your server_com
    #endif
    Streamer<DataItem,ServerCom<DataItem>> server_streamer =  Streamer<DataItem,ServerCom<DataItem>>(&server_com);
    server_com.open();

    /**
     * Client
     */
     #ifdef LOOP_BACK_TEST
        ClientCom<DataItem> client_com = ClientCom(shared_buffer);
     #else
        //define your client_com
    #endif
    Streamer<DataItem,ClientCom<DataItem>> client_streamer =  Streamer<DataItem,ClientCom<DataItem>>(&client_com);
    client_com.open();

#define MEM_SIZE 8192
    auto mem_1 = new unsigned char[MEM_SIZE];
    auto mem_2 = new unsigned char[MEM_SIZE];

    //Test variable transfer sizes and variable data
    srand( time(nullptr) ); //Randomize seed initialization

    for(int i = 0; i < MEM_SIZE; i++){
        int randVal = (rand() % 0xFF)&0xFF;
        mem_1[i]=randVal;
    }

    size_t i_write = 0;
    size_t i_read = 0;

    while(i_write < MEM_SIZE){
        size_t n_sent;
        int randSize = rand() % 10 + 1; // Generate a random number between 1 and 10
        while((n_sent = client_streamer.write(reinterpret_cast<DataItem*>(&mem_1[i_write]), randSize))>0){
            if(n_sent > 0){
                i_write += n_sent;
                if(i_write >= MEM_SIZE - 10)
                    randSize = MEM_SIZE - i_write;
                if(i_write >= MEM_SIZE)
                    break;
            }
        }

        size_t n_read;
        while((n_read = server_streamer.rx_size()) > 0){
            auto size = server_streamer.read(reinterpret_cast<DataItem*>(&mem_2[i_read]), n_read);
            if(size > 0)
                i_read += size;
            //flush() has to be called from continuously from the sender loop
            // in case of polling based transmission.
            client_streamer.flush();//It is required otherwise items remains in the client's stream
        }
    }

    if(memcmp(mem_1,mem_2,MEM_SIZE) == 0)
        cout << "Streamer test passed: Memory blocks match" << endl;
    else
        cout << "Streamer test failed: Memory blocks don't match" << endl;
#ifdef DBG_TEST
    for(int i = 0; i < MEM_SIZE; ++i){
        cout << std::hex << +mem_1[i] << " " << std::hex << +mem_2[i] << endl;
    }
#endif
    delete[] mem_1;
    delete[] mem_2;

#else

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
    //RpcServer server = RpcServer<DataItem,Data, ServerCom<DataItem>>(&server_com);
    RpcServer my_server = CREATE_SERVER(ServerCom<DataItem>, server_com);
#else
    //define your server_com
            EmptyCom<DataItem> server_com = EmptyCom<DataItem>();
            RpcServer my_server = CREATE_SERVER(EmptyCom<DataItem>, server_com);
#endif
    server = &my_server;
    server_com.open();
#endif


#if BMRPC_CLIENT
#ifdef LOOP_BACK_TEST
    ClientCom<DataItem> client_com = ClientCom(shared_buffer);
    //#define CREATE_CLIENT(com_class, com_object) RpcClient<DataItem,Data, com_class>(&(com_object))
    RpcClient my_client = CREATE_CLIENT(ClientCom<DataItem>, client_com);
#else
    //define your client_com
            EmptyCom<DataItem> client_com = EmptyCom<DataItem>();
            RpcClient my_client = CREATE_CLIENT(EmptyCom<DataItem>, client_com);
#endif
    client = &my_client;
    client_com.open();
#endif


#if BMRPC_SERVER
#ifdef TEST_F0
    test_f0_srv();
#endif

#ifdef TEST_F1
    test_f1_srv();
#endif

#ifdef TEST_F2
    test_f2_srv();
#endif

#ifdef TEST_F3
    test_f3_srv();
#endif

#ifdef TEST_F4
    test_f4_srv();
#endif

#ifdef TEST_F5
    test_f5_srv();
#endif

#ifdef TEST_F6
    test_f6_srv();
#endif

#ifdef TEST_F7
    test_f7_srv();
#endif

#ifdef TEST_F8
    test_f8_srv();
#endif
#endif


#if BMRPC_CLIENT
#ifdef TEST_F0
    init_test_f0_cln();
#endif

#ifdef TEST_F1
    init_test_f1_cln();
#endif

#ifdef TEST_F2
    init_test_f2_cln();
#endif

#ifdef TEST_F3
    init_test_f3_cln();
#endif

#ifdef TEST_F4
    init_test_f4_cln();
#endif

#ifdef TEST_F5
    init_test_f5_cln();
#endif

#ifdef TEST_F6
    init_test_f6_cln();
#endif

#ifdef TEST_F7
    init_test_f7_cln();
#endif

#ifdef TEST_F8
    init_test_f8_cln();
#endif

    for(int i = 0; i < TEST_MULTIPLE; ++i)
    {
#ifdef TEST_F0
        test_f0_cln();
#endif

#ifdef TEST_F1
        test_f1_cln();
#endif

#ifdef TEST_F2
        test_f2_cln();
#endif

#ifdef TEST_F3
        test_f3_cln();
#endif

#ifdef TEST_F4
        test_f4_cln();
#endif

#ifdef TEST_F5
        test_f5_cln();
#endif

#ifdef TEST_F6
        test_f6_cln();
#endif

#ifdef TEST_F7
        test_f7_cln();
#endif

#ifdef TEST_F8
        test_f8_cln();
#endif
    }
#endif//BMRPC_CLIENT

    //Both loops together
    /***
     * Pay attention that in case of lvalue the derived class reference has to be used with the CRTP instead of the baseclass.
     * The stack gets corrupted otherwise.
     * In case of pointer then the pointer of the base class initialized with the derived one can be used otherwise.
     */
    TimeOutChrono tout;
#define SLEEP_TIME_LOOP 5
    long average_loop_time = (((SERVER_LOOP_TOUT_MS * 2 + CLIENT_LOOP_TOUT_MS * 2) >> 1) + SLEEP_TIME_LOOP)*5;
    tout.preset(average_loop_time * TEST_MULTIPLE + average_loop_time * MAX_F8_INVOKATIONS / 3); //0.5s per test group
    tout.start();
#if BMRPC_CLIENT
    my_client.initLoop();
#endif
#if BMRPC_SERVER
    my_server.initLoop();
#endif


#ifdef TEST_F9
#if BMRPC_CLIENT
    long client_beats_0 = 0;
    long client_beats_1 = 0;
    long client_beats_2 = 0;
    bool enable_client_beats_0 = true;
    bool enable_client_beats_1 = true;
    bool enable_client_beats_2 = true;
    RpcHandle<Stub<Data>> f9h_0 = client->connect("f9", f9);
    RpcHandle<Stub<Data>> f9h_1 = client->connect("f9", f9);
    RpcHandle<Stub<Data>> f9h_2 = client->connect("f9", f9);
    int f9_invokations = 0;
#endif
#if BMRPC_SERVER
    Skeleton<Data>* f9rpc = server->CONNECT(f9);
#endif
#endif

    while (!tout.expired())
    {
#ifdef TEST_F9
#if BMRPC_CLIENT

        if(f9_invokations < MAX_F8_INVOKATIONS) {
            if (enable_client_beats_0) {
                bool was_called = client->ASYNC_RPC_WITH_CB(f9, f9h_0, [&](ReturnValue r) {
                    if (r.valid())
                        cout << "f9_cln RV: " << r.get_value<int>() << " " << client_beats_0 << endl;
                    else
                        cout << "f9_cln: invalid RPC return type" << endl;
                    if (r.get_value<int>() == client_beats_0)
                        cout << "Test f9_cln: PASSED!" << endl;
                    else
                        cout << "Test f9_cln: FAILED!" << endl;
                    enable_client_beats_0 = true;
                }, true, client_beats_0);
                if(was_called) {
                    enable_client_beats_0 = false;
                    ++f9_invokations;
                }
            }

            if (enable_client_beats_1) {
                bool was_called = client->ASYNC_RPC_WITH_CB(f9, f9h_1, [&](ReturnValue r) {
                    if (r.valid())
                        cout << "f9_cln RV: " << r.get_value<int>() << " " << client_beats_1 << endl;
                    else
                        cout << "f9_cln: invalid RPC return type" << endl;
                    if (r.get_value<int>() == client_beats_1)
                        cout << "Test f9_cln: PASSED!" << endl;
                    else
                        cout << "Test f9_cln: FAILED!" << endl;
                    enable_client_beats_1 = true;
                }, true, client_beats_1);
                if(was_called) {
                    enable_client_beats_1 = false;
                    ++f9_invokations;
                }
            }

            if (enable_client_beats_2) {
                bool was_called = client->ASYNC_RPC_WITH_CB(f9, f9h_2, [&](ReturnValue r) {
                    if (r.valid())
                        cout << "f9_cln RV: " << r.get_value<int>() << " " << client_beats_2 << endl;
                    else
                        cout << "f9_cln: invalid RPC return type" << endl;
                    if (r.get_value<int>() == client_beats_2)
                        cout << "Test f9_cln: PASSED!" << endl;
                    else
                        cout << "Test f9_cln: FAILED!" << endl;
                    enable_client_beats_2 = true;
                }, true, client_beats_2);
                if(was_called) {
                    enable_client_beats_2 = false;
                    ++f9_invokations;
                }
            }
        }

#endif
#endif

#if BMRPC_SERVER
        my_server.doLoop();
#endif
#if BMRPC_CLIENT
        my_client.doLoop();
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_LOOP));
    }

#if BMRPC_CLIENT

#ifdef TEST_F8
    //Check test F8 outcome
    cout << "f8_cln: " << a_f8 << " " << b_f8 <<  " " << c_f8 << endl;
    int term = stoi(F8_s1)+F8_i+b_f8;
    if(a_f8 == term && c_f8 == F8_s2) {
        cout << "Test f8_cln: PASSED!" << endl;
        total_passed_test++;
    }
    else
        cout << "Test f8_cln: FAILED!" << endl;
#endif

    if(total_passed_test + TEST_MULTIPLE - 1 == TEST_MULTIPLE * MAX_MULTIPLE_F)//it considers differently F8 test
        cout << endl << "MULTIPLE TESTS: PASSED!" << endl;
    else
        cout << endl << "MULTIPLE TESTS: FAILED!" << endl;
    cout << "TOTAL TESTS: " << total_passed_test / MAX_MULTIPLE_F << endl;

    if(client_beats_0 != 0 && client_beats_0 == client_beats_1 && client_beats_1 == client_beats_2)
        cout << endl << "HANDLES TESTS: PASSED!" << endl;
    else
        cout << endl << "HANDLES TESTS: FAILED!" << endl;
    cout << "TOTAL TESTS: " << client_beats_0 << endl;


#ifdef TEST_F0
    end_test_f0_cln();
#endif

#ifdef TEST_F1
    end_test_f1_cln();
#endif

#ifdef TEST_F2
    end_test_f2_cln();
#endif

#ifdef TEST_F3
    end_test_f3_cln();
#endif

#ifdef TEST_F4
    end_test_f4_cln();
#endif

#ifdef TEST_F5
    end_test_f5_cln();
#endif

#ifdef TEST_F6
    end_test_f6_cln();
#endif

#ifdef TEST_F7
    end_test_f7_cln();
#endif

#ifdef TEST_F8
    end_test_f8_cln();
#endif
#endif

#if BMRPC_SERVER
#ifdef TEST_F0
    end_test_f0_srv();
#endif

#ifdef TEST_F1
    end_test_f1_srv();
#endif

#ifdef TEST_F2
    end_test_f2_srv();
#endif

#ifdef TEST_F3
    end_test_f3_srv();
#endif

#ifdef TEST_F4
    end_test_f4_srv();
#endif

#ifdef TEST_F5
    end_test_f5_srv();
#endif

#ifdef TEST_F6
    end_test_f6_srv();
#endif

#ifdef TEST_F7
    end_test_f7_srv();
#endif

#ifdef TEST_F8
    end_test_f8_srv();
#endif

#ifdef TEST_F9
#if BMRPC_CLIENT
    client->disconnect(f9h_0);
    client->disconnect(f9h_1);
    client->disconnect(f9h_2);
#endif
#if BMRPC_SERVER
    server->disconnect(f9rpc);
#endif
#endif

#endif

#endif//TEST_STREAMER

    cout << "test ended." << endl;

}//end test






