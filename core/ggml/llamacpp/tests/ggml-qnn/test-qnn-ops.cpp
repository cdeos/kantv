/*
 * Copyright (c) 2024- KanTV Authors
 *
 * this is implementation of Android command line application for verify GGML QNN backend
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <tuple>
#include <queue>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <memory>
#include <regex>
#include <random>
#include <functional>
#include <unordered_map>
#include <condition_variable>
#include <cassert>
#include <unordered_set>
#include <utility>

#include "ggml.h"
#include "ggml-alloc.h"
#include "ggml-backend.h"
#include "ggml-qnn.h"

#define GGML_QNN_LOGBUF_LEN                             4096
#define GGML_QNN_DEBUG                                  1

#define QNN_LOG_ERROR(...) ggml_qnn_log_internal(GGML_LOG_LEVEL_DEBUG,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define QNN_LOG_WARN(...)  ggml_qnn_log_internal(GGML_LOG_LEVEL_DEBUG , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define QNN_LOG_INFO(...)  ggml_qnn_log_internal(GGML_LOG_LEVEL_DEBUG , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#if GGML_QNN_DEBUG
#define QNN_LOG_DEBUG(...) ggml_qnn_log_internal(GGML_LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define QNN_LOG_DEBUG(...)
#endif

static void ggml_qnn_log_internal(ggml_log_level level, const char * file, const char * func, int line, const char * format, ...) {
    static std::mutex ggml_qnn_log_internal_mutex;
    static char s_ggml_qnn_log_internal_buf[GGML_QNN_LOGBUF_LEN];

    {
        std::lock_guard<std::mutex> lock(ggml_qnn_log_internal_mutex);
        va_list args;
        va_start(args, format);
        int len_prefix = snprintf(s_ggml_qnn_log_internal_buf, GGML_QNN_LOGBUF_LEN, "[%s, %d]: ", func, line);
        int len = vsnprintf(s_ggml_qnn_log_internal_buf + len_prefix, GGML_QNN_LOGBUF_LEN - len_prefix, format, args);
        if (len < (GGML_QNN_LOGBUF_LEN - len_prefix)) {
            //for Android command line application or WoA
            printf("%s\n", s_ggml_qnn_log_internal_buf);
        }
        va_end(args);
    }
}


static const char *get_qnn_backend_name(int n_backend_type) {
    switch (n_backend_type) {
        case 0:
            return "QNN-CPU";
        case 1:
            return "QNN-GPU";
        case 2:
            return "QNN-NPU(HTP/DSP)";
        case 3:
            return "ggml";
        default:
            return "unknown";
    }
}


static bool ggml_graph_compute_helper(
        struct ggml_backend *backend,
        struct ggml_cgraph *graph,
        std::vector<uint8_t> &buf,
        int n_threads,
        ggml_abort_callback abort_callback,
        void *abort_callback_data) {
    struct ggml_cplan plan = ggml_graph_plan(graph, n_threads);

    plan.abort_callback = abort_callback;
    plan.abort_callback_data = abort_callback_data;

    if (plan.work_size > 0) {
        buf.resize(plan.work_size);
        plan.work_data = buf.data();
    }

    if (ggml_backend_is_cpu(backend)) {
        ggml_backend_cpu_set_n_threads(backend, n_threads);
    }

#ifdef GGML_USE_QNN
    if (ggml_backend_is_qnn(backend)) {
        ggml_backend_qnn_set_n_threads(backend, n_threads);
    }
#endif

    //a new approch of mixed inference
    if (nullptr != backend)
        return ggml_backend_graph_compute(backend, graph) == GGML_STATUS_SUCCESS;
    else
        return ggml_graph_compute(graph, &plan);
}


static float tensor_sum_elements(const ggml_tensor *tensor) {
    double sum = 0;
    float value = 0;
    std::ostringstream tmposs;
    if (tensor->type == GGML_TYPE_F32) {
        for (int h = 0; h < tensor->ne[3]; h++) {
            for (int i = 0; i < tensor->ne[2]; i++) {
                for (int j = 0; j < tensor->ne[1]; j++) {
                    for (int k = 0; k < tensor->ne[0]; k++) {
                        value = ((float *) tensor->data)[h * tensor->ne[2] + i * tensor->ne[1] +
                                                         j * tensor->ne[0] + k];
                        sum += value;
                        tmposs << std::setw(8) << std::fixed << std::setprecision(2) << value
                               << " ";
                    }
                    if (strlen(tmposs.str().c_str()) > 4000) {

                    } else {
                        QNN_LOG_DEBUG("%s", tmposs.str().c_str());
                    }
                    tmposs.clear();
                    tmposs.str("");
                    QNN_LOG_DEBUG("\n");
                }
            }
        }
    }

    QNN_LOG_DEBUG("\n");
    return sum;
}


static void tensor_dump(const ggml_tensor *tensor, const char *name) {
    QNN_LOG_DEBUG("dump ggml tensor %s(%s)", name, tensor->name);
    QNN_LOG_DEBUG("%15s: type = %i (%5s) ne = %5" PRIi64 " x %5" PRIi64 " x %5" PRIi64 ", nb = (%5zi, %5zi, %5zi)",
          name,
          tensor->type, ggml_type_name(tensor->type),
          tensor->ne[0], tensor->ne[1], tensor->ne[2], tensor->nb[0], tensor->nb[1], tensor->nb[2]);
    float sum = tensor_sum_elements(tensor);

    QNN_LOG_DEBUG("\n");
}


#define TENSOR_DUMP(tensor) tensor_dump(tensor, #tensor)
static uint32_t get_tensor_rank(const ggml_tensor * tensor) {
    uint32_t rank = 0;
    for (int i = 0; i < GGML_MAX_DIMS; i++) {
        if ((0 != tensor->ne[i]) && (1 != tensor->ne[i])) {
            rank++;
        }
    }
    return rank;
}


static uint32_t get_tensor_data_size(const ggml_tensor *tensor) {
#if 0
    size_t data_size = ggml_row_size(tensor->type, tensor->ne[0]);
    size_t n_dims = get_tensor_rank(tensor);
    for (int i = 1; i < n_dims; i++) {
        data_size *= tensor->ne[i];
    }

    QNN_LOG_DEBUG("get_tensor_data_size %d", data_size);
    QNN_LOG_DEBUG("ggml_nbytes(tensor) %d", ggml_nbytes(tensor));
#endif
    return ggml_nbytes(tensor);
}


//ref: test-qnn-ops.cpp in upstream project llamacpp
static void init_tensor_uniform(ggml_tensor * tensor, float min = -1.0f, float max = 1.0f) {
    // static RNG initialization (revisit if n_threads stops being constant)
    static const size_t n_threads = std::thread::hardware_concurrency();
    static std::vector<std::default_random_engine> generators = []() {
        std::random_device rd;
        std::vector<std::default_random_engine> vec;
        vec.reserve(n_threads);
        //for (size_t i = 0; i < n_threads; i++) { vec.emplace_back(1234 + i); } // fixed seed
        for (size_t i = 0; i < n_threads; i++) { vec.emplace_back(rd()); }
        return vec;
    }();

    size_t size = ggml_nelements(tensor);
    std::vector<float> data(size);

    auto init_thread = [&](size_t ith, size_t start, size_t end) {
        std::uniform_real_distribution<float> distribution(min, max);
        for (size_t i = start; i < end; i++) {
            data[i] = distribution(generators[ith]);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    for (size_t i = 0; i < n_threads; i++) {
        size_t start =     i*size/n_threads;
        size_t end   = (i+1)*size/n_threads;
        threads.emplace_back(init_thread, i, start, end);
    }
    for (auto & t : threads) {
        t.join();
    }
    if (tensor->type == GGML_TYPE_F32 || tensor->type == GGML_TYPE_I32) {
        ggml_backend_tensor_set(tensor, data.data(), 0, size * sizeof(float));
    } else if (ggml_is_quantized(tensor->type) || tensor->type == GGML_TYPE_F16 || tensor->type == GGML_TYPE_BF16) {
        GGML_ASSERT(size % ggml_blck_size(tensor->type) == 0);
        std::vector<uint8_t> dataq(ggml_row_size(tensor->type, size));
        std::vector<float> imatrix(tensor->ne[0], 1.0f); // dummy importance matrix
        const float * im = imatrix.data();
        if (!ggml_quantize_requires_imatrix(tensor->type)) {
            // when the imatrix is optional, we want to test both quantization with and without imatrix
            // use one of the random numbers to decide
            if (data[0] > 0.5f*(min + max)) {
                im = nullptr;
            }
        }
        ggml_quantize_chunk(tensor->type, data.data(), dataq.data(), 0, size/tensor->ne[0], tensor->ne[0], im);
        GGML_ASSERT(ggml_validate_row_data(tensor->type, dataq.data(), dataq.size()));
        ggml_backend_tensor_set(tensor, dataq.data(), 0, dataq.size());
    } else if (tensor->type == GGML_TYPE_I8 || tensor->type == GGML_TYPE_I16 || tensor->type == GGML_TYPE_I32) {
        // This is going to create some weird integers though.
        ggml_backend_tensor_set(tensor, data.data(), 0, ggml_nbytes(tensor));
    } else {
        GGML_ASSERT(false);
    }
}


static void initialize_tensors(ggml_context * ctx) {
    for (ggml_tensor * t = ggml_get_first_tensor(ctx); t != nullptr; t = ggml_get_next_tensor(ctx, t)) {
        init_tensor_uniform(t);
    }
}


static void show_usage() {
    printf(" " \
        "\nUsage: test_qnn_ops [options]\n" \
        "\n" \
        "Options:\n" \
        " -t GGML_OP_ADD / GGML_OP_MUL / GGML_OP_MULMAT\n" \
        " ?/h print usage infomation\n\n"
    );
}


int main(int argc, char *argv[])
{
    int result = 0;
    int64_t n_begin_time = 0LL;
    int64_t n_end_time = 0LL;
    int64_t n_duration = 0LL;
    size_t ctx_size = 0;
    struct ggml_context *ctx = nullptr;
    struct ggml_cgraph *gf = nullptr;
    struct ggml_tensor *src0 = nullptr;
    struct ggml_tensor *src1 = nullptr;
    struct ggml_tensor *dst = nullptr;
    std::vector<uint8_t> work_buffer;

    int num_threads = 4;
    int n_backend_type = QNN_BACKEND_CPU;
    int n_ggml_op_type = GGML_OP_ADD;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                if (0 == memcmp(argv[i+1], "GGML_OP_ADD", 11)) {
                    n_ggml_op_type = GGML_OP_ADD;
                } else if (0 == memcmp(argv[i+1], "GGML_OP_MUL_MAT", 15)) {
                    n_ggml_op_type = GGML_OP_MUL_MAT;
                } else if (0 == memcmp(argv[i+1], "GGML_OP_MUL", 11)) {
                    n_ggml_op_type = GGML_OP_MUL;
                }
                break;
            } else {
                show_usage();
                return 1;
            }
        } else {
            show_usage();
            return 1;
        }
    }

    QNN_LOG_DEBUG("enter qnn_ggml_op\n");
    QNN_LOG_DEBUG("ggml op:%d(%s)", n_ggml_op_type, ggml_op_name((enum ggml_op) n_ggml_op_type));

    int sizey = 4;
    int sizex = 4;
    int sizez = 1;

    const ggml_type qtype = GGML_TYPE_F32;
    //const ggml_type qtype = GGML_TYPE_Q8_0;

    n_begin_time = ggml_time_us();
    srand(time(NULL));

    ctx_size += 1024 * 1024 * 32;
    QNN_LOG_DEBUG("Allocating Memory of size %zi bytes, %zi MB\n", ctx_size,
                    (ctx_size / 1024 / 1024));

    struct ggml_init_params params = {
            /*.mem_size   =*/ ctx_size,
            /*.mem_buffer =*/ NULL,
            /* no_alloc   =*/ 0
    };

    ggml_backend_t backend = nullptr;
    ggml_backend_buffer_t buffer = nullptr;
    if (n_backend_type != QNN_BACKEND_GGML) {//QNN_BACKEND_GGML is fake QNN backend "ggml", just used to compare performance between QNN backend and original GGML
        params.no_alloc = true;
        backend = ggml_backend_qnn_init(n_backend_type, "/data/local/tmp/");
        if (nullptr == backend) {
            QNN_LOG_DEBUG("create qnn backend %d(%s) failed", n_backend_type, get_qnn_backend_name(n_backend_type));
            return 1;
        }
        //num_threads = 1;
    }

    ctx = ggml_init(params);
    if (!ctx) {
        QNN_LOG_WARN("%s: ggml_init() failed\n");
        return 1;
    }

    QNN_LOG_DEBUG("creating new tensors\n");
    QNN_LOG_DEBUG("ggml_blck_size(%s) %d", ggml_type_name(qtype), ggml_blck_size(qtype));
    QNN_LOG_DEBUG("ggml_type_size(%s) %d", ggml_type_name(qtype), ggml_type_size(qtype));
    if (qtype != GGML_TYPE_F32) {
        sizex = ggml_blck_size(qtype);
    }

    switch (n_ggml_op_type) {
        case GGML_OP_ADD:
            src0 = ggml_new_tensor_2d(ctx, qtype, sizex, sizey);
            ggml_set_input(src0);
            src1 = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, sizex, sizey);
            ggml_set_input(src1);
            dst = ggml_add(ctx, src0, src1);
            break;
        case GGML_OP_MUL:
            src0 = ggml_new_tensor_2d(ctx, qtype, sizex, sizey);
            ggml_set_input(src0);
            src1 = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, sizex, sizey);
            ggml_set_input(src1);
            dst = ggml_mul(ctx, src0, src1);
            break;
        case GGML_OP_MUL_MAT:
            src0 = ggml_new_tensor_2d(ctx, qtype, sizex, sizey);
            ggml_set_input(src0);
            src1 = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, sizex, sizey);
            ggml_set_input(src1);
            dst = ggml_mul_mat(ctx, src0, src1);
            break;
        default:
            QNN_LOG_DEBUG("ggml op %d(%s) not supported", n_ggml_op_type,
                  ggml_op_name((enum ggml_op) n_ggml_op_type));
            QNN_LOG_DEBUG("leave qnn_ggml_op UT(unit test)\n");
            ggml_free(ctx);
            ggml_backend_free(backend);
            return 2;
            //break;
    }
    ggml_set_output(dst);
