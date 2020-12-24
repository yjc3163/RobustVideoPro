#define _CRT_SECURE_NO_WARNINGS
#define __STDC_CONSTANT_MACROS

#include "stdio.h"
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libavutil\channel_layout.h"
#include "libavutil\common.h"
#include "libavutil\imgutils.h"
#include "libswscale\swscale.h"    
#include "libavutil\opt.h" 
#include "libavutil\mathematics.h"
#include "libavutil\samplefmt.h" 
#include "libavfilter\avfilter.h"
int main()
{
	char * VideoPathIn = "1.mp4";
	char *VideoPathOut = "stego.mp4";
	char *temp264 = "temp.264";
	int qStep = 60; //QIM步长
	//编码的一些参数
	AVRational frame_rate;
	int ret = 0;
	int frameNum = 0;
	int frameHeight = 0;
	int frameWidth = 0;
	/************Ddecode the video************/
	//avcodec_register_all();

	//封装格式上下文
	AVFormatContext *pFormatCoctx = avformat_alloc_context();//这个结构体描述了一个媒体文件或媒体流的构成和基本信息
	AVCodecContext *pDecodeCtx = avcodec_alloc_context3(NULL);
	//打开输入视频文件
	if (avformat_open_input(&pFormatCoctx, VideoPathIn, NULL, NULL) != 0) {
		printf("打开输入视频文件失败\n");
		return -1;
	}

	//获取视频信息
	if (avformat_find_stream_info(pFormatCoctx, NULL) < 0) {
		printf("获取视频信息失败\n");
		return -1;
	}

	//视频解码，需要找到视频对应的AVStream所在pFormatCoctx->streams的索引位置
	int video_stream_idx = -1;
	int i = 0;
	for (; i < pFormatCoctx->nb_streams; i++) {
		//根据类型判断，是否是视频流
		if (pFormatCoctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_idx = i;
			break;
		}
	}

	//获取视频解码器
	avcodec_parameters_to_context(pDecodeCtx, pFormatCoctx->streams[video_stream_idx]->codecpar);
	AVCodec *pDecodec = avcodec_find_decoder(pDecodeCtx->codec_id);

	if (pDecodec == NULL) {
		printf("无法解码\n");
		return -1;
	}

	//打开解码器
	if (avcodec_open2(pDecodeCtx, pDecodec, NULL) < 0) {
		printf("解码器无法打开");
		return -1;
	}

	//编码数据
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//像素数据（解码数据）
	AVFrame *frame = av_frame_alloc();
	AVFrame *yuvFrame = av_frame_alloc();
	yuvFrame->format = pDecodeCtx->pix_fmt;
	yuvFrame->width = pDecodeCtx->width;
	yuvFrame->height = pDecodeCtx->height;

	//只有指定了AVFrame的像素格式、画面大小才能真正分配内存
	//缓冲区分配内存
	uint8_t *out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height, 1));
	//初始化缓冲区
	av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, out_buffer, AV_PIX_FMT_YUV420P, pDecodeCtx->width, pDecodeCtx->height, 1);

	frameHeight = pDecodeCtx->height;
	frameWidth = pDecodeCtx->width;
	//用于像素格式转换或者缩放，这里换成420p
	struct SwsContext *sws_ctx = sws_getContext(
		pDecodeCtx->width, pDecodeCtx->height, pDecodeCtx->pix_fmt,
		pDecodeCtx->width, pDecodeCtx->height, AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, NULL, NULL, NULL);

	/*************** Encoder ***************/
	AVFormatContext *pFormatDectx = NULL;
	avformat_alloc_output_context2(&pFormatDectx, NULL, NULL, temp264);
	if (avio_open(&pFormatDectx->pb, temp264, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Failed to open output file! \n");
		return -1;
	}

	AVStream* video_st = avformat_new_stream(pFormatDectx, 0);

	frameNum = (int)(pFormatCoctx->streams[0]->nb_frames);
	video_st->time_base.num = pFormatCoctx->streams[0]->avg_frame_rate.den;
	video_st->time_base.den = pFormatCoctx->streams[0]->avg_frame_rate.num;
	//av_dump_format(pFormatDectx, 0, temp264, 1);
	AVCodec *Codec = avcodec_find_encoder(AV_CODEC_ID_H264);//查找编解码器
	if (NULL == Codec)
	{
		printf("找不到编码器\n");
		return -1;
	}
	//分配AVCodecContex实例
	AVCodecContext *pCodeCtx = avcodec_alloc_context3(Codec);
	if (NULL == pCodeCtx)
	{
		printf("找不到编码器上下文\n");
		return -1;
	}
	//设置编码器的参数
	pCodeCtx->width = pDecodeCtx->width;//帧高
	pCodeCtx->height = pDecodeCtx->height;
	pCodeCtx->bit_rate = pDecodeCtx->bit_rate;//比特率

	pCodeCtx->time_base.num = pFormatCoctx->streams[0]->avg_frame_rate.den;
	pCodeCtx->time_base.den = pFormatCoctx->streams[0]->avg_frame_rate.num;
	pCodeCtx->gop_size = pDecodeCtx->gop_size;
	pCodeCtx->max_b_frames = 0;//实时编码
	pCodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	//H264
	pCodeCtx->me_range = 16;
	pCodeCtx->max_qdiff = pDecodeCtx->max_qdiff;
	pCodeCtx->qcompress = pDecodeCtx->qcompress;
	pCodeCtx->qmin = 10;
	pCodeCtx->qmax = 31;
	av_opt_set(pCodeCtx->priv_data, "preset", "fast", 0);
	av_opt_set(pCodeCtx->priv_data, "tune", "zerolatency", 0);
	av_opt_set(pCodeCtx->priv_data, "crf", "18", 0);

	AVDictionary *param = 0;
	//打开编码器
	if (avcodec_open2(pCodeCtx, Codec, &param) < 0)
	{
		printf("%d", avcodec_open2(pCodeCtx, Codec, &param));
		printf("打不开编码器\n");
		return -1;
	}

	avcodec_parameters_from_context(video_st->codecpar, pCodeCtx);
//	av_dump_format(pFormatDectx, 0, temp264, 1);

	//写入头文件
	avformat_write_header(pFormatDectx, NULL);
	AVPacket pkt;

	int got_frame = 0;
	int framecount = 0;

	//DE-block
	int BlockSize = 16;
	int BlockRows = frameHeight / BlockSize;
	int BlockCols = frameWidth / BlockSize;
	int BlockNumPerFrame = BlockCols * BlockRows;
	/*************DWT参数**************/
	double **BlockData = (double**)malloc(BlockSize * sizeof(double*));
	for (int i = 0; i < BlockSize; i++) {
		BlockData[i] = (double*)malloc(BlockSize * sizeof(double));
	}
	double **ReconData = (double**)malloc(BlockSize * sizeof(double*));
	for (int i = 0; i < BlockSize; i++) {
		ReconData[i] = (double*)malloc(BlockSize * sizeof(double));
	}

	double **LL = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		LL[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **ReconLL = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		ReconLL[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **LH = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		LH[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **HL = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		HL[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **HH = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		HH[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	/*************DWT参数**************/
	/************************************svd参数*********************************/
	double **U = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		U[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **V = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		V[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **UT = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		UT[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double **TempUTS = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		TempUTS[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	double *S = (double*)malloc((BlockSize / 2) * sizeof(double));
	double **Smat = (double**)malloc((BlockSize / 2) * sizeof(double*));
	for (int i = 0; i < BlockSize / 2; i++) {
		Smat[i] = (double*)malloc((BlockSize / 2) * sizeof(double));
	}
	/************************************svd参数*********************************/
	//一帧一帧读取压缩的视频数据AVPacket
	while (av_read_frame(pFormatCoctx, packet) >= 0)
	{
		if (packet->stream_index == video_stream_idx) 
		{
			//解码AVPacket->AVFrame
			avcodec_send_packet(pDecodeCtx, packet);
			got_frame = avcodec_receive_frame(pDecodeCtx, frame);
			//非零正在解码
			if (!got_frame) 
			{
				//frame->yuvFrame (YUV420P)
				//转为指定的YUV420P像素帧
				sws_scale(sws_ctx,
					frame->data, frame->linesize, 0, frame->height,
					yuvFrame->data, yuvFrame->linesize);
				//向YUV文件保存解码之后的帧数据
				int y_size = pDecodeCtx->width * pDecodeCtx->height;
				printf("Embedding:%d帧\n", framecount);
				/******* Embedding yuvFrame->data[0] Here *******/
				uint8_t *y_data = yuvFrame->data[0];
				for (int BlockIdx = 0; BlockIdx < BlockNumPerFrame; BlockIdx++)
				{
					int rowIdx = BlockIdx / BlockCols;
					int colIdx = BlockIdx % BlockCols;
					for (int x = 0; x < BlockSize; x++)
					{
						for (int y = 0; y < BlockSize; y++)
						{
							BlockData[x][y] = (double)(y_data[(rowIdx * BlockSize + x) * frameWidth + colIdx * BlockSize + y]);
						}
					}

					//DWT-SVD Modulation
					DWT2D(BlockData, BlockSize, BlockSize, LL, LH, HL, HH);
					svd(LL, BlockSize / 2, BlockSize / 2, BlockSize / 2, U, S, V);
					int MessageBit = 1;//先不管怎么样都嵌入1
					S[0] = floor(S[0] / qStep)*qStep + (((int)floor(S[0] / qStep + MessageBit)) % 2)*qStep;

					//Reconstruct
					for (int x = 0; x < BlockSize / 2; x++) {
						for (int y = 0; y < BlockSize / 2; y++) {
							if (x == y) Smat[x][y] = S[x];
							else Smat[x][y] = 0;
						}
					}
					matInv(U, BlockSize / 2, BlockSize / 2, UT);
					matMul(UT, BlockSize / 2, BlockSize / 2, Smat, BlockSize / 2, BlockSize / 2, TempUTS);
					matMul(TempUTS, BlockSize / 2, BlockSize / 2, V, BlockSize / 2, BlockSize / 2, ReconLL);
					IDWT2D(ReconData, BlockSize / 2, BlockSize / 2, ReconLL, LH, HL, HH);


					for (int x = 0; x < BlockSize; x++)
					{
						for (int y = 0; y < BlockSize; y++)
						{
							if (ReconData[x][y] > 255)
								ReconData[x][y] = 255;
							if (ReconData[x][y] < 0)
								ReconData[x][y] = 0;
							y_data[(rowIdx * BlockSize + x) * frameWidth + colIdx * BlockSize + y] = (uint8_t)(ReconData[x][y]);
						}
					}

				}

				uint8_t *yData = yuvFrame->data[0];//???
				yuvFrame->pts = framecount;
				av_init_packet(&pkt);
				avcodec_send_frame(pCodeCtx, (const AVFrame *)yuvFrame);
				int got_packet = avcodec_receive_packet(pCodeCtx, &pkt);
				if (!got_packet) 
				{
					//LOGI("编码输出%d帧", framecount);
					framecount++;
					pkt.stream_index = video_st->index;
					av_packet_rescale_ts(&pkt, pCodeCtx->time_base, video_st->time_base);
					frame_rate.den = pCodeCtx->time_base.num;
					frame_rate.num = pCodeCtx->time_base.den;
					pkt.pos = -1;
					av_interleaved_write_frame(pFormatDectx, &pkt);
					av_packet_unref(&pkt);
				}
			}
			
		}
		av_packet_unref(packet);

		//flush decoder
		//FIX: Flush Frames remained in Codec
		

	}

	while (1)
	{
		//解码AVPacket->AVFrame
		avcodec_send_packet(pDecodeCtx, packet);
		got_frame = avcodec_receive_frame(pDecodeCtx, frame);
		if (got_frame)
			break;
		sws_scale(sws_ctx,
			frame->data, frame->linesize, 0, frame->height,
			yuvFrame->data, yuvFrame->linesize);
		int y_size = pDecodeCtx->width * pDecodeCtx->height;
		printf("Embedding:%d帧\n", framecount);

		uint8_t *y_data = yuvFrame->data[0];
		for (int BlockIdx = 0; BlockIdx < BlockNumPerFrame; BlockIdx++)
		{
			int rowIdx = BlockIdx / BlockCols;
			int colIdx = BlockIdx % BlockCols;
			for (int x = 0; x < BlockSize; x++)
			{
				for (int y = 0; y < BlockSize; y++)
				{
					BlockData[x][y] = (double)(y_data[(rowIdx * BlockSize + x) * frameWidth + colIdx * BlockSize + y]);
				}
			}

			//DWT-SVD Modulation
			DWT2D(BlockData, BlockSize, BlockSize, LL, LH, HL, HH);
			svd(LL, BlockSize / 2, BlockSize / 2, BlockSize / 2, U, S, V);
			int MessageBit = 1;
			S[0] = floor(S[0] / qStep)*qStep + (((int)floor(S[0] / qStep + MessageBit)) % 2)*qStep;

			//Reconstruct
			for (int x = 0; x < BlockSize / 2; x++)
			{
				for (int y = 0; y < BlockSize / 2; y++)
				{
					if (x == y) Smat[x][y] = S[x];
					else Smat[x][y] = 0;
				}
			}
			matInv(U, BlockSize / 2, BlockSize / 2, UT);
			matMul(UT, BlockSize / 2, BlockSize / 2, Smat, BlockSize / 2, BlockSize / 2, TempUTS);
			matMul(TempUTS, BlockSize / 2, BlockSize / 2, V, BlockSize / 2, BlockSize / 2, ReconLL);
			IDWT2D(ReconData, BlockSize / 2, BlockSize / 2, ReconLL, LH, HL, HH);

			for (int x = 0; x < BlockSize; x++)
			{
				for (int y = 0; y < BlockSize; y++)
				{
					if (ReconData[x][y] > 255)
						ReconData[x][y] = 255;
					if (ReconData[x][y] < 0)
						ReconData[x][y] = 0;
					y_data[(rowIdx * BlockSize + x) * frameWidth + colIdx * BlockSize + y] = (uint8_t)(ReconData[x][y]);
				}
			}

		}

		yuvFrame->pts = framecount;
		av_init_packet(&pkt);
		avcodec_send_frame(pCodeCtx, (const AVFrame *)yuvFrame);
		int got_packet = avcodec_receive_packet(pCodeCtx, &pkt);
		if (!got_packet)
		{
			//LOGI("编码输出%d帧", framecount);
			framecount++;
			pkt.stream_index = video_st->index;
			av_packet_rescale_ts(&pkt, pCodeCtx->time_base, video_st->time_base);
			frame_rate.den = pCodeCtx->time_base.num;
			frame_rate.num = pCodeCtx->time_base.den;
			pkt.pos = -1;
			av_interleaved_write_frame(pFormatDectx, &pkt);
			av_packet_unref(&pkt);
		}
	}
	/********free malloc memory********/
	for (int i = 0; i < BlockSize; i++) {
		free(BlockData[i]);
		free(ReconData[i]);
	}
	free(BlockData);
	free(ReconData);
	BlockData = NULL;
	ReconData = NULL;

	for (int i = 0; i < BlockSize / 2; i++) {
		free(LL[i]);
		free(LH[i]);
		free(HL[i]);
		free(HH[i]);
		free(ReconLL[i]);
		free(U[i]);
		free(V[i]);
		free(UT[i]);
		free(TempUTS[i]);
		free(Smat[i]);
	}
	free(LL);
	free(HL);
	free(LH);
	free(HH);
	free(ReconLL);
	free(U);
	free(V);
	free(UT);
	free(TempUTS);
	free(Smat);

	free(S);
	S = NULL;
	U = NULL;
	V = NULL;
	UT = NULL;
	TempUTS = NULL;
	Smat = NULL;
	LL = NULL;
	HL = NULL;
	LH = NULL;
	HH = NULL;
	ReconLL = NULL;

	sws_freeContext(sws_ctx);
	av_write_trailer(pFormatDectx);
	avio_close(pFormatDectx->pb);

	/*************** encapsulate h264 to mp4 ***************/
	AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext
	AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL, *ofmt_ctx = NULL;
	int videoindex_v = 0, videoindex_out = 0;
	int frame_index = 0;
	int64_t cur_pts_v = 0, cur_pts_a = 0;

	//av_register_all();
	//Input
	if ((ret = avformat_open_input(&ifmt_ctx_v, temp264, 0, 0)) < 0) {
		printf("Could not open input file.");
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		goto end;
	}

	//av_dump_format(ifmt_ctx_v, 0, h264_output, 0);
	//Output
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, VideoPathOut);
	if (!ofmt_ctx) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	printf("ifmt_ctx_v->nb_streams=%d\n", ifmt_ctx_v->nb_streams);

	AVCodecContext* pCodecCtx = avcodec_alloc_context3(NULL);
	for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		//if(ifmt_ctx_v->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		//{
		AVStream *in_stream = ifmt_ctx_v->streams[i];
		avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);
		AVCodec *Mcodec = avcodec_find_decoder(pCodecCtx->codec_id);
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, Mcodec);
		videoindex_v = i;
		videoindex_out = out_stream->index;
		//Copy the settings of AVCodecContext
		if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
			printf("Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codecpar->codec_tag = 0;
		//}
	}

	//av_dump_format(ofmt_ctx, 0, output, 1);
	//Open output file
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx->pb, VideoPathOut, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", VideoPathOut);
			goto end;
		}
	}
	//Write file header
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("Error occurred when opening output file\n");
		goto end;
	}
	//FIX
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
	AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif
	while (1) {
		AVFormatContext *ifmt_ctx;
		int stream_index = 0;
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		//if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0)
		{

			ifmt_ctx = ifmt_ctx_v;
			stream_index = videoindex_out;
			if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
				do {
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];
					if (pkt.stream_index == videoindex_v) {
						//FIX：No PTS (Example: Raw H.264)
						//Simple Write PTS
						if (pkt.pts == AV_NOPTS_VALUE) {
							printf("frame_index==%d\n", frame_index);
							//Write PTS
							in_stream->r_frame_rate = frame_rate;
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_v = pkt.pts;
						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else {
				break;
			}
		}
#if USE_H264BSF
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;
		printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error muxing packet\n");
			break;
		}
		av_packet_unref(&pkt);
	}
	//Write file trailer
	av_write_trailer(ofmt_ctx);
#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
#endif
end:
	if (pCodecCtx != NULL)
	{
		//avcodec_flush_buffers(pCodecCtx);
		avcodec_close(pCodecCtx);
		avcodec_free_context(&pCodecCtx);
	}

	avformat_close_input(&ifmt_ctx_v);
	avformat_free_context(ifmt_ctx_v);
	//avformat_close_input(&ifmt_ctx_a);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return -1;
	}
	av_frame_free(&frame);
	av_free(out_buffer);
	out_buffer = NULL;
	av_frame_free(&yuvFrame);
	avcodec_flush_buffers(pCodeCtx);
	avcodec_flush_buffers(pDecodeCtx);
	avcodec_close(pDecodeCtx);
	avcodec_close(pCodeCtx);
	avcodec_free_context(&pDecodeCtx);
	avcodec_free_context(&pCodeCtx);
	avformat_close_input(&pFormatCoctx);
	avformat_free_context(pFormatCoctx);
	avformat_free_context(pFormatDectx);
	//avformat_free_context(pFormatCoctx);
	//avformat_free_context(pFormatDectx);
	//av_free(pDecodeCtx);
	//av_free(pCodeCtx);

	remove(temp264);



	return 0;
}