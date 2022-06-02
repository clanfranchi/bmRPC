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


#ifndef BMRPCREGISTRY_H
#define BMRPCREGISTRY_H

namespace bm
{
namespace rpc
{
    template <typename T>
    class FunctionsRegistry{
    public:

        T* insert(T& rpc){
            std::string name = rpc.getName();
            auto got = functions_map.find(name);
            if ( got != functions_map.end() ){
                //rpc already registered
                T* q = &(*got->second);
                return q;
            }
            //new rpc
            functions_list.push_front(std::move(rpc));
            auto p = functions_list.begin();
            T* q = &(*p);
            functions_map.insert(make_pair(name,q));
            return q;
        }

        void remove(T* rpc){
            if(rpc != nullptr){
                std::string name = rpc->getName();
                this->functions_map.erase(name);
                auto pre_it = functions_list.before_begin();
                for (auto it = functions_list.begin(); it != functions_list.end(); it++) {
                    if (it->getName() == name) {
                        functions_list.erase_after(pre_it);
                        break;
                    }
                    pre_it = it;
                }
            }
        }

        T* find(std::string name){
            auto got = functions_map.find(name);
            if ( got != functions_map.end() )
                return got->second;
            return nullptr;
        }

        T* get(const std::string& name){
            return functions_map[name];
        }

        [[maybe_unused]] bool empty(){
            return functions_map.empty();
        }

    private:
        //average O(1) search, insert, deletion via hash
        //needed for hash name access. Iterators change when rehashed.
        std::unordered_map<std::string, T*> functions_map;
        //needed for pointer to element access. New elements are appended. No iterators change.
        std::forward_list<T> functions_list;
    };

}//namespace rpc
}//namespace bm


#endif // BMRPCREGISTRY_H




