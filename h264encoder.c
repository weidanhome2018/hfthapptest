#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "h264encoder.h"


static int x264_param_apply_preset( x264_param_t *param, const char *preset )
{
    char *end;
    int i = strtol( preset, &end, 10 );
    if( *end == 0 && i >= 0 && i < sizeof(x264_preset_names)/sizeof(*x264_preset_names)-1 )
        preset = x264_preset_names[i];

    if( !strcasecmp( preset, "ultrafast" ) )
    {
        param->i_frame_reference = 1;//参考帧的最大帧数设为1
        param->i_scenecut_threshold = 0;
        param->b_deblocking_filter = 0;//不使用去块滤波  
        param->b_cabac = 0;//关闭cabac
        param->i_bframe = 0;//关闭b帧
        param->analyse.intra = 0;
        param->analyse.inter = 0;
        param->analyse.b_transform_8x8 = 0;
        param->analyse.i_me_method = X264_ME_DIA;;////运动估算法的选择 
        param->analyse.i_subpel_refine = 0;
        param->rc.i_aq_mode = 0;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->i_bframe_adaptive = X264_B_ADAPT_NONE;//关闭b帧判定选项
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
        param->analyse.b_weighted_bipred = 0;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "superfast" ) )
    {
        param->analyse.inter = X264_ANALYSE_I8x8|X264_ANALYSE_I4x4;
        param->analyse.i_me_method = X264_ME_DIA;;//钻石模板  
        param->analyse.i_subpel_refine = 1;//亚像素运动估计质量为1  
        param->i_frame_reference = 1;//参考帧的最大帧数设为1
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "veryfast" ) )
    {
        param->analyse.i_me_method = X264_ME_HEX;
        param->analyse.i_subpel_refine = 2;
        param->i_frame_reference = 1;//参考帧的最大帧数设为1
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 10;
    }
    else if( !strcasecmp( preset, "faster" ) )
    {
        param->analyse.b_mixed_references = 0;
        param->i_frame_reference = 2;//参考帧的最大帧数设为2
        param->analyse.i_subpel_refine = 4;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 20;
    }
    else if( !strcasecmp( preset, "fast" ) )
    {
        param->i_frame_reference = 2;//参考帧的最大帧数设为2
        param->analyse.i_subpel_refine = 6;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 30;
    }
    else if( !strcasecmp( preset, "medium" ) )
    {
        /* Default is medium 
           *默认参考 set_param_default();
            */
    }
    else if( !strcasecmp( preset, "slow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;//运动估算发的选择
        param->analyse.i_subpel_refine = 8;
        param->i_frame_reference = 5;//参考帧的最大帧数设为5
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->rc.i_lookahead = 50;
    }
    else if( !strcasecmp( preset, "slower" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;//运动估算发的选择
        param->analyse.i_subpel_refine = 9;
        param->i_frame_reference = 8;//参考帧的最大帧数设为8
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->rc.i_lookahead = 60;
    }
    else if( !strcasecmp( preset, "veryslow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;//运动估算发的选择
        param->analyse.i_subpel_refine = 10;
        param->analyse.i_me_range = 24;
        param->i_frame_reference = 16;//参考帧的最大帧数设为16
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->i_bframe = 8;//两个参考帧之间b帧为8
        param->rc.i_lookahead = 60;
    }
    else if( !strcasecmp( preset, "placebo" ) )
    {
        param->analyse.i_me_method = X264_ME_TESA;//运动估算发的选择
        param->analyse.i_subpel_refine = 11;
        param->analyse.i_me_range = 24;//运动估计范围设为24
        param->i_frame_reference = 16;//参考帧的最大帧数设为16
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.b_fast_pskip = 0;
        param->analyse.i_trellis = 2;
        param->i_bframe = 16;//参考帧之间b帧为16
        param->rc.i_lookahead = 60;
    }
    else
    {
        x264_log( NULL, X264_LOG_ERROR, "invalid preset '%s'\n", preset );
        return -1;
    }
    return 0;
} 


void compress_begin(Encoder *en, int width, int height) {
      en->param = (x264_param_t *) malloc(sizeof(x264_param_t));
       if(en->param==NULL)
	{
           printf("param=null\n");
           exit(1);
	}
	en->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));
       if(en->picture==NULL)
	{
           printf("pic=null\n");
           exit(1);
	}
	x264_param_default(en->param); //set default param 
	
	 //ultrafast cpu占用最小，zerolatency 不缓存帧  
	en->param->i_width = width;  
	en->param->i_height = height;  
	en->param->b_repeat_headers = 1; //每个关键帧前都发送sps和pps  
	en->param->b_cabac = 1; //自适应上下文算术编码，baseline 不支持      
	en->param->i_fps_den=1;//帧率分母  
	en->param->i_fps_num=30;//帧率分子   
	en->param->cpu = x264_cpu_detect();
    en->param->i_threads = X264_THREADS_AUTO;  
	
	en->param->rc.b_mb_tree=0;//不为0导致编码延时帧，实时编码是强烈建议为0    
	en->param->rc.i_rc_method=X264_RC_CQP; //码率控制参数 CQP 恒定质量 CRF 恒定码率 ABR 平均码率  
	  
	//x264_param_apply_profile(&en->param,"heigh");  
	en->param->i_level_idc = 30;  
	en->param->i_log_level = X264_LOG_NONE;  
	//en->param->pf_log = x264_pf_log;   
	en->param->i_csp = X264_CSP_I422; // 不支持baseline  
	
	x264_param_apply_preset(en->param, "ultrafast");
	
	
	x264_param_apply_profile(en->param, x264_profile_names[0]);  
	 
	if ((en->handle = x264_encoder_open(en->param)) == 0) {
		printf("x264_encoder_open failed");
		return;
	} 
	/* Create a new pic */
	x264_picture_alloc(en->picture, X264_CSP_I422, en->param->i_width,
			en->param->i_height);
	en->picture->img.i_csp = X264_CSP_I422;
	en->picture->img.i_plane = 3;
	
}

