#include <RayFlow/Util/bitmap.h>
#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>

namespace rayflow {
BitMap::BitMap(const std::string& filename, bool gamma) : 
    mGamma_(gamma) {
    Read(filename);
}

void BitMap::SetPixel(const Point2i& pos, const Spectrum& value) {
    int channels = GetChannelsCount();
    int offset = (pos.y * mResolution_.x + pos.x) * channels;

    if (mComponentFormat_ == BitMapComponentFormat::EUInt8) {
        uint8_t* data = GetUInt8Data();
        for (int i = 0; i < channels; ++i) {
            data[offset + i] = static_cast<uint8_t>(Clamp(GammaCorrect((float)value[i]) * 255, 0, 255));
        }
    }
    else if (mComponentFormat_ == BitMapComponentFormat::EFloat16) {
        Half* data = GetFloat16Data();
        for (int i = 0; i < channels; ++i) {
            data[offset + i] = Half(float(value.values[i]));
        }
    }
    else if (mComponentFormat_ == BitMapComponentFormat::EFloat32) {
        float* data = GetFloat32Data();
        for (int i = 0; i < channels; ++i) {
            data[offset + i] = static_cast<float>(value.values[i]);
        }
    }
    else if (mComponentFormat_ == BitMapComponentFormat::EFloat64) {
        double* data = GetFloat64Data();
        for (int i = 0; i < channels; ++i) {
            data[offset + i] = static_cast<double>(value.values[i]);
        }
    }
}


void BitMap::Read(const std::string& filename) {
    uint8_t* data = stbi_load(filename.c_str(), &mResolution_.x, &mResolution_.y, &mChannelCount_, 0);
    
    std::string fileEnd = filename.substr(filename.find_last_of('.') + 1);
    if (fileEnd == "png" || fileEnd == "jpg") {
        mComponentFormat_ = BitMapComponentFormat::EFloat16;

        int pixelCount = GetPixelCount();
        Half* buffer = new Half[pixelCount * mChannelCount_];

        for (int i = 0; i < pixelCount; ++i) {
            int offset = i * mChannelCount_;
            for (int j = 0; j < mChannelCount_; ++j) {
                buffer[offset + j] = mGamma_ ? Half(GammaCorrect(data[offset + j] / 255.0f)) :              
                                               Half(InverseGammaCorrect(data[offset + j] / 255.0f));
            }
        }

        mData_ = (uint8_t*)(buffer);
    }

    stbi_image_free(data);
}

void BitMap::Write(const std::string& filename) const {
    std::string fileEnd = filename.substr(filename.find_last_of('.') + 1);
    if (fileEnd == "png") {
        WritePNG(filename);
    }
}

void BitMap::WritePNG(const std::string& filename) const {
    uint8_t* data = mData_;

    if (mComponentFormat_ != BitMapComponentFormat::EUInt8) {
        const int count = GetPixelCount();
        const Half* formatData = GetFloat16Data();
        uint8_t* buffer = new uint8_t[count];
        
        for (int i = 0; i < count; ++i) {
            int offset = i * mChannelCount_;
            
            buffer[offset] = mGamma_ ? static_cast<uint8_t>(Clamp(GammaCorrect((float)formatData[offset]) * 255, 0, 255)) : 
                                       Clamp((float)formatData[offset] * 255, 0, 255);
            buffer[offset + 1] = mGamma_ ? static_cast<uint8_t>(Clamp(GammaCorrect((float)formatData[offset + 1]) * 255, 0, 255)) :
                                           Clamp((float)formatData[offset + 1] * 255, 0, 255);
            buffer[offset + 2] = mGamma_ ? static_cast<uint8_t>(Clamp(GammaCorrect((float)formatData[offset + 2]) * 255, 0, 255)) :
                                          Clamp((float)formatData[offset + 2] * 255, 0, 255);
        }

        data = buffer;
    }

    stbi_write_png(filename.c_str(), mResolution_.x, mResolution_.y, mChannelCount_, data, 0);

    if (data != mData_) {
        delete []data;
    }
}

}