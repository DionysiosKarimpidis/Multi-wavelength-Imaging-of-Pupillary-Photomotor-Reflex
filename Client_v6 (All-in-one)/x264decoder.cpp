#include "x264decoder.h"
#include <QDebug>



static AVFrame * icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height, bool alloc)
{
    AVFrame * picture;
    uint8_t * picture_buf;
    int size;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    size = avpicture_get_size( (AVPixelFormat) pix_fmt, width, height);
    if(alloc)
    {
        picture_buf = (uint8_t *) malloc(size);
        if (!picture_buf)
        {
            av_frame_free (&picture);
            std::cout << "picture buff = NULL" << std::endl;
            return NULL;
        }
        avpicture_fill((AVPicture *)picture, picture_buf,  (AVPixelFormat) pix_fmt, width, height);
    }
    return picture;
}


void x264Decoder::initialize(int w = 640, int h = 480)
{
    avcodec_register_all();
    image_h_ = h;
    image_w_ = w;

    rgb_size_ = avpicture_get_size(AV_PIX_FMT_RGB24, image_w_, image_h_);
    av_codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    av_codec_context_ = avcodec_alloc_context3(av_codec_);
    av_codec_context_->width = image_w_;
    av_codec_context_->height = image_h_;
    av_codec_context_->extradata = NULL;
    av_codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;

//    av_codec_context_->bit_rate = 3000000;
//    av_codec_context_->framerate.den = 1;
//    av_codec_context_->framerate.num = 20;
    av_codec_context_->profile = FF_PROFILE_H264_HIGH;
    av_codec_context_->level = 42;

    av_codec_context_->skip_frame = AVDISCARD_DEFAULT;
    av_codec_context_->thread_count=1;
    av_codec_context_->qmin = 14;
    av_codec_context_->qmax = 51;

    avcodec_open2(av_codec_context_, av_codec_, NULL);

    convert_context_ = sws_getContext(image_w_, image_h_, AV_PIX_FMT_YUV420P, image_w_, image_h_, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    av_frame_ = icv_alloc_picture_FFMPEG(AV_PIX_FMT_YUV420P, image_w_, image_h_, true);
    av_frame_RGB_ = icv_alloc_picture_FFMPEG(AV_PIX_FMT_RGB24, image_w_, image_h_, true);

}

bool x264Decoder::decodeFrame(char *src_buffer, unsigned int size, char *rgb_buffer, int *used)
{
    AVPacket av_packet;
    av_new_packet(&av_packet, size);
    av_packet.data = (uint8_t *)src_buffer;
    av_packet.size = size;

    int frame_finished = 0;

    int av_return = avcodec_decode_video2(av_codec_context_, av_frame_, &frame_finished, &av_packet );

    if(used != NULL)
       *used = av_return;

    if(av_return <= 0 || !frame_finished)
    {

        return false;
    }
    //Convert the frame from YUV420 to RGB24
    sws_scale(convert_context_, av_frame_->data, av_frame_->linesize, 0, image_h_, av_frame_RGB_->data, av_frame_RGB_->linesize);
    //Manadatory function to copy the image form an AVFrame to a generic buffer.
    avpicture_layout((AVPicture *)av_frame_RGB_, AV_PIX_FMT_RGB24, image_w_, image_h_, (unsigned char *)rgb_buffer, rgb_size_);


    return true;
}

void x264Decoder::close()
{
    avcodec_close(av_codec_context_);
}


