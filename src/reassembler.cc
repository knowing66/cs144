#include "reassembler.hh"
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  // if(first_index==0){
  //   output_.writer().push(data);
  //   output_last_index=first_index+data.size();
  // }

  if(is_last_substring) last_substring_index=first_index;

  if(first_index<=output_last_index+1/*==output_.writer().bytes_pushed()*/){
    uint64_t push_length=min(output_.writer().available_capacity(),first_index+data.size()-last_substring_index+1);
    output_.writer().push( data.substr( last_substring_index,push_length ));
    output_last_index = last_substring_index+push_length;
    if(is_last_substring){
      output_.writer().close();
    }
    
    uint64_t next=first_index+data.size();
    while(string_waited_reorder.size()!=0 && next==string_waited_reorder.begin()->first){

      output_.writer().push( string_waited_reorder.begin()->second );
      output_last_index=string_waited_reorder.begin()->first+string_waited_reorder.begin()->second.size();
      next=string_waited_reorder.begin()->first+string_waited_reorder.begin()->second.size();

      if( string_waited_reorder.begin()-> first == last_substring_index ){
        string_waited_reorder.erase( string_waited_reorder.begin() );
        output_.writer().close();
        break;
      }

      string_waited_reorder.erase(string_waited_reorder.begin());
    }
  }else{
    if(first_index + data.size() < output_last_index ){

    }else{

      auto compare = [](const std::pair<uint64_t, std::string>& a,const uint64_t& b){
        return a.first < b;
      };

      auto pos_iter=std::lower_bound(string_waited_reorder.begin(),string_waited_reorder.end(),first_index,compare);

      if(pos_iter==string_waited_reorder.end() || pos_iter->first !=first_index)
        string_waited_reorder.insert(pos_iter,{first_index,data});

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
