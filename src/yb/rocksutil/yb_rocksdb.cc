// Copyright (c) YugaByte, Inc.


#include <string>

#include "yb/gutil/strings/substitute.h"
#include "yb/rocksutil/yb_rocksdb.h"
#include "yb/rocksutil/yb_rocksdb_logger.h"

using std::shared_ptr;
using std::string;
using strings::Substitute;

using namespace rocksdb;

namespace yb {

void InitRocksDBOptions(rocksdb::Options *options,
                        const string& tablet_id,
                        const shared_ptr<rocksdb::Statistics>& statistics) {
  options->create_if_missing = true;
  options->disableDataSync = true;
  options->statistics = statistics;
  options->info_log = std::make_shared<YBRocksDBLogger>(Substitute("T $0: ", tablet_id));
  options->info_log_level = YBRocksDBLogger::ConvertToRocksDBLogLevel(FLAGS_minloglevel);
  options->set_last_seq_based_on_sstable_metadata = true;
}

}