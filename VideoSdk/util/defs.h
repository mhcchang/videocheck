#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#define __WINDOWS_OS__	1
#define __LINUX_OS__	0
#else
#define __WINDOWS_OS__	0
#define __LINUX_OS__	1
#endif

#define MHC_STLNOW_SECOND  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define MHC_STLNOW_MILLISEC  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define MHC_STLNOW_MICROSEC  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

// video pixel format
#define VIDEO_FMT_NONE          0
#define VIDEO_FMT_YUYV422       1
#define VIDEO_FMT_YUV420P       2
#define VIDEO_FMT_YVYU422       3
#define VIDEO_FMT_UYVY422       4
#define VIDEO_FMT_NV12          5
#define VIDEO_FMT_NV21          6
#define VIDEO_FMT_RGB24         7
#define VIDEO_FMT_RGB32         8
#define VIDEO_FMT_ARGB          9
#define VIDEO_FMT_BGRA          10
#define VIDEO_FMT_YV12          11
#define VIDEO_FMT_BGR24         12

// video codec
#define VIDEO_CODEC_NONE        0
#define VIDEO_CODEC_H264        1
#define VIDEO_CODEC_MP4         2
#define VIDEO_CODEC_JPEG        3
#define VIDEO_CODEC_H265        4

// audio codec
#define AUDIO_CODEC_NONE        0
#define AUDIO_CODEC_G711A       1
#define AUDIO_CODEC_G711U       2
#define AUDIO_CODEC_G726        3
#define AUDIO_CODEC_AAC         4
#define AUDIO_CODEC_G722        5
#define AUDIO_CODEC_OPUS        6

#define DATA_TYPE_AUDIO 0
#define DATA_TYPE_VIDEO 1
