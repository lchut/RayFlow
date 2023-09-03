#pragma once

#include <RayFlow/Util/vecmath.h>
#include <RayFlow/Util/half.h>
#include <RayFlow/Core/spectrum.h>

namespace rayflow {

enum class BitMapPixelFormat {
    ERGB = 0,
    EFLOAT
};

enum class BitMapComponentFormat {
    EUInt8,
    EFloat16,
    EFloat32,
    EFloat64
};

enum class BitMapFileFormat {
    EPNG,
    EOPENEXR,
};

class BitMap {
public:
    BitMap() = default;

    BitMap(BitMapPixelFormat pixelFormat, BitMapComponentFormat componentFormat,
           const Point2i& resolution, uint8_t* data) :
           mPixelFormat_(pixelFormat),
           mComponentFormat_(componentFormat),
           mResolution_(resolution),
           mData_(data),
           mGamma_(false) {
        switch (mPixelFormat_) {
            case BitMapPixelFormat::EFLOAT :
                mChannelCount_ = 1;
                break;
            case BitMapPixelFormat::ERGB :
                mChannelCount_ = 3;
                break;
            default :
                mChannelCount_ = 0;
                break;
        }
    }
    
    
    BitMap(const std::string& filename, bool gamma = true);

    RAYFLOW_CPU_GPU int GetPixelCount() const { 
        return mResolution_.x * mResolution_.y;
    }
    
    RAYFLOW_CPU_GPU BitMapPixelFormat GetPixelFormat() const { return mPixelFormat_; }

    RAYFLOW_CPU_GPU Point2i GetResolution() const { return mResolution_; }
    
    template <typename T>
    RAYFLOW_CPU_GPU T GetPixel(const Point2i& pos) const; 

    RAYFLOW_CPU_GPU void SetPixel(const Point2i& pos, const Spectrum& value);

    RAYFLOW_CPU_GPU Float GetGamma() const { return mGamma_; }

    RAYFLOW_CPU_GPU void SetGamma(Float gamma) { mGamma_ = gamma; }

    RAYFLOW_CPU_GPU int GetChannelsCount() const { return size_t(mChannelCount_); }

    RAYFLOW_CPU_GPU size_t GetComponentByets() const {
        switch (mComponentFormat_) {
            case BitMapComponentFormat::EUInt8 : 
                return sizeof(uint8_t);
                break;
            case BitMapComponentFormat::EFloat16 :
                return sizeof(uint16_t);
                break;
            case BitMapComponentFormat::EFloat32 :
                return sizeof(float);
                break;
            case BitMapComponentFormat::EFloat64 :
                return sizeof(double);
                break;
            default:
                return 0;
                break;
        }
    }

    RAYFLOW_CPU_GPU size_t GetBytesPerPixel() const {
        return GetComponentByets() * GetChannelsCount();
    }

    RAYFLOW_CPU_GPU void *GetData() { return mData_; }

    RAYFLOW_CPU_GPU const void *GetData() const { return mData_; }

    RAYFLOW_CPU_GPU uint8_t *GetUInt8Data() { return mData_; }

    RAYFLOW_CPU_GPU const uint8_t *GetUInt8Data() const { return mData_; }

    RAYFLOW_CPU_GPU Half *GetFloat16Data() { return (Half *) mData_; }

    RAYFLOW_CPU_GPU const Half *GetFloat16Data() const { return (const Half *) mData_; }

    RAYFLOW_CPU_GPU float *GetFloat32Data() { return (float *) mData_; }

    RAYFLOW_CPU_GPU const float *GetFloat32Data() const { return (const float *) mData_; }

    RAYFLOW_CPU_GPU double *GetFloat64Data() { return (double *) mData_; }

    RAYFLOW_CPU_GPU const double *GetFloat64Data() const { return (const double *) mData_; }

    RAYFLOW_CPU_GPU Float *GetFloatData() { return (Float *) mData_; }

    RAYFLOW_CPU_GPU const Float *GetFloatData() const { return (const Float *) mData_; }

    RAYFLOW_CPU_GPU void Write(const std::string& filename) const;

private:
    template <typename T>
    T Convert(Float v[4]) const;

    template <>
    Float Convert(Float v[4]) const {
        return v[0];
    }

    template <>
    RGBSpectrum Convert(Float v[4]) const {
        return RGBSpectrum(v);
    }

    inline Float GammaCorrect(Float value) const {
        if (value <= 0.0031308f) {
            return 12.92f * value;
        }
        return 1.055f * std::pow(value, (Float)(1.f / 2.4f)) - 0.055f;
    }

    inline Float InverseGammaCorrect(Float value) const {
        if (value <= 0.04045f) {
            return value * 1.f / 12.92f;
        }
        return std::pow((value + 0.055f) * 1.f / 1.055f, (Float)2.4f);
    }

private:
    void Read(const std::string& filename);

    void WritePNG(const std::string& filename) const;

    void WriteOpenEXR(const std::string& filename) const;

    BitMapPixelFormat mPixelFormat_;
    BitMapComponentFormat mComponentFormat_;
    BitMapFileFormat mFileFormat_;
    Point2i mResolution_;
    uint8_t *mData_;
    bool mGamma_;
    int mChannelCount_;
};


template <typename T>
T BitMap::GetPixel(const Point2i& pos) const {
    Float values[4];
    if (pos.x < 0 || pos.x >= mResolution_.x || pos.y < 0 || pos.y >= mResolution_.y) {
        values[0] = values[1] = values[2] = values[3] = 0;
    }
    else {
        int componentBytes = GetComponentByets();
        int byteOffset = (pos.y * mResolution_.x + pos.x) * GetBytesPerPixel();

        if (mComponentFormat_ == BitMapComponentFormat::EFloat16) {
            Half buf[4];
            memcpy(buf, mData_ + byteOffset, componentBytes * mChannelCount_);
            values[0] = Float(buf[0]);
            values[1] = Float(buf[1]);
            values[2] = Float(buf[2]);
            values[3] = Float(buf[3]);
        }
    }

    return Convert<T>(values);
}
}