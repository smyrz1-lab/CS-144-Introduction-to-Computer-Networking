#include "stream_reassembler.hh"
#include <cassert>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _expected_index(0), buffer(), _unassembled_bytes(0), _eof(-1), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    auto pos_iter = buffer.upper_bound(index); // 获取map中第一个大于index的迭代器指针
    // 获取第一个小于index的指针
    if (pos_iter != buffer.begin()) {
        pos_iter--;
    }

    /**
     *  三种情况，
     *  1. 一种是为 end_iter，表示不存在任何数据
     *  2. 一种是为非 end_iter，
     *      a. 一种表示确实存在一个小于等于 idx 的迭代器指针
     *      b. 一种表示当前指向的是大于 idx 的迭代器指针
     */

    // 处理后的初始位置
    size_t new_idx = index;
    // 如果存在这个指针
    if (pos_iter != buffer.end() && pos_iter->first <= index) {
        const size_t up_idx = pos_iter->first;

        //如果有重叠
        if (index < up_idx + pos_iter->second.size()) {
            new_idx = up_idx + pos_iter->second.size();
        }

        
    } else if (index < _expected_index) {
            new_idx = _expected_index;
    }

    // 有效 data 的开始索引
    const size_t data_start_pos = new_idx - index;
    // 有效data size
    ssize_t data_size = data.size() - data_start_pos;

    // 获取当前子串的下个子串时，需要考虑到 new_idx 可能会和 down_idx 重合
    pos_iter = buffer.lower_bound(new_idx); // 获取第一个大于等于new_idx的迭代器指针

    // 如果和buffer中存储的内容冲突，则截断长度
    while (pos_iter != buffer.end() && new_idx <= pos_iter->first) {
        const size_t data_end_pos = new_idx + data_size;
        // 如果存在重叠区域
        if (pos_iter->first < data_end_pos) {
            // 如果是部分重叠
            if (data_end_pos < pos_iter->first + pos_iter->second.size()) {
                data_size = pos_iter->first - new_idx;
                break;
            }
            // 如果是全部重叠
            else {
                _unassembled_bytes -= pos_iter->second.size();
                pos_iter = buffer.erase(pos_iter);
                continue;
            }
        }
        // 如果不重叠则直接 break
        else
            break;
    }







    // 检测是否超容量   
    size_t first_unacceptable_idx = _expected_index + _capacity - _output.buffer_size();
    if (first_unacceptable_idx <= new_idx) {
        return;
    }

    // 写入或缓存截取后的有效新字符串
    if (data_size > 0) {
        const string new_data = data.substr(data_start_pos, data_size);
        // 如果新字符串可以直接写入
        if (new_idx == _expected_index) {
            const size_t write_byte = _output.write(new_data);
            _expected_index += write_byte;
            // 如果没写全，缓存剩余部分
            if (write_byte < new_data.size()) {
                const string data_to_store = new_data.substr(write_byte, new_data.size() - write_byte);
                _unassembled_bytes += data_to_store.size();
                buffer.insert(make_pair(_expected_index, std::move(data_to_store)));
            }
        } else {
            // const string data_to_store = new_data.substr(0, new_data.size());
            const string data_to_store = new_data;
            _unassembled_bytes += data_to_store.size();
            buffer.insert(make_pair(new_idx, std::move(data_to_store)));
        }
    }
    // 新字符串处理完成

    // 查看缓存中是否有能直接写入的部分
    for (auto iter = buffer.begin(); iter != buffer.end();) {
        // 如果当前刚好是一个可被接收的信息
        assert(_expected_index <= iter->first);
        if (iter->first == _expected_index) {
            const size_t write_num = _output.write(iter->second);
            _expected_index += write_num;
            // _unassembled_bytes -= iter->second.size();
            // 如果没写全，则说明写满了，保留剩余没写全的部分并退出
            if (write_num < iter->second.size()) {
                _unassembled_bytes += iter->second.size() - write_num;
                buffer.insert(make_pair(_expected_index, std::move(iter->second.substr(write_num))));

                _unassembled_bytes -= iter->second.size();
                buffer.erase(iter);
                break;
            }
            // 如果写全了，则删除原有迭代器，并进行更新
            _unassembled_bytes -= iter->second.size();
            iter = buffer.erase(iter);
        }
        // 否则直接离开
        else
            break;
    }
    if (eof)
        _eof = index + data.size();
    if (_eof <= _expected_index)
        _output.end_input();



}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
