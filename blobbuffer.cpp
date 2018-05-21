#include "blobbuffer.h"

BlobBuffer::BlobBuffer(zim::Blob blob)
    : blob(blob)
{
    setData(blob.data(), blob.size());
}
