// Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//

#ifndef YB_RPC_CONNECTION_CONTEXT_H
#define YB_RPC_CONNECTION_CONTEXT_H

#include "yb/rpc/rpc_fwd.h"
#include "yb/rpc/rpc_introspection.pb.h"

#include "yb/util/result.h"
#include "yb/util/strongly_typed_bool.h"
#include "yb/util/net/socket.h"

namespace yb {

class MemTracker;

namespace rpc {

typedef std::function<void()> IdleListener;
YB_STRONGLY_TYPED_BOOL(ReadBufferFull);

class GrowableBufferAllocator;

// ConnectionContext class is used by connection for doing protocol
// specific logic.
class ConnectionContext {
 public:
  virtual ~ConnectionContext() {}

  // Split data into separate calls and invoke them.
  // Returns number of processed bytes.
  virtual Result<size_t> ProcessCalls(const ConnectionPtr& connection,
                                      const IoVecs& data,
                                      ReadBufferFull read_buffer_full) = 0;

  // Dump information about status of this connection context to protobuf.
  virtual void DumpPB(const DumpRunningRpcsRequestPB& req, RpcConnectionPB* resp) = 0;

  // Checks whether this connection context is idle.
  virtual bool Idle() = 0;

  // Returns a human-readable description of why the context is not idle.
  virtual std::string ReasonNotIdle() = 0;

  // Listen for when context becomes idle.
  virtual void ListenIdle(IdleListener listener) = 0;

  // Shutdown this context.
  virtual void Shutdown(const Status& status) = 0;

  // Reading buffer limit for this connection context.
  // The reading buffer will never be larger than this limit.
  virtual size_t BufferLimit() = 0;

  virtual GrowableBufferAllocator& Allocator() = 0;

  virtual void QueueResponse(const ConnectionPtr& connection, InboundCallPtr call) = 0;

  virtual void AssignConnection(const ConnectionPtr& connection) {}

  virtual void Connected(const ConnectionPtr& connection) = 0;

  virtual uint64_t ProcessedCallCount() = 0;

  virtual RpcConnectionPB::StateType State() = 0;
};

class ConnectionContextBase : public ConnectionContext {
 public:
  explicit ConnectionContextBase(GrowableBufferAllocator* allocator)
      : allocator_(allocator) {
  }

  GrowableBufferAllocator& Allocator() override {
    return *allocator_;
  }

 private:
  GrowableBufferAllocator* allocator_;
};

class ConnectionContextFactory {
 public:
  ConnectionContextFactory(
      size_t block_size, size_t memory_limit,
      const std::shared_ptr<MemTracker>& parent_mem_tracker);

  virtual std::unique_ptr<ConnectionContext> Create() = 0;

  GrowableBufferAllocator& allocator() {
    return *allocator_;
  }

  const std::shared_ptr<MemTracker>& parent_tracker() {
    return parent_tracker_;
  }

 protected:
  ~ConnectionContextFactory() {}

  std::unique_ptr<GrowableBufferAllocator> allocator_;
  std::shared_ptr<MemTracker> parent_tracker_;
  std::shared_ptr<MemTracker> call_tracker_;
};

template <class ContextType>
class ConnectionContextFactoryImpl : public ConnectionContextFactory {
 public:
  ConnectionContextFactoryImpl(
      size_t block_size, size_t memory_limit,
      const std::shared_ptr<MemTracker>& parent_mem_tracker = nullptr)
      : ConnectionContextFactory(block_size, memory_limit, parent_mem_tracker) {}

  std::unique_ptr<ConnectionContext> Create() override {
    return std::make_unique<ContextType>(allocator_.get(), call_tracker_);
  }

  virtual ~ConnectionContextFactoryImpl() {}
};

} // namespace rpc
} // namespace yb

#endif // YB_RPC_CONNECTION_CONTEXT_H
