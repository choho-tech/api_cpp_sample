# 朝厚云服务C++调用示例

请注意： 本份代码仅供在C++下的编程参考，使用了轻量级的CPR库。真实使用时应调用您使用的库中的HTTP请求函数（如 boost / QT）

本份代码在Ubuntu 20.04环境下编译成功，其他环境（如windows）可能需要一定调整

**如果是服务器部署，生产上不应该使用示例代码中的轮询**判断任务完成部分，
而**应该通过回调**获取任务完成信息（即配置启动任务信息中的 `notification` 字段）。

我们不建议您将API调用信息写入公开发行的客户端软件。
用户可能通过逆向您的软件非法获得API账号信息进行调用，这些调用次数产生的费用将添加到您的账单中。
我们建议您将API调用部分逻辑写在服务器端，客户端通过您的服务器调用朝厚云服务。您可以在服务器端对用户鉴权。

想要快速开始或者查看更多算法调用示例？建议先使用我们的Python样例了解http请求方法以及请求参数： https://gitee.com/chohotech/api_python_sample

## 编译要求

- CMake >= 3.15
- g++, gcc 8+ (C++ standard 17)

## 使用的第三方库

- [RapidJson 1.1.0](https://github.com/Tencent/rapidjson)
- [cpr 1.5.0](https://github.com/whoshuu/cpr)

## 编译步骤

用户需要 USER_ID, USER_TOKEN, SERVER_URL 和 FILE_SERVER_URL 用于编译，如果您没有这些, 请联系我们获取

```bash
mkdir build && cd build
cmake .. -DUSER_ID=<您的USER_ID> -DUSER_TOKEN=<您的ZH_TOKEN> -DSERVER_URL=<BASE_URL> -DFILE_SERVER_URL=<FILE_SERVER_URL>
make
```

请注意，如果您更改USER_ID, USER_TOKEN, SERVER_URL 或 FILE_SERVER_URL, 您需要删除整个build文件夹并重新编译, 否则CMakeCache会造成问题。

如果您编译遇到问题，请参考我们的[CI指令](.travis.yml)

## 样例

- 本样例展示了
  1. 如何新建任务JSON
  2. 如何向服务器新建任务
  3. 如何向服务器查询任务状态并等待任务完成
  4. 如何获取任务结果
  5. 如何解析任务结果
- 样例的核心函数是seg.cpp中的segment_jaw. 请注意，这里我们展示了如何进行分牙任务，但是其他任务大同小异，用户经过简单的修改即可使用
- 本样例的main函数展示的是如何将一个STL文件进行切分并将结果存入用户指定的文件夹

## 样例使用

1. 将一个待分牙的STL文件重命名为l.stl (如果是下颌) 或 u.stl (如果是上颌)
2. 在编译完成后，`build` 目录里会有一个 `seg` 可执行文件, 执行 `./seg <path_to_stl> <path_to_result_dir>`
3. 命令完成后，`result_dir`内会有预处理后的网格，对应分牙结果。请注意，您不需要手动创建`result_dir`, 如果该文件夹不存在，它会自动被创建。如果您重复使用`result_dir`, 以前的结果会被覆盖。

## 代码许可

本仓库基于AGPL v3.0许可开源，如果您在项目中使用本仓库的代码，则您的项目必须向用户（包括SaaS用户）提供源代码。如果您是朝厚的付费用户，此份代码将根据我们的订阅用户协议向您授权，您没有遵守AGPL v3.0开源协议的义务。