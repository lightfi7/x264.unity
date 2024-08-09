// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

x264_param_t param;
x264_picture_t pic_in, pic_out;
x264_t* encoder = nullptr;
x264_nal_t* nal = nullptr;
int i_nal = 0;
int frame_size = 0;

int InitializeEncoder(int w, int h) {
    x264_param_default_preset(&param, "veryfast", "zerolatency");
    param.i_width = w;
    param.i_height = h;
    param.i_csp = X264_CSP_I420;
    param.b_vfr_input = 0;
    // param.i_bframe = 0;

    if (x264_param_apply_profile(&param, "high") < 0) {
        return -1;
    }

    // Allocate picture for encoding
    if (x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height) < 0) {
        return -1;
    }

    // Open the encoder
    encoder = x264_encoder_open(&param);
    if (!encoder) {
        std::cerr << "Failed to open x264 encoder" << std::endl;
        return -1;
    }

    return 0;
}
// Converts RGB to YUV420 (I420) format
void ConvertRGB2YUV420(int width, int height, const char* rgbData, char** yuvData) {
    int frameSize = width * height;
    int chromaSize = frameSize / 4;

    // Allocate memory for YUV data
    *yuvData = new char[frameSize + 2 * chromaSize];

    char* yPlane = *yuvData;
    char* uPlane = yPlane + frameSize;
    char* vPlane = uPlane + chromaSize;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            int rgbIndex = index * 3;

            int r = (unsigned char)rgbData[rgbIndex];
            int g = (unsigned char)rgbData[rgbIndex + 1];
            int b = (unsigned char)rgbData[rgbIndex + 2];

            // Y component
            yPlane[index] = (char)((0.257f * r) + (0.504f * g) + (0.098f * b) + 16);

            // U and V components (subsampled by a factor of 2)
            if (y % 2 == 0 && x % 2 == 0) {
                int uvIndex = (y / 2) * (width / 2) + (x / 2);
                uPlane[uvIndex] = (char)(-(0.148f * r) - (0.291f * g) + (0.439f * b) + 128);
                vPlane[uvIndex] = (char)((0.439f * r) - (0.368f * g) - (0.071f * b) + 128);
            }
        }
    }
}

int EncodeFrame(int w, int h, const char* in_buffer, char** out_buffer) {

    // Convert RGB to YUV
    char* YUVData = nullptr;
    ConvertRGB2YUV420(w, h, in_buffer, &YUVData);

    if (YUVData != nullptr) {
        memcpy(pic_in.img.plane[0], YUVData, w * h); // Y plane
        memcpy(pic_in.img.plane[1], YUVData + w * h, w * h / 4); // U plane
        memcpy(pic_in.img.plane[2], YUVData + w * h + w * h / 4, w * h / 4); // V plane

        delete[] YUVData; // Free YUVData after copying
    }

    // Encode the frame
    frame_size = x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
    if (frame_size < 0) {
        std::cerr << "x264 encoding failed" << std::endl;
        return -1;
    }

    if (frame_size > 0) {
        *out_buffer = new char[frame_size];
        memcpy(*out_buffer, nal[0].p_payload, frame_size);
    }
    
    return frame_size;
}

void DisposeEncoder() {
    x264_picture_clean(&pic_in);
    x264_encoder_close(encoder);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

