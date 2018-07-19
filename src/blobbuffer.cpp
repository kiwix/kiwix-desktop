#include "blobbuffer.h"

BlobBuffer::BlobBuffer(zim::Blob blob)
    : m_blob(blob)
{
    setData(blob.data(), blob.size());
}
