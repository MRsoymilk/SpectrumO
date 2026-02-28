#ifndef ENVIREADER_H
#define ENVIREADER_H

#include <QFile>
#include <QString>
#include <QVector>

struct EnviHeader
{
    QString description = "";
    int samples = 0;
    int lines = 0;
    int bands = 0;
    int header_offset = 0;
    QString file_type = "";
    int data_type = 0;
    int byte_order = 0;
    int x_start = 0;
    int y_start = 0;
    QString interleave;
    QString wavelength_units;
    QVector<QString> wavelength;

    QString toString() const {
        return QString("description: %1\n"
                       "samples: %2\n"
                       "lines: %3\n"
                       "bands: %4\n"
                       "header offset: %5\n"
                       "file type: %6\n"
                       "data type: %7\n"
                       "byte order: %8\n"
                       "x start: %9\n"
                       "y start: %10\n"
                       "interleave: %11\n"
                       "wavelength units: %12\n"
                       "wavelength: {%13}\n")
            .arg(description)
            .arg(samples)
            .arg(lines)
            .arg(bands)
            .arg(header_offset)
            .arg(file_type)
            .arg(data_type)
            .arg(byte_order)
            .arg(x_start)
            .arg(y_start)
            .arg(interleave)
            .arg(wavelength_units)
            .arg(wavelength.join(", "));
    }
};

class EnviReader
{
public:
    EnviReader() = default;
    ~EnviReader();

    bool open(const QString& hdrPath);
    void close();

    quint16 value(int band, int line, int sample) const;

    const EnviHeader& header() const { return m_header; }

private:
    bool parseHdr(const QString& path);
    inline qint64 indexBSQ(int b, int y, int x) const;

private:
    EnviHeader m_header;
    QFile m_file;
    uchar* m_mapped = nullptr;
    qint64 m_mapSize = 0;
};

#endif // ENVIREADER_H
