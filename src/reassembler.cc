#include "reassembler.hh"
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_last_substring ) {
    total_pushed_len_ = first_index + data.length();
  }
  insert_or_store( first_index, std::move( data ) );
  write_stored_str();
  if ( total_pushed_len_.has_value() && output_.writer().bytes_pushed() == *total_pushed_len_ ) {
    output_.writer().close();
  }
}

void Reassembler::insert_or_store( uint64_t first_index, std::string data )
{
  if ( first_index < next_index_ ) {
    first_index = truncate_head( first_index, data );
  }
  if ( first_index > next_index_ ) {
    store( first_index, std::move( data ) );
  } else {
    write( std::move( data ) );
  }
}

void Reassembler::write( std::string data )
{
  output_.writer().push( std::move( data ) );
  next_index_ = output_.writer().bytes_pushed();
}

void Reassembler::write_stored_str()
{
  for ( auto& [first_index, data] : pending_substr_ ) {
    if ( first_index <= next_index_ ) {
      auto buf = std::exchange( data, "" );
      bytes_pending_ -= buf.length();
      insert_or_store( first_index, std::move( buf ) );
    }
  }
  std::erase_if( pending_substr_, []( const auto& elem ) { return elem.second.empty(); } );
}

uint64_t Reassembler::truncate_head( uint64_t old_index, std::string& data )
{
  data.erase( 0, next_index_ - old_index );
  return next_index_;
}

void Reassembler::store( uint64_t first_index, std::string data )
{
  if ( auto len = output_.writer().available_capacity() - ( first_index - next_index_ ); data.length() >= len ) {
    data.erase( data.begin() + len, data.end() );
  }
  if ( data.empty() ) {
    return;
  }
  if ( pending_substr_.empty() ) {
    bytes_pending_ += data.length();
    pending_substr_.emplace( first_index, std::move( data ) );
  } else {
    auto final_index = first_index + data.length() - 1;

    if ( pending_substr_.contains( first_index ) ) {
      if ( pending_substr_[first_index].length() >= data.length() ) {
        return;
      }
      auto mapped_data = std::exchange( pending_substr_[first_index], "" );
      bytes_pending_ -= mapped_data.length();
      pending_substr_.erase( first_index );
    }

    std::erase_if( pending_substr_, [&]( const auto& node ) {
      if ( node.first >= first_index && node.first + node.second.length() - 1 <= final_index ) {
        bytes_pending_ -= node.second.length();
        return true;
      }
      return false;
    } );

    for ( const auto& [idx, str] : pending_substr_ ) {
      if ( first_index >= idx && final_index <= idx + str.length() - 1 ) {
        return;
      }
    }

    bytes_pending_ += data.length();
    pending_substr_.emplace( first_index, std::move( data ) );

    auto begin_node = pending_substr_.lower_bound( first_index );
    auto end_node = pending_substr_.upper_bound( final_index );
    if ( begin_node != std::begin( pending_substr_ ) ) {
      begin_node = std::prev( begin_node );
    }
    for ( auto node = begin_node; std::next( node ) != end_node; ++node ) {
      auto next_node = std::next( node );
      auto this_final_index = node->first + node->second.length() - 1;
      auto next_first_index = next_node->first;
      if ( this_final_index >= next_first_index ) {
        auto len = this_final_index - next_first_index + 1;
        bytes_pending_ -= len;
        node->second.erase( node->second.begin() + node->second.length() - len, node->second.end() );
      }
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return bytes_pending_;
}
