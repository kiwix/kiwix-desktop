#ifndef BLOBBUFFER_H
#define BLOBBUFFER_H

#include <zim/blob.h>
#include <QBuffer>

class BlobBuffer : public QBuffer
{
public:
    BlobBuffer(zim::Blob blob);

private:
    zim::Blob blob;
};

#endif // BLOBBUFFER_H
