#!/bin/bash 

set -euxo pipefail

project_dir=$( cd `dirname $0` && pwd )
build_dir="$project_dir/build/debug"

mkdir -p "$build_dir"
cd "$build_dir"

# Even though thirdparty/build-if-necessary.sh has its own build stamp file,
# the logic here is simplified: we only build third-party dependencies once and
# never rebuild it.

thirdparty_built_flag_file="$PWD/built_thirdparty"

if [ -f "$thirdparty_built_flag_file" ]; then
  export NO_REBUILT_THIRDPARTY=1
fi

if [ ! -f Makefile ]; then
  cmake -DKUDU_LINK=dynamic "$project_dir"
fi

make -j8

touch "$thirdparty_built_flag_file"

