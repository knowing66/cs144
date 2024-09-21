#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return this->send_cnt_ - this->acko_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return this->resend_cnt_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  while((window_size_ == 0? 1 : window_size_) > sequence_numbers_in_flight()){
      if(is_fin_){
        break;
      }

      auto msg_to_send = make_empty_message();
      // if(!is_fin_ && reader().is_finished()){
      //   is_syn_ = true;
      //   msg_to_send.FIN = true;
      // }

      if(!is_syn_){
        msg_to_send.SYN = true;
        is_syn_ = true;
      }

      auto remaining = (window_size_ == 0? 1 : window_size_) - sequence_numbers_in_flight();
      auto len = min( TCPConfig::MAX_PAYLOAD_SIZE, remaining );
      auto&& data = msg_to_send.payload;
      while(!is_fin_&& reader().bytes_buffered() && data.size() < len){
        auto buffer = input_.reader().peek();
        auto cur_data = buffer.substr(0,len - data.size());
        data += cur_data;
        input_.reader().pop(cur_data.size());
      }

      if( !is_fin_ && data.size() < remaining && reader().is_finished() ){
        is_fin_ = true;
        msg_to_send.FIN = true;
      }

      if( msg_to_send.sequence_length() == 0 ){
        break;
      }

      transmit(msg_to_send);
      send_cnt_ += msg_to_send.sequence_length();
      outstanding_.emplace(msg_to_send);
  }
  
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  return {.seqno = Wrap32::wrap(send_cnt_,isn_),
          .SYN = false,
          .payload = {},
          .FIN = false,
          .RST = input_.has_error()};

}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  this->window_size_= msg.window_size;
  if(msg.RST) this->input_.set_error();

  if(msg.ackno.has_value()){
    auto recv_acko = msg.ackno.value().unwrap(isn_,acko_);

    if(recv_acko > send_cnt_){
      return;
    }

    while( !outstanding_.empty() ){
      auto first_msg = outstanding_.front();

      if( recv_acko < acko_ + first_msg.sequence_length()){
        break;
      }

      outstanding_.pop();
      acko_ += first_msg.sequence_length();
      cur_RTO_ = initial_RTO_ms_;
      timer_ = 0;
      resend_cnt_ = 0;
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  this->timer_ += ms_since_last_tick;
  // if(this -> timer_ >= this -> cur_RTO_ ){
  //     transmit(this->outstanding_.front());
  //     this->resend_cnt_ ++;
  //     this->cur_RTO_ *= 2;
  //     this->timer_ = 0;
  // }

  if(this -> timer_ >= this -> cur_RTO_ ){
    if (!outstanding_.empty()){
      auto re_msg = outstanding_.front();

      transmit(re_msg);
      resend_cnt_++;

      if(window_size_)
        cur_RTO_ *= 2;
      
      timer_ = 0;
    }
  }
   
}
