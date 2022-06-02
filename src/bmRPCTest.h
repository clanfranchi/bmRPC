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


#ifndef BMRPC_BMRPCTEST_H
#define BMRPC_BMRPCTEST_H

//Test Cases List
//STREAMER TEST is in alternative to all the other tests.
//#define TEST_STREAMER
#define TEST_F0 //int f0(int a, double b, float& c)
#define TEST_F1 //void f1(int a)
#define TEST_F2 // long f2(int a, double b, double& c)
#define TEST_F3 // double f3(char c, bool b, short s, int i, long l, long long ll, float f, double d)
#define TEST_F4 // float f4(int& i, long& l, long long& ll, float& f, double& d, string& s, vector<unsigned char>& v)
#define TEST_F5 // float f5(int a, double& b, const string& c)
#define TEST_F6 // int f6(int a, float& b, string& c)
#define TEST_F7 // double f7(unsigned char c, bool b, unsigned short s, unsigned int i, unsigned long l, unsigned long long ll, float f, double d)
#define TEST_F8 // void f8(int& a, float& b, string& c) //no callback
#define MAX_MULTIPLE_F 9
#define TEST_MULTIPLE MAX_CLIENT_MSG_BUFFER_SIZE/MAX_MULTIPLE_F
#define TEST_F9 // int f8(bool b, long& c)
#define MAX_F8_INVOKATIONS 300

void test();

#endif //BMRPC_BMRPCTEST_H
