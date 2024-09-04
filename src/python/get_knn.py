import sys
import os

# 将 vdb_module.so 的目录添加到 sys.path
sys.path.append('/home/mhlee/Work/dev/vraft/output/libs')

import vdb_module

# 创建 VdbEngine 实例
engine = vdb_module.VdbEngine("/tmp/local_console")

table = "test-table"
vec = [0.172457, 0.383009, 0.255386, 0.016210, 0.705780, 0.920516, 0.678624, 0.796226, 0.115947, 0.185988]  # 示例向量
limit = 5

res_code, results = engine.get_knn(table, vec, limit)

print("Response code:", res_code)
for result in results:
    print(result.to_print_string())
