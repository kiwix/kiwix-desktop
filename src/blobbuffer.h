#ifndef BLOBBUFFER_H
#define BLOBBUFFER_H

#include <zim/blob.h>
#include <QBuffer>

class BlobBuffer : public QBuffer
{
public:
    BlobBuffer(zim::Blob m_blob);
    virtual ~BlobBuffer() = default;

private:
    zim::Blob m_blob;
};

#endif // BLOBBUFFER_H
