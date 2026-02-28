#include "envireader.h"

#include <QFileInfo>
#include <QtEndian>

EnviReader::~EnviReader()
{
    close();
}

bool EnviReader::open(const QString& hdrPath)
{
    if (!parseHdr(hdrPath))
        return false;

    QFileInfo info(hdrPath);

    // 直接生成同名数据文件完整路径
    QString rawPath =
        info.absolutePath() + "/" + info.completeBaseName();

    // 如果数据文件有扩展名（比如 .raw)
    if (!QFile::exists(rawPath))
        rawPath = info.absolutePath() + "/" +
                  info.completeBaseName() + ".raw";

    if (!QFile::exists(rawPath))
    {
        qDebug() << "Raw file not found:" << rawPath;
        return false;
    }

    m_file.setFileName(rawPath);

    if (!m_file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Open failed:" << rawPath;
        return false;
    }

    m_mapSize = m_file.size();
    m_mapped = m_file.map(0, m_mapSize);

    return m_mapped != nullptr;
}

void EnviReader::close()
{
    if (m_mapped)
    {
        m_file.unmap(m_mapped);
        m_mapped = nullptr;
    }

    if (m_file.isOpen())
        m_file.close();
}

quint16 EnviReader::value(int band, int line, int sample) const
{
    if (!m_mapped)
        return 0;

    if (m_header.data_type != 12)
        return 0; // 目前只支持 uint16

    qint64 idx = indexBSQ(band, line, sample);

    const quint16* data =
        reinterpret_cast<const quint16*>(m_mapped + m_header.header_offset);

    quint16 v = data[idx];

    // 如果是大端
    if (m_header.byte_order == 1)
        v = qFromBigEndian(v);

    return v;
}

bool EnviReader::parseHdr(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    bool inBrace = false;
    QString currentKey;
    QString buffer;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        // 处理多行 { }
        if (inBrace)
        {
            buffer += line;

            if (line.contains("}"))
            {
                inBrace = false;

                buffer.remove("{");
                buffer.remove("}");

                QStringList list = buffer.split(",", Qt::SkipEmptyParts);

                if (currentKey == "wavelength")
                {
                    for (auto& s : list)
                        m_header.wavelength.append(s.trimmed());
                }
                else if (currentKey == "description")
                {
                    m_header.description = buffer.trimmed();
                }

                buffer.clear();
            }
            continue;
        }

        if (!line.contains("="))
            continue;

        QStringList parts = line.split("=");
        if (parts.size() < 2)
            continue;

        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();

        // 多行块开始
        if (value.startsWith("{") && !value.contains("}"))
        {
            inBrace = true;
            currentKey = key;
            buffer = value;
            continue;
        }

        // 单行 { }
        if (value.startsWith("{") && value.contains("}"))
        {
            value.remove("{");
            value.remove("}");

            if (key == "wavelength")
            {
                QStringList list = value.split(",", Qt::SkipEmptyParts);
                for (auto& s : list)
                    m_header.wavelength.append(s.trimmed());
            }
            else if (key == "description")
            {
                m_header.description = value.trimmed();
            }
            continue;
        }

        // 普通字段
        if (key == "samples")
            m_header.samples = value.toInt();
        else if (key == "lines")
            m_header.lines = value.toInt();
        else if (key == "bands")
            m_header.bands = value.toInt();
        else if (key == "header offset")
            m_header.header_offset = value.toInt();
        else if (key == "file type")
            m_header.file_type = value;
        else if (key == "data type")
            m_header.data_type = value.toInt();
        else if (key == "byte order")
            m_header.byte_order = value.toInt();
        else if (key == "x start")
            m_header.x_start = value.toInt();
        else if (key == "y start")
            m_header.y_start = value.toInt();
        else if (key == "interleave")
            m_header.interleave = value;
        else if (key == "wavelength units")
            m_header.wavelength_units = value;
    }

    return true;
}

inline qint64 EnviReader::indexBSQ(int b, int y, int x) const
{
    return static_cast<qint64>(b) * m_header.lines * m_header.samples
           + static_cast<qint64>(y) * m_header.samples
           + x;
}
