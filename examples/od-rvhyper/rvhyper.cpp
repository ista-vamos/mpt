#include <cassert>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "inputs.h"
#include "events.h"
#include "monitor.h"

bool InputStream::hasEvent() const { return _data[0] != nullptr; }

bool InputStream::isDone() const { return _data[0] == nullptr; }

Event *InputStream::getEvent() {
    assert(hasEvent() && "getEvent() when there is no event");

    size_t &pos = reinterpret_cast<size_t &>(_data[1]);
    ++pos;

    std::fstream *&fs = reinterpret_cast<std::fstream *&>(_data[0]);
    if (fs->eof())
        return nullptr;

    static Event_InputL I(0);
    static Event_OutputL O(0);

    // In this code, we assume that either only inputs or outputs are set,
    // and that if there is nothing set, it is a dummy event.
    // This might not be true for general traces, but for those that we use it
    // is (or it makes no difference)
    uint64_t *num = &I.data.InputL.value;
    *num = 0;
    bool in_num = false;
    unsigned n = 0;

    int c;
    char buff[64] = {0};
    while ((c = fs->get()) != -1) {
        if (std::isdigit(c)) {
            if (!in_num) {
                // START
                n = 0;
            }
            in_num = true;
            buff[n++] = c;
        } else {
            if (in_num) {
                // END

                // convert the string that holds the current number of the bit
                // into an unsigned number
                unsigned pow = 1;
                unsigned N = 0;
                while (n-- > 0) {
                    N += (buff[n] - '0') * pow;
                    pow *= 10;
                }
                assert(N < 64 && "Our variables are at most 64-bit");
                // set the Nth bit to true in the number
                *num |= 1UL << N;
            }
            in_num = false;
        }

        if (c == '\n') {
            break;
        }
        if (c == ';') {
            num = &O.data.OutputL.value;
            *num = 0;
        }
    }

    // this is the last event
    if (fs->peek() == -1) {
        // close and delete the stream
        fs->close();
        delete fs;
        fs = nullptr;

        return &O;
    }

    return &I;
}


Inputs::Inputs(char *files[], size_t) {
    char **&F = reinterpret_cast<char **&>(_data[1]);
    size_t &returned = reinterpret_cast<size_t &>(_data[0]);

    returned = 0;
    F = files;
}

bool Inputs::done() const {
    char **files = reinterpret_cast<char **>(_data[1]);
    size_t returned = reinterpret_cast<size_t>(_data[0]);
    return files[returned] == nullptr;
}

InputStream *Inputs::getNewInputStream() {
    size_t &pos = reinterpret_cast<size_t &>(_data[0]);
    char **files = reinterpret_cast<char **>(_data[1]);

    auto *file = files[pos];
    if (file == nullptr)
        return nullptr;

    std::fstream *fs = new std::fstream();
    fs->open(file, std::fstream::in);
    if (!fs->is_open()) {
        std::cerr << "Failed opening `" << file << "`\n";
        abort();
    }

    auto *stream = new InputStream(_streams.size());
    _streams.emplace_back(stream);

    reinterpret_cast<std::fstream *&>(stream->_data[0]) = fs;
    reinterpret_cast<size_t &>(stream->_data[1]) = 0;

    ++pos;

    return stream;
}
