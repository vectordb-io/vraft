#include  <functional>
#include <glog/logging.h>
#include "env.h"
#include "grpc_server.h"
#include "thread_pool.h"

namespace vraft {

GrpcServer::GrpcServer()
    :running_(false),
     cq_out_(std::make_unique<grpc::CompletionQueue>()) {
}

GrpcServer::~GrpcServer() {
    server_->Shutdown();
    cq_in_->Shutdown();
    cq_out_->Shutdown();
}

void
GrpcServer::IntendOnPing() {
    // optimizing by memory pool
    auto p = new AsyncTaskOnPing(&service_, cq_in_.get());
    assert(on_ping_cb_);
    p->cb_ = on_ping_cb_;
    service_.RequestRpcPing(&(p->ctx_), &(p->request_), &(p->responder_), p->cq_in_, p->cq_in_, p);
}

void
GrpcServer::OnPing(AsyncTaskOnPing *p) {
    p->cb_(p->request_, p->reply_);
    p->done_ = true;
    p->responder_.Finish(p->reply_, grpc::Status::OK, p);
}

Status
GrpcServer::AsyncPing(const vraft_rpc::Ping &request, const std::string &address, PingFinishCallBack cb) {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    std::unique_ptr<vraft_rpc::VRaft::Stub> stub = vraft_rpc::VRaft::NewStub(channel);
    // optimizing by memory pool
    auto p = new AsyncTaskPing();
    p->cb_ = cb;

    p->response_reader_ = stub->PrepareAsyncRpcPing(&(p->ctx_), request, cq_out_.get());
    p->response_reader_->StartCall();
    p->response_reader_->Finish(&p->reply_, &p->status_, (void*)p);

    return Status::OK();
}

void
GrpcServer::IntendOnRequestVote() {
    // optimizing by memory pool
    auto p = new AsyncTaskOnRequestVote(&service_, cq_in_.get());
    assert(on_request_vote_cb_);
    p->cb_ = on_request_vote_cb_;
    service_.RequestRpcRequestVote(&(p->ctx_), &(p->request_), &(p->responder_), p->cq_in_, p->cq_in_, p);
}

void
GrpcServer::OnRequestVote(AsyncTaskOnRequestVote *p) {
    p->cb_(p->request_, p->reply_);
    p->done_ = true;
    p->responder_.Finish(p->reply_, grpc::Status::OK, p);
}

void
GrpcServer::IntendOnAppendEntries() {
    // optimizing by memory pool
    auto p = new AsyncTaskOnAppendEntries(&service_, cq_in_.get());
    assert(on_append_entries_cb_);
    p->cb_ = on_append_entries_cb_;
    service_.RequestRpcAppendEntries(&(p->ctx_), &(p->request_), &(p->responder_), p->cq_in_, p->cq_in_, p);
}

void
GrpcServer::OnAppendEntries(AsyncTaskOnAppendEntries *p) {
    p->cb_(p->request_, p->reply_);
    p->done_ = true;
    p->responder_.Finish(p->reply_, grpc::Status::OK, p);
}

Status
GrpcServer::AsyncRequestVote(const vraft_rpc::RequestVote &request, const std::string &address, RequestVoteFinishCallBack cb) {


    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    std::unique_ptr<vraft_rpc::VRaft::Stub> stub = vraft_rpc::VRaft::NewStub(channel);


    // optimizing by memory pool
    auto p = new AsyncTaskRequestVote();
    p->cb_ = cb;


    p->response_reader_ = stub->PrepareAsyncRpcRequestVote(&(p->ctx_), request, cq_out_.get());


    p->response_reader_->StartCall();


    p->response_reader_->Finish(&p->reply_, &p->status_, (void*)p);


    return Status::OK();
}

Status
GrpcServer::AsyncAppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address, AppendEntriesFinishCallBack cb) {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    std::unique_ptr<vraft_rpc::VRaft::Stub> stub = vraft_rpc::VRaft::NewStub(channel);
    // optimizing by memory pool
    auto p = new AsyncTaskAppendEntries();
    p->cb_ = cb;

    p->response_reader_ = stub->PrepareAsyncRpcAppendEntries(&(p->ctx_), request, cq_out_.get());
    p->response_reader_->StartCall();
    p->response_reader_->Finish(&p->reply_, &p->status_, (void*)p);

    return Status::OK();
}

Status
GrpcServer::StartService() {
    std::string server_address(Config::GetInstance().me().ToString());
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_in_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    running_ = true;

    LOG(INFO) << "grpc server message_in_thread start ...";
    thread_called_ = std::make_unique<std::thread>(&GrpcServer::ThreadAsyncCalled, this);

    LOG(INFO) << "grpc server message_out_thread start ...";
    thread_call_ = std::make_unique<std::thread>(&GrpcServer::ThreadAsyncCall, this);

    IntendOnPing();
    IntendOnRequestVote();
    IntendOnAppendEntries();

    return Status::OK();
}

Status
GrpcServer::Start() {
    LOG(INFO) << "grpc server start ...";

    LOG(INFO) << "grpc server boot_thread start ...";
    boot_thread_ = std::make_unique<std::thread>(&GrpcServer::StartService, this);

    return Status::OK();
}

Status
GrpcServer::Stop() {
    LOG(INFO) << "grpc server stop ...";
    running_ = false;

    boot_thread_->join();
    LOG(INFO) << "grpc server boot_thread stop ...";

    thread_called_->join();
    LOG(INFO) << "grpc server message_in_thread stop ...";

    thread_call_->join();
    LOG(INFO) << "grpc server message_out_thread stop ...";

    return Status::OK();
}

void
GrpcServer::ThreadAsyncCalled() {
    void* tag;
    bool ok;
    while (running_) {
        cq_in_->Next(&tag, &ok);
        if (ok) {
            AsyncTaskCalled *p = static_cast<AsyncTaskCalled*>(tag);
            p->Process();
        } else {
            LOG(INFO) << "ThreadAsyncCalled error";
        }
    }
}

void
GrpcServer::ThreadAsyncCall() {
    void* tag;
    bool ok = false;
    while (cq_out_->Next(&tag, &ok) && running_) {
        AsyncTaskCall *p = static_cast<AsyncTaskCall*>(tag);
        if (!ok) {
            LOG(INFO) << "ThreadAsyncCall error";
            assert(0);
        }

        if (p->GetStatus().ok()) {
            //p->Process();
            Env::GetInstance().thread_pool()->ProduceOne(std::bind(&AsyncTaskCall::Process, p));
        } else {
            LOG(ERROR) << "err:" << p->GetStatus().error_message();
            delete p;
            //printf("delete %p \n", p);
            //fflush(nullptr);
        }
    }
}

} // namespace vraft
