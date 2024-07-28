#include "reassembler.hh"
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  if(first_index<=first_unassembled_index && first_index+data.size() >= first_unassembled_index){

    uint64_t len=min(first_index+data.size()-first_unassembled_index,output_.writer().available_capacity());
    output_.writer().push(data.substr(first_unassembled_index-first_index,len));
    first_unassembled_index+=len;

    if(!string_waited_reorder.empty() && string_waited_reorder[0].first <= first_unassembled_index /*&& string_waited_reorder[0].first + string_waited_reorder[0].second.size() >= first_unassembled_index*/){
      if(string_waited_reorder[0].first + string_waited_reorder[0].second.size() >= first_unassembled_index){

        uint64_t start_index=first_unassembled_index - string_waited_reorder[0].first;
        output_.writer().push(string_waited_reorder[0].second.substr( start_index ));
        first_unassembled_index+=string_waited_reorder[0].second.size();

      }
      string_waited_reorder.erase(string_waited_reorder.begin());
    }

    if(is_last_substring) 
      output_.writer().close();

    if(first_unassembled_index == last_index +1)
      output_.writer().close();

  }else if (first_index > first_unassembled_index) { //将接受的substring 存入string_waited_reorder中

    if( output_.writer().available_capacity() > bytes_pending() ){
      if(bytes_pending() == 0) {

        uint64_t first_unacceptable_index=first_unassembled_index+output_.writer().available_capacity();
        uint64_t len=min(output_.writer().available_capacity(),data.size());

        len=min(len,first_unacceptable_index-first_index);
        string_waited_reorder.push_back({first_index,data.substr(0,len)});

        if( is_last_substring ){
          last_index = first_index + data.size() - 1;
        }
      
      } else {

        uint64_t l = first_unassembled_index;
        uint64_t r;

        for(auto it=string_waited_reorder.begin(); it != string_waited_reorder.end(); it++){
          r = it->first ;
          if(first_index > l && first_index + data.size() < r-1 && r-l >1){
            string_waited_reorder.insert(it,{first_index,data});
          } else if (first_index <= l && first_index + data.size() >= r-1 && r-l >1){
            uint64_t len = r - l;
            string_waited_reorder.insert(it,{l,data.substr(l - first_index,len)});
          } else if (first_index <= l && first_index + data.size() < r-1 && r-l >1){
            uint64_t len = first_index + data.size()-l;
            string_waited_reorder.insert(it,{l,data.substr(l - first_index,len)});
          } else if (first_index > l && first_index + data.size() >= r-1 && r-l >1){
            uint64_t len=r - first_index;
            string_waited_reorder.insert(it,{first_index,data.substr(0,len)});
          }

          l= it->first + it->second.size();
        }


        r = first_unassembled_index + output_.writer().available_capacity();
        if(first_index > l && first_index + data.size() < r-1){
            string_waited_reorder.push_back({first_index,data});
          } else if (first_index <= l && first_index + data.size() >= r-1){
            uint64_t len = r - l;
            string_waited_reorder.push_back({l,data.substr(l - first_index,len)});
          } else if (first_index <= l && first_index + data.size() < r-1){
            uint64_t len = first_index + data.size()-l;
            string_waited_reorder.push_back({l,data.substr(l - first_index,len)});
          } else if (first_index > l && first_index + data.size() >= r-1){
            uint64_t len=r - first_index;
            string_waited_reorder.push_back({first_index,data.substr(0,len)});
          }
        
      }
    }

  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t ans=0;
  for(const auto &s:string_waited_reorder){
    ans+=s.second.size();
  }
  return ans;
}