#ifdef GGML_USE_QNN
    if (n_backend_type != QNN_BACKEND_GGML) {//QNN_BACKEND_GGML is fake QNN backend "ggml", just used to compare performance between QNN backend and original GGML
        QNN_LOG_DEBUG("creating backend buffer\n");
        buffer = ggml_backend_alloc_ctx_tensors(ctx, backend);
        if (!buffer) {
            QNN_LOG_DEBUG("%s: failed to allocate backend buffer\n", __func__);
            ggml_free(ctx);
            ggml_backend_free(backend);
            return false;
        }
    }
#endif

    QNN_LOG_DEBUG("creating compute graph\n");
    gf = ggml_new_graph(ctx);
    ggml_build_forward_expand(gf, dst);

#if 0//TEST_F32
    ggml_set_f32(src0, (rand() % 100 + 1));
    ggml_set_f32(src1, (rand() % 100 + 1));
    ggml_set_f32(dst, 0.0f);
#else
    if (n_backend_type != QNN_BACKEND_GGML) {
        initialize_tensors(ctx);
    }
#endif

    ggml_graph_compute_helper(backend, gf, work_buffer, num_threads, nullptr, nullptr);
    if (get_tensor_data_size(dst) < (32 * 32)) {
        QNN_LOG_DEBUG("dump:\n");
        TENSOR_DUMP(src0);
        TENSOR_DUMP(src1);
        TENSOR_DUMP(dst);
    } else {
        QNN_LOG_DEBUG("%15s: type = %i (%5s) ne = %5" PRIi64 " x %5" PRIi64 " x %5" PRIi64 ", nb = (%5zi, %5zi, %5zi)\n",
              src0->name,
              src0->type, ggml_type_name(src0->type), src0->ne[0], src0->ne[1], src0->ne[2],
              src0->nb[0], src0->nb[1], src0->nb[2]);
        QNN_LOG_DEBUG("%15s: type = %i (%5s) ne = %5" PRIi64 " x %5" PRIi64 " x %5" PRIi64 ", nb = (%5zi, %5zi, %5zi)\n",
              src1->name,
              src1->type, ggml_type_name(src1->type), src1->ne[0], src1->ne[1], src1->ne[2],
              src1->nb[0], src1->nb[1], src1->nb[2]);
        QNN_LOG_DEBUG("%15s: type = %i (%5s) ne = %5" PRIi64 " x %5" PRIi64 " x %5" PRIi64 ", nb = (%5zi, %5zi, %5zi)\n",
              dst->name,
              dst->type, ggml_type_name(dst->type), dst->ne[0], dst->ne[1], dst->ne[2], dst->nb[0],
              dst->nb[1], dst->nb[2]);
    }

    ggml_free(ctx);
    ggml_backend_buffer_free(buffer);
    ggml_backend_free(backend);

    n_end_time = ggml_time_us();
    n_duration = (n_end_time - n_begin_time) / 1000;
    QNN_LOG_DEBUG("duration of ut %s : %lld milliseconds\n", ggml_op_name((enum ggml_op)n_ggml_op_type), n_duration);

    return 0;
}
