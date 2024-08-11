#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.RST)
    this->reassembler_.reader().set_error();

  if(this->syn_){
    if(message.seqno.unwrap(this->zero_point_,this->reassembler_.writer().bytes_pushed()) > 0){
      this->reassembler_.insert(message.seqno.unwrap(this->zero_point_,this->reassembler_.writer().bytes_pushed()) - 1,message.payload,message.FIN);
      akno_=Wrap32::wrap(this->reassembler_.writer().bytes_pushed() + 1 ,zero_point_);
      if(this->reassembler_.writer().is_closed()) this->akno_=this->akno_+1;
    }
  }
  
  if(message.SYN){
    this->zero_point_ = message.seqno;
    this->syn_=true;
    // this->akno_=Wrap32::wrap(1,zero_point_);
    this->reassembler_.insert(message.seqno.unwrap(this->zero_point_,this->reassembler_.writer().bytes_pushed()),message.payload,message.FIN);
    akno_=Wrap32::wrap(this->reassembler_.writer().bytes_pushed() + 1 ,zero_point_);
    if(message.FIN){
      // this->reassembler_.insert(message.seqno.unwrap(this->zero_point_,this->reassembler_.writer().bytes_pushed()),message.payload,message.FIN);
      this->akno_=this->akno_+1;
    }
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage ans;
  if(this->syn_)
    ans.ackno = this->akno_;
  
  ans.window_size=this->reassembler_.writer().available_capacity() <= UINT16_MAX ? this->reassembler_.writer().available_capacity() :UINT16_MAX;

  if(this->reassembler_.writer().has_error())
    ans.RST=true;
  return ans;
}
