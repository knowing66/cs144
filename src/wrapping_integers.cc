#include "wrapping_integers.hh"
#include <cstdlib>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32((static_cast<uint32_t>(n) + zero_point.raw_value_) % (1UL << 32));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t start_64 = checkpoint & 0xFFFFFFFF00000000;
  uint64_t ans=0;
  if( start_64 >= 0x0000000100000000){
    uint64_t offset = 0;
    if(this -> raw_value_>= zero_point.raw_value_)
      offset =static_cast<uint64_t>(this->raw_value_ - zero_point.raw_value_);
    else
      offset =static_cast<uint64_t>(this->raw_value_ + (1UL << 32) - zero_point.raw_value_);
    uint64_t start_64_0=start_64 - 0x0000000100000000;
    uint64_t start_64_1=start_64 + 0x0000000100000000;
    uint64_t distance_0=(checkpoint > start_64_0 + offset)? checkpoint-(start_64_0 + offset) :(start_64_0 + offset) - checkpoint;
    uint64_t distance_=(checkpoint > start_64 + offset)? checkpoint-(start_64 + offset) :(start_64 + offset) - checkpoint;
    uint64_t distance_1=(checkpoint > start_64_1 + offset)? checkpoint-(start_64_1 + offset) :(start_64_1 + offset) - checkpoint;
    if(distance_ < distance_1){
      if(distance_ < distance_0)
        ans = start_64 + offset;
      else
        ans = start_64_0 + offset;
    }else{
      if(distance_1 < distance_0)
        ans = start_64_1 + offset;
      else
        ans = start_64_0 + offset;
    }
  } else {
    uint64_t offset = 0;
    if(this -> raw_value_>= zero_point.raw_value_)
      offset =static_cast<uint64_t>(this->raw_value_ - zero_point.raw_value_);
    else
      offset =static_cast<uint64_t>(this->raw_value_ + (1UL << 32) - zero_point.raw_value_);
    uint64_t start_64_1=start_64 + 0x0000000100000000;
    uint64_t distance_=(checkpoint > start_64 + offset)? checkpoint-(start_64 + offset) :(start_64 + offset) - checkpoint;
    uint64_t distance_1=(checkpoint > start_64_1 + offset)? checkpoint-(start_64_1 + offset) :(start_64_1 + offset) - checkpoint;
    if(distance_ < distance_1){
      ans = start_64 + offset;
    }else{
      ans = start_64_1 + offset;
    }
  }
  return ans;
}
