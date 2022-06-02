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


#ifndef BMRPCTIMEOUT_H
#define BMRPCTIMEOUT_H

namespace bm
{
namespace rpc
{
    //Use curiously recurring template pattern (CRTP) for static polymorphism
    //This technique achieves a similar effect to the use of virtual functions,
    // without the costs (and some flexibility) of dynamic polymorphism.
    template <typename T>
    class TimeOut{
    public:
        INLINE void preset(long milliseconds){
            static_cast<T*>(this)->preset_impl(milliseconds);
        }

        INLINE void start(){
            static_cast<T*>(this)->start_impl();
        }

        INLINE bool expired(){
            return static_cast<T*>(this)->expired_impl();
        }
    };


    class TimeOutChrono : public TimeOut<TimeOutChrono>{
    public:

        TimeOutChrono():m_tout_preset(10){};

        void preset_impl(long milliseconds){
            m_tout_preset = milliseconds;
        }

        void start_impl(){
            m_start = std::chrono::high_resolution_clock::now();
        }

        bool expired_impl(){
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - m_start);
            if(duration >= std::chrono::milliseconds(m_tout_preset)) return true;
            return false;
        }

    private:
        std::chrono::high_resolution_clock::time_point m_start;
        long m_tout_preset;
    };

    using TimeOut_t = TimeOut<TimeOutChrono>;

}//namespace rpc
}//namespace bm



#endif // BMRPCTIMEOUT_H




