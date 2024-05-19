/*
 * Copyright (c) 2024- KanTV Authors
 *
 * JNI implementation of GGML for Project KanTV
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KANTV_GGML_JNI_H
#define KANTV_GGML_JNI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "libavutil/cde_log.h"
#include "ggml.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JNI_BUF_LEN                 4096
#define JNI_TMP_LEN                 256


#define BECHMARK_MEMCPY             0       //memcpy  benchmark
#define BECHMARK_MULMAT             1       //mulmat  benchmark
#define BENCHMARK_QNN_GGML_OP       2       //UT for PoC-S49: implementation of GGML OPs using QNN API
#define BENCHMARK_QNN_AUTO_UT       3       //automation UT for PoC-S49: implementation of GGML OPs using QNN API
#define BECHMARK_ASR                4       //ASR(whisper.cpp) benchmark
#define BENCHMARK_LLM               5       //LLM(llama.cpp) benchmark
#define BENCHMARK_TEXT2IMAGE        6       //TEXT2IMAGE(stablediffusion.cpp) benchmark
#define BENCHMARK_CV_MNIST          7       //mnist
#define BENCHMARK_TTS               8       //TTS(bark.cpp) benchmark
#define BENCHMAKR_MAX               8

#define BACKEND_CPU                 0
#define BACKEND_GPU                 1
#define BACKEND_DSP                 2
#define BACKEND_GGML                3       //"fake" QNN backend just for compare performance between QNN and original GGML
#define BACKEND_MAX                 3


#define GGML_JNI_NOTIFY(...)        ggml_jni_notify_c_impl(__VA_ARGS__)

// JNI helper function for whisper.cpp benchmark
    void         ggml_jni_notify_c_impl(const char * format, ...);
    int          whisper_get_cpu_core_counts(void);
    void         whisper_set_benchmark_status(int b_exit_benchmark);
    /**
    *
    * @param sz_model_path   /sdcard/kantv/file_name_of_gguf_model or qualcomm's prebuilt dedicated model.so or ""
    * @param sz_user_data    ASR: /sdcard/kantv/jfk.wav / LLM: user input / TEXT2IMAGE: user input / MNIST: image path / TTS: user input
    * @param n_bench_type    0: memcpy 1: mulmat 2: QNN GGML OP(QNN UT) 3: QNN UT automation 4: ASR(whisper.cpp) 5: LLM(llama.cpp) 6: TEXT2IMAGE(stablediffusion.cpp) 7:MNIST 8: TTS
    * @param n_threads       1 - 8
    * @param n_backend_type  0: CPU  1: GPU  2: NPU 3: ggml("fake" QNN backend, just for compare performance)
    * @param n_op_type       type of GGML OP
    * @return
    */
    void         ggml_jni_bench(const char * sz_model_path, const char * sz_user_data, int n_bench_type, int num_threads, int n_backend_type, int n_op_type);


    const char * whisper_get_ggml_type_str(enum ggml_type wtype);


    // JNI helper function for ASR(whisper.cpp)
    /**
    * @param sz_model_path
    * @param n_threads
    * @param n_asrmode            0: normal transcription  1: asr pressure test 2:benchmark 3: transcription + audio record
    * @param n_backend            0: QNN CPU 1: QNN GPU 2: QNN HTP(DSP) 3:ggml
    */
    int          whisper_asr_init(const char *sz_model_path, int n_threads, int n_asrmode, int n_backend);
    void         whisper_asr_finalize(void);

    void         whisper_asr_start(void);
    void         whisper_asr_stop(void);
    /**
    * @param sz_model_path
    * @param n_threads
    * @param n_asrmode            0: normal transcription  1: asr pressure test 2:benchmark 3: transcription + audio record
    * @param n_backend            0: QNN CPU 1: QNN GPU 2: QNN HTP(DSP) 3:ggml
    */
    int          whisper_asr_reset(const char * sz_model_path, int n_threads, int n_asrmode, int n_backend);





// =================================================================================================
// trying to integrate llama.cpp from 03/26/2024 to 03/28/2024
// =================================================================================================
    /**
    *
    * @param sz_model_path         /sdcard/kantv/xxxxxx.gguf
    * @param prompt
    * @param bench_type            not used currently
    * @param n_threads             1 - 8
    * @param n_backend            0: QNN CPU 1: QNN GPU 2: QNN HTP(DSP) 3:ggml
    * @return
    */
    int          llama_inference(const char * model_path, const char * prompt, int bench_type, int num_threads, int n_backend);


// =================================================================================================
// PoC#121:Add Qualcomm mobile SoC native backend for GGML from 03-29-2024
// =================================================================================================
    /**
     * this special function is for PoC-S49: implementation of other GGML OP(non-mulmat) using QNN API, https://github.com/zhouwg/kantv/issues/121
     * it's similar to qnn_ggml but different with qnn_ggml, because data path in these two function is totally different
     *
     * this function will calling GGML QNN backend directly
     *
     * this function used to validate PoC-S49:implementation of other GGML OP(non-mulmat) using QNN API
     * or this function is UT for PoC-S49:implementation of other GGML OP(non-mulmat) using QNN API
     *
     * @param model_path whisper.cpp model at the first step, llama.cpp model at the second step
     * @param num_threads 1 - 8
     * @param n_backend_type 0: QNN CPU, 1: QNN GPU, 2: QNN DSP(HTA), 3: ggml(fake QNN backend, just used to compare performance between QNN and original GGML)
     * @param n_op_type GGML OP type
     * @return
     */
    int qnn_ggml_op(const char * model_path, int num_threads, int n_backend_type, int n_ggml_op_type);

    /**
     * similar to qnn_ggml_op, but an automation UT for a specify GGML OP with a specify backend
     */
    int qnn_ggml_op_automation_ut(const char * model_path, int num_threads, int n_backend_type, int n_ggml_op_type);


// =================================================================================================
// trying to integrate stablediffusion.cpp on 04-06-2024(Apri,6,2024)
// =================================================================================================
/**
*
* @param sz_model_path         /sdcard/kantv/xxxxxx.gguf
* @param prompt
* @param bench_type            not used currently
* @param n_threads             1 - 8
* @param n_backend_type 0: QNN CPU, 1: QNN GPU, 2: QNN DSP(HTA), 3: ggml(fake QNN backend, just used to compare performance)
* @return
*/
int  stablediffusion_inference(const char * model_path, const char * prompt, int bench_type, int num_threads, int n_backend_type);


// =================================================================================================
// MNIST inference using ggml
// =================================================================================================
int  mnist_inference(const char * sz_model_path, const char * sz_image_path, int bench_type, int num_threads, int n_backend_type);


// =================================================================================================
// TTS inference using ggml
// =================================================================================================
int  tts_inference(const char * sz_model_path, const char * prompt, int bench_type, int num_threads, int n_backend_type);

#ifdef __cplusplus
}
#endif


#endif
