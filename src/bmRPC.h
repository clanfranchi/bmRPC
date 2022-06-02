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

#ifndef BMRPC_H
#define BMRPC_H


#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <array>
#include <queue>
#include <forward_list> // for registry
#include <charconv>     //from_chars and to_chars
#include <algorithm>// for godbolt
#include <chrono> // for godbolt
#include <cstring> //std::memcpy for godbolt
#include <climits> //char_bit for godbolt

/**
 * User Settings
 */

#define LOOP_BACK_TEST
//Loop back test is implemented by connecting client and server through a shared buffer in the same process.
//In this case bytes swapping for endianess occurs both when transmitting and receiving.

#define BMRPC_CLIENT true
//Install a client if true
[[maybe_unused]] const bool bmrpc_client = BMRPC_CLIENT;

#define BMRPC_SERVER true
//Install a server if true
[[maybe_unused]] const bool bmrpc_server = BMRPC_SERVER;

//Set Protocol type.
#define BINARY_BASED_PROTOCOL  true
const bool is_binary_protocol = BINARY_BASED_PROTOCOL;

//Set P64 (long long and double types) support
#define P64  true
const bool is_p64 = P64; //sizeof(void*)>4

//Set Endianess
enum class endian {
    little = 0,
    big = 1,
    SRV_NATIVE = big,
    CLN_NATIVE = little
};
constexpr bool to_swap = endian::SRV_NATIVE != endian::CLN_NATIVE;

//Set the maximum size (sizeof(SIZE_T) of blob types (std::string and std::vector<unsigned char>)
using SIZE_T = uint16_t;

//Set client message buffer buffer size
#define MAX_CLIENT_MSG_BUFFER_SIZE 512

//Set streamer circular buffer size.
//Must be a power of two.

#define STREAMER_BUFFER_SIZE 1024
constexpr bool streamer_power_of_2_req = STREAMER_BUFFER_SIZE && !(STREAMER_BUFFER_SIZE & (STREAMER_BUFFER_SIZE - 1));
static_assert(streamer_power_of_2_req,"The Streamer circular buffer size must be a power of two");

/**
 * System Settings
 */

#define SERVER_LOOP_TOUT_MS 5
#define CLIENT_LOOP_TOUT_MS 5

#define FORCE_INLINING
#ifdef FORCE_INLINING
#define INLINE inline __attribute__((always_inline))
#else
#define INLINE
#endif


#include "bmRPCUtilities.h"
#include "bmRPCDataLink.h"
#include "bmRPCStreamer.h"
#include "bmRPCTimeout.h"
#include "bmRPCMessage.h"
#include "bmRPCSerializer.h"
#include "bmRPCAnyArg.h"
#include "bmRPCMarshaller.h"
#include "bmRPCStub.h"
#include "bmRPCRegistry.h"
#if BMRPC_SERVER
    #include "bmRPCServer.h"
#endif
#if BMRPC_CLIENT
    #include "bmRPCClient.h"
#endif


using TextDataItem = char;
using TextData = std::string;
using BinaryDataItem = unsigned char;
using BinaryData = std::vector<unsigned char>;

using DataItem = typename std::conditional<is_binary_protocol,BinaryDataItem,TextDataItem>::type;
using Data = typename std::conditional<is_binary_protocol, BinaryData, TextData>::type;

#define CREATE_SERVER(com_class, com_object) RpcServer<DataItem,Data, com_class>(&(com_object))
#define CREATE_CLIENT(com_class, com_object) RpcClient<DataItem,Data, com_class>(&(com_object))
#define CONNECT(func) connect(#func, func)


#endif // BMRPC_H

/**
 * Endianess. It is set at compile time by means of behavior controlled type punning.
 * Transmitter is in charge of re-ordering bytes according to the receiver endianess.
 * While IEEE 754 standard does not specify endianess for float and double types it is assumed that these follow the integer representation.
 * It is used
 */

//constexpr uint32_t i{ 0x01020304 };
//const bool is_little_endian{ *reinterpret_cast<const uint8_t*>(&i) == 0x04 };


//- buffers uses <unsigned char>
//- Use sizeof(CHAR_BITS) to get the bits number.
//- Adopt the "network byte order" standard, which is Big Endian.
//  Little Endian are in charge of conversion.
//- Test Endianess at runtime (MSB == Lower Address -> Big Endian)
// On C++ 20: constexpr bool is_little_endian = (std::endian::native == std::endian::little);
//
