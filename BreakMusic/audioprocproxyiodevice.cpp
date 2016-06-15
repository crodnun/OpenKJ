#include "audioprocproxyiodevice.h"
#include "smbPitchShift/smbPitchShift.fftw3.h"
#include <QtDebug>
#include <QApplication>
#include <QMutexLocker>


AudioProcProxyIODevice::AudioProcProxyIODevice(QBuffer *audioBuffer, QAudioFormat format, QObject *parent) : QIODevice(parent)
{
    m_keyChange = 1.0;
    m_format = format;
    this->audioBuffer = audioBuffer;
    buffering = false;
    pitchShifter = new PitchShifter(2048,32,44100);
}

void AudioProcProxyIODevice::setFormat(QAudioFormat format)
{
    m_format = format;
}


bool AudioProcProxyIODevice::isSequential() const
{
    return audioBuffer->isSequential();
}

qint64 AudioProcProxyIODevice::pos() const
{
    return audioBuffer->pos();
}

qint64 AudioProcProxyIODevice::size() const
{
    return audioBuffer->size();
}

bool AudioProcProxyIODevice::seek(qint64 pos)
{
    return audioBuffer->seek(pos);
}

bool AudioProcProxyIODevice::atEnd() const
{
    return audioBuffer->atEnd();
}

bool AudioProcProxyIODevice::reset()
{
    return audioBuffer->reset();
}

qint64 AudioProcProxyIODevice::bytesAvailable() const
{
    return audioBuffer->bytesAvailable();
}

qint64 AudioProcProxyIODevice::bytesToWrite() const
{
    return audioBuffer->bytesToWrite();
}

bool AudioProcProxyIODevice::canReadLine() const
{
    return audioBuffer->canReadLine();
}

bool AudioProcProxyIODevice::waitForReadyRead(int msecs)
{
    return audioBuffer->waitForBytesWritten(msecs);
//    qCritical() << "waitForReadyRead() called";
//    while (buffering)
//        QApplication::processEvents();
//    return true;
}

bool AudioProcProxyIODevice::waitForBytesWritten(int msecs)
{
    return audioBuffer->waitForBytesWritten(msecs);
}

qint64 AudioProcProxyIODevice::readData(char *data, qint64 maxlen)
{
    int requestSize = maxlen;
    if (maxlen > audioBuffer->bytesAvailable())
    {
        requestSize = audioBuffer->bytesAvailable();
    }
    qint64 samplesSize = audioBuffer->read(data, requestSize);
    if (m_keyChange != 1.0)
    {
        pitchShifter->smbPitchShift(m_keyChange, m_format.framesForBytes(samplesSize) * m_format.channelCount(), 2048, 32, m_format.sampleRate(),(float*)data,(float*)data);
    }
    return requestSize;
}

qint64 AudioProcProxyIODevice::readLineData(char *data, qint64 maxlen)
{
    return audioBuffer->readLine(data, maxlen);
}


float AudioProcProxyIODevice::keyChange() const
{
    return m_keyChange;
}

void AudioProcProxyIODevice::setKeyChange(float keyChange)
{
    m_keyChange = keyChange;
}


qint64 AudioProcProxyIODevice::writeData(const char *data, qint64 len)
{
    // We don't write through this proxy, only read
    Q_UNUSED(data)
    Q_UNUSED(len)
    return -1;
}
