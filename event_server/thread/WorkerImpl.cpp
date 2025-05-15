//
// Created by bont on 24. 3. 5.
//

#include "WorkerImpl.h"

#include <utility>

WorkerImpl::WorkerImpl(const std::function<void()>& cb, std::string name)
        : dispatcherPtr_(std::make_unique<Event::Dispatcher>()), cb_(cb), name_(std::move(name)) {

}

WorkerImpl::~WorkerImpl() {

}

void WorkerImpl::threadRoutine(const std::function<void()> &cb) {
    LOG(INFO) <<"worker entering dispatch loop";
    dispatcherPtr_->post([this, cb]() {
        cb();
    });
    dispatcherPtr_->run(Event::RunType::Block);
    LOG(INFO) <<"worker exited dispatch loop";

    //dispatcher_->shutdown();

    // We must close all active connections before we actually exit the thread. This prevents any
    // destructors from running on the main thread which might reference thread locals. Destroying
    // the handler does this which additionally purges the dispatcher delayed deletion list.
/*    handler_.reset();
    tls_.shutdownThread();
    watch_dog_.reset();*/
}

void WorkerImpl::start() {
    threadPtr_ = std::make_unique<muduo::Thread>(
            [this]() { threadRoutine(cb_); }, name_);
    threadPtr_->start();
}


