#include "common.h"
#include "bpf_helpers.h"

char __license[] SEC("license") = "Dual MIT/GPL";

#define MAX_SIZE 100

struct http_request_t {
    char method[MAX_SIZE];
    char path[MAX_SIZE];
};

struct {
	__uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
} events SEC(".maps");

// This instrumentation attaches uprobe to the following function:
// func (mux *ServeMux) ServeHTTP(w ResponseWriter, r *Request)
SEC("uprobe/ServerMux_ServeHTTP")
int uprobe_ServerMux_ServeHTTP(struct pt_regs *ctx) {
    // positions
    u64 servermux_pos = 1; // not relevant
    u64 resWriter_pos = 2; // +1 for interface, not relevant
    u64 request_pos = 4;

    // Positions inside request struct
    u64 method_ptr_pos = 0;
    u64 method_len_pos = 1;
    u64 uri_ptr_pos = 2;

    // Positions inside URI struct
    u64 path_ptr_pos = 7;
    u64 path_len_pos = 8;
    struct http_request_t httpReq = {};

    // Get request struct
    void* req_ptr = 0;
    bpf_probe_read(&req_ptr, sizeof(req_ptr), (void *)(ctx->rsp+(request_pos*8)));

    // Get method from request
    void* method_ptr = 0;
    bpf_probe_read(&method_ptr, sizeof(method_ptr), (void *)(req_ptr+(method_ptr_pos*8)));
    u64 method_len = 0;
    bpf_probe_read(&method_len, sizeof(method_len), (void *)(req_ptr+(method_len_pos*8)));
    u64 method_size = sizeof(httpReq.method);
    method_size = method_size < method_len ? method_size : method_len;
    bpf_probe_read(&httpReq.method, method_size, method_ptr);

    // get path from Request.URI
    void *uri_ptr = 0;
    bpf_probe_read(&uri_ptr, sizeof(uri_ptr), (void *)(req_ptr+(uri_ptr_pos*8)));
    void* path_ptr = 0;
    bpf_probe_read(&path_ptr, sizeof(path_ptr), (void *)(uri_ptr+(path_ptr_pos*8)));
    u64 path_len = 0;
    bpf_probe_read(&path_len, sizeof(path_len), (void *)(uri_ptr+(path_len_pos*8)));
    u64 path_size = sizeof(httpReq.path);
    path_size = path_size < path_len ? path_size : path_len;
    bpf_probe_read(&httpReq.path, path_size, path_ptr);

    // Write event
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &httpReq, sizeof(httpReq));
    return 0;
}

SEC("uprobe/ServerMux_ServeHTTP")
int uprobe_ServerMux_ServeHTTP_ByRegisters(struct pt_regs *ctx) {
    // Positions inside request struct
    u64 method_ptr_pos = 0;
    u64 method_len_pos = 1;
    u64 uri_ptr_pos = 2;

    // Positions inside URI struct
    u64 path_ptr_pos = 7;
    u64 path_len_pos = 8;
    struct http_request_t httpReq = {};

    // Get request struct
    void* req_ptr = (void *)(ctx->rdi);

    // Get method from request
    void* method_ptr = 0;
    bpf_probe_read(&method_ptr, sizeof(method_ptr), (void *)(req_ptr+(method_ptr_pos*8)));
    u64 method_len = 0;
    bpf_probe_read(&method_len, sizeof(method_len), (void *)(req_ptr+(method_len_pos*8)));
    u64 method_size = sizeof(httpReq.method);
    method_size = method_size < method_len ? method_size : method_len;
    bpf_probe_read(&httpReq.method, method_size, method_ptr);

    // get path from Request.URI
    void *uri_ptr = 0;
    bpf_probe_read(&uri_ptr, sizeof(uri_ptr), (void *)(req_ptr+(uri_ptr_pos*8)));
    void* path_ptr = 0;
    bpf_probe_read(&path_ptr, sizeof(path_ptr), (void *)(uri_ptr+(path_ptr_pos*8)));
    u64 path_len = 0;
    bpf_probe_read(&path_len, sizeof(path_len), (void *)(uri_ptr+(path_len_pos*8)));
    u64 path_size = sizeof(httpReq.path);
    path_size = path_size < path_len ? path_size : path_len;
    bpf_probe_read(&httpReq.path, path_size, path_ptr);

    // Write event
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &httpReq, sizeof(httpReq));
    return 0;
}