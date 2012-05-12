// corresponding headers
#include "util.h"

QString Util::bytesToString(int bytes)
{
    qreal size = (qreal)bytes;
    QString unit;
    if (size < 1024) {
        unit = "Bytes";
    } else if (size < 1024*1024) {
        size /= 1024;
        unit = "KiB";
    } else if (size < 1024*1024*1024){
        size /= 1024*1024;
        unit = "MiB";
    } else {
        size /= 1024*1024*1024;
        unit = "GiB";
    }

    return QString("%1%2")
        .arg(size, 0, 'f', 1)
        .arg(unit)
        ;
}