int compress_frame(Encoder *en, int type, uint8_t *in, uint8_t *out) {
	x264_picture_t pic_out;
	int nNal = -1;
	int result = 0;
	int i = 0;
	uint8_t *p_out = out;

	char *y = en->picture->img.plane[0];
	char *u = en->picture->img.plane[2];
	char *v = en->picture->img.plane[1];

	int is_y = 1, is_u = 1;
	int y_index = 0, u_index = 0, v_index = 0;

	int yuv422_length = 2 * en->param->i_width * en->param->i_height;

	//序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
	for (i = 0; i < yuv422_length; ++i) {
		if (is_y) {
			*(y + y_index) = *(in + i);
			++y_index;
			//printf("Y");
			is_y = 0;
		} else {
			if (is_u) {
				*(u + u_index) = *(in + i);
				++u_index;
				//printf("U");
				is_u = 0;
			} else {
				*(v + v_index) = *(in + i);  
				//printf("%d",*(v + v_index));
				++v_index;
				is_u = 1;
			}
			is_y = 1;
		}
	} 

	switch (type) {
	case 0:
		en->picture->i_type = X264_TYPE_P;
		break;
	case 1:
		en->picture->i_type = X264_TYPE_IDR;
		break;
	case 2:
		en->picture->i_type = X264_TYPE_I;
		break;
	default:
		en->picture->i_type = X264_TYPE_AUTO;
		break;
	} 
	if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
			&pic_out) < 0) {
		return -1;
	}

	for (i = 0; i < nNal; i++) {
		memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
		p_out += en->nal[i].i_payload;
		result += en->nal[i].i_payload;
	}

	return result;
}

void compress_end(Encoder *en) {
	if (en->picture) {
		x264_picture_clean(en->picture);
		free(en->picture);
		en->picture = 0;
	}
	if (en->param) {
		free(en->param);
		en->param = 0;
	}
	if (en->handle) {
		x264_encoder_close(en->handle);
	}
	free(en);
}
