#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), error_( false ), bytes_written( 0 ), bytes_read( 0 ), end_flag( false )
{}

bool Writer::is_closed() const
{
  // Your code here.
  return end_flag;
}

void Writer::push( string data )
{
  // Your code here.
  size_t bytes_to_write = std::min( data.size(), available_capacity() );

  for ( size_t i = 0; i < bytes_to_write; i++ ) {
    buffer.push_back( data[i] );
  }
  bytes_written += bytes_to_write;
}

void Writer::close()
{
  // Your code here.
  end_flag = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_written;
}

bool Reader::is_finished() const
{
  // Your code here.
  return end_flag && buffer.empty();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_read;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( buffer.empty() ) {
    return std::string_view();
  }

  return std::string_view( &buffer.front(), 1 );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  size_t bytes_to_read = std::min( buffer.size(), len );

  for ( size_t i = 0; i < bytes_to_read; i++ ) {
    buffer.pop_front();
  }

  bytes_read += bytes_to_read;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer.size();
}
