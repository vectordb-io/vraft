#!/usr/bin/env python
# coding: utf-8

# In[6]:


import os
from openai import OpenAI

client = OpenAI(
    api_key=os.getenv("DASHSCOPE_API_KEY"),
    base_url="https://dashscope.aliyuncs.com/compatible-mode/v1"
)

data2="客户端内存需求原生连接方式由于客户端应用程序采用 taosc 与服务器进行通信，因此会产生一定的内存消耗。这些内存消耗主要源于：写入操作中的 SQL、表元数据信息的缓存，以及固有的结构开销。假设该数据库服务能够支持的最大表数量为 N（每个通过超级表创建的表的元数据开销约为 256B），最大并发写入线程数为 T，以及最大 SQL 语句长度为 S（通常情况下为1MB）。基于这些参数，我们可以对客户端的内存消耗进行估算（单位为 MB）。 M = (T × S × 3 + (N / 4096) + 100)例如，用户最大并发写入线程数为 100，子表数为 10 000 000，那么客户端的内存最低要求如下： 100 × 3 + (10000000 / 4096) + 100 ≈ 2841 (MB) 即配置 3GB 内存是最低要求。RESTful/WebSocket 连接方式当将 WebSocket 连接方式用于数据写入时，如果内存占用不大，通常可以不予关注。然而，在执行查询操作时，WebSocket 连接方式会消耗一定量的内存。接下来，我们将详细讨论查询场景下的内存使用情况。当客户端通过 WebSocket 连接方式发起查询请求时，为了接收并处理查询结果，必须预留足够的内存空间。得益于 WebSocket 连接方式的特性，数据可以分批次接收和解码，这样就能够在确保每个连接所需内存固定的同时处理大量数据。计算客户端内存占用的方法相对简单：只须将每个连接所需的读 / 写缓冲区容量相加即可。通常每个连接会额外占用 8MB 的内存。因此，如果有 C 个并发连接，那么总的额 外内存需求就是 8×C（单位 MB）。例如，如果用户最大并发连接数为 10，则客户端的额外内存最低要求是 80（8×10）MB。与 WebSocket 连接方式相比，RESTful 连接方式在内存占用上更大，除了缓冲区所需的内存以外，还需要考虑每个连接响应结果的内存开销。这种内存开销与响应结果的JSON 数据大小密切相关，特别是在查询数据量很大时，会占用大量内存。由于 RESTful 连接方式不支持分批获取查询数据，这就导致在查询获取超大结果集时，可能会占用特别大的内存，从而导致内存溢出，因此，在大型项目中，建议打开batchfetch=true 选项，以启用 WebSocket 连接方式，实现流式结果集返回，从而避免内存溢出的风险注意建议采用 RESTful/WebSocket 连接方式来访问 TDengine 集群，而不采用taosc 原生连接方式。在绝大多数情形下，RESTful/WebSocket 连接方式均满足业务写入和查询要求，并且该连接方式不依赖于 taosc，集群服务器升级与客户端连接方式完全解耦，使得服务器维护、升级更容易。CPU 需求TDengine 用户对 CPU 的需求主要受以下 3 个因素影响：数据分片：在 TDengine 中，每个 CPU 核心可以服务 1 至 2 个 vnode。假设一个集群配置了 100 个 vgroup，并且采用三副本策略，那么建议该集群的 CPU 核心数量为 150~300 个，以实现最佳性能。数据写入：TDengine 的单核每秒至少能处理 10,000 个写入请求。值得注意的是，每个写入请求可以包含多条记录，而且一次写入一条记录与同时写入 10 条记录相比，消耗的计算资源相差无几。因此，每次写入的记录数越多，写入效率越高。例如，如果一个写入请求包含 200 条以上记录，单核就能实现每秒写入 100 万条记录的速度。然而，这要求前端数据采集系统具备更高的能力，因为它需要缓存记录，然后批量写入。查询需求：虽然 TDengine 提供了高效的查询功能，但由于每个应用场景的查询差异较大，且查询频次也会发生变化，因此很难给出一个具体的数字来衡量查询所需的计算资源。用户需要根据自己的实际场景编写一些查询语句，以便更准确地确定所需的计算资源。综上所述，对于数据分片和数据写入，CPU 的需求是可以预估的。然而，查询需求所消耗的计算资源则难以预测。在实际运行过程中，建议保持 CPU 使用率不超过 50%，以确保系统的稳定性和性能。一旦 CPU 使用率超过这一阈值，就需要考虑增加新的节点或增加 CPU 核心数量，以提供更多的计算资源。存储需求相较于传统通用数据库，TDengine 在数据压缩方面表现出色，拥有极高的压缩率。在大多数应用场景中，TDengine 的压缩率通常不低于 5 倍。在某些特定情况下，压缩率 甚至可以达到 10 倍乃至上百倍，这主要取决于实际场景中的数据特征。要计算压缩前的原始数据大小，可以采用以下方式： RawDataSize = numOfTables × rowSizePerTable × rowsPerTable示例：1000 万块智能电表，电表每 15min 采集一次数据，每次采集的数据量为 20B，那么一年的原始数据量约 7TB。TDengine 大概需要消耗 1.4TB 存储空间。为了迎合不同用户在数据存储时长及成本方面的个性化需求，TDengine 赋予了用户极大的灵活性，用户可以通过一系列数据库参数配置选项来定制存储策略。其中，keep参数尤其引人注目，它赋予用户自主设定数据在存储空间上的最长保存期限的能力。这一功能设计使得用户能够依据业务的重要性和数据的时效性，精准调控数据的存储生命周期，进而实现存储成本的精细化控制。然而，单纯依赖 keep 参数来优化存储成本仍显不足。为此，TDengine 进一步创新，推出了多级存储策略。此外，为了加速数据处理流程，TDengine 特别支持配置多块硬盘，以实现数据的并发写入与读取。这种并行处理机制能够最大化利用多核 CPU 的处理能力和硬盘 I/O 带宽，大幅提高数据传输速度，完美应对高并发、大数据量的应用场景挑战。技巧 如何估算 TDengine 压缩率用户可以利用性能测试工具 taosBenchmark 来评估 TDengine 的数据压缩效果。通过使用 -f 选项指定写入配置文件，taosBenchmark 可以将指定数量的 CSV 样例数据写入指定的库参数和表结构中。在完成数据写入后，用户可以在 taos shell 中执行 flush database 命令，将所有数据强制写入硬盘。接着，通过 Linux 操作系统的 du 命令获取指定 vnode 的数据文件夹大小。最后，将原始数据大小除以实际存储的数据大小，即可计算出真实的压缩率。通过如下命令可以获得 TDengine 占用的存储空间。"
data = "TDengine一个cpu可以处理几个vnode？"
query="根据如下内容：“" + data2 + "” " + "回答问题： “" + data + "”"

print(query)

completion = client.chat.completions.create(
    model="qwen-max", # 更多模型请参见模型列表文档。
    messages = [{"role": "user", "content": query}]
)

print("大模型回答：")
print(completion.choices[0].message.content)
