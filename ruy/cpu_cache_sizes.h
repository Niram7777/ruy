/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef RUY_RUY_CPU_CACHE_SIZES_H_
#define RUY_RUY_CPU_CACHE_SIZES_H_

namespace ruy {

struct CpuCacheSizes {
  // Minimum value, over all cores, of the size in bytes of the last data
  // cache level that is local to each core i.e. not shared with other cores.
  // Example: if L1 and L2 are local and L3 is shared, this is the L2 size. If
  // some cores have L2=128k and others have L2=256k, then this is 128k. The
  // implementation may choose to ignore a cache if it's suspected not to have
  // compelling performance.
  int local = 0;
  // Minimum value, over all cores, of the size in bytes of the last-level data
  // cache of each core. Example: if L1 and L2 are local and L3 is shared, this
  // is the L3 size. The implementation may choose to ignore a cache if it's
  // suspected not to have compelling performance.
  int last_level = 0;
};

}  // namespace ruy

#endif  // RUY_RUY_CPU_CACHE_SIZES_H_
