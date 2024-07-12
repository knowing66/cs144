#include "byte_stream.hh"
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) ,
  isclosed_(false),pushed_count_(0),poped_count_(0){}

bool Writer::is_closed() const
{
  return isclosed_;
}

void Writer::push( string data )
{
  if(isclosed_){
    cout <<"push data when pipeline is closed"<< endl;
    return;
  }
  int buffer_unused_size=capacity_-buffer_.size();
  int can_push_size=min(static_cast<int>(data.size()),buffer_unused_size);
  if(!isclosed_){
    buffer_+=data.substr(0,can_push_size);
  }
  pushed_count_+=can_push_size;
  return;
}

void Writer::close()
{
  isclosed_=true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_-buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_count_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return isclosed_ && pushed_count_==poped_count_;
}

uint64_t Reader::bytes_popped() const
{
  return poped_count_;
}

string_view Reader::peek() const
{
  if(buffer_.empty()){
    return {};
  }
  return std::string_view(buffer_);
}

void Reader::pop( uint64_t len )
{
  uint64_t bytes_nums_inbuffer=buffer_.size();
  uint64_t nums_can_pop=min(len,(bytes_nums_inbuffer));
  buffer_=buffer_.substr(nums_can_pop);
  poped_count_+=nums_can_pop;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}
