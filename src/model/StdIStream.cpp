#include "StdIStream.h"

StdIStream::StdIStream(std::istream& stream)
  : IStream("Stream")
  , m_stream(stream)
{}

bool StdIStream::read(char c[], int n)
{
    m_stream.read(c, n);

    return m_stream.good();
}

uint64_t StdIStream::tellg()
{
    return m_stream.tellg();
}

void StdIStream::seekg(uint64_t pos)
{
    m_stream.seekg(pos);
}
