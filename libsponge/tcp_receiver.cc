#include "tcp_receiver.hh"
#include "byte_stream.hh"
#include "stream_reassembler.hh"
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if (!_set_syn_flag){
        if (!header.syn){
            return;
            
        }
        _isn = header.seqno;
        _set_syn_flag = true;
    }
    //uint64_t abs_ack = _reassembler.stream_out().bytes_written() + 1;
    //uint64_t cur_abs_seqno = unwrap(header.seqno, _isn, abs_ack);
    //uint64_t stream_index = cur_abs_seqno - 1 + (header.syn);

    //_reassembler.push_substring(seg.payload().copy(), stream_index, header.fin);


    WrappingInt32 seqno = header.syn ? header.seqno + 1 : header.seqno; //第一个有效字符的序列号
    uint64_t checkpoint = _reassembler.stream_out().bytes_written();
    //uwarp出来是 absolute seqno 需要 -1 才能转化成 stream index
    uint64_t absolute_seqno = unwrap(seqno,_isn,checkpoint);
    uint64_t stream_index = absolute_seqno - 1;
    _reassembler.push_substring(seg.payload().copy(), stream_index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if (!_set_syn_flag) {
        return nullopt;
    }
    uint64_t abs_ack = _reassembler.stream_out().bytes_written() + 1;
    if (_reassembler.stream_out().input_ended()) {
        abs_ack++;
    }

     
    return wrap(abs_ack, _isn); }

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
