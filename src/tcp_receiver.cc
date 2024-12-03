#include "tcp_receiver.hh"
#include <numeric>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }
  if ( !message.SYN && !connected_ ) {
    return;
  }
  if ( message.SYN ) {
    connected_ = true;
    zero_point_ = message.seqno;
  } else {
    if ( zero_point_ == message.seqno ) {
      return;
    }
    stream_index_ = message.seqno.unwrap( *zero_point_, reassembler_.writer().bytes_pushed() ) - 1;
  }
  auto sequence_len = message.sequence_length();
  reassembler_.insert( stream_index_, std::move( message.payload ), message.FIN );
  ack_ = reassembler_.writer().bytes_pushed() + 1 + reassembler_.writer().is_closed();
  stream_index_ += sequence_len - message.FIN - message.SYN;
  this->send();
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  return TCPReceiverMessage {
    zero_point_.has_value() ? Wrap32::wrap( ack_, *zero_point_ ) : std::optional<Wrap32> {},
    static_cast<uint16_t>( std::min( static_cast<uint64_t>( std::numeric_limits<uint16_t>::max() ),
                                     reassembler_.writer().available_capacity() ) ),
    reassembler_.writer().has_error() };
}
