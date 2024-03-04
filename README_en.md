# ChohoCloud C++ sample

Please note: This code is provided solely as a programming reference under C++, utilizing the lightweight CPR library. When used in real scenarios, you should call the HTTP request functions from the library you use (such as Boost / QT).

This code compiles successfully in an Ubuntu 20.04 environment; however, adjustments may be necessary for other environments (such as Windows).

**For server deployment, polling should not be used in the production environment** to determine the completion of tasks as shown in the sample code. Instead, **callbacks should be used** to receive task completion information (i.e., configure the `notification` field in the task startup information).

We do not recommend embedding API call information in publicly distributed client software. Users might illegally obtain API account information through reverse engineering of your software, resulting in charges added to your bill for these API calls. We recommend implementing the API call logic on the server side, and clients can interact with Chohocloud services through your server. You can authenticate users on the server side.

To get started quickly or view more algorithm invocation examples, we suggest starting with our Python sample to understand HTTP request methods and request parameters: [https://gitee.com/chohotech/api_python_sample](https://gitee.com/chohotech/api_python_sample) (Github: [https://github.com/choho-tech/api_python_sample](https://github.com/choho-tech/api_python_sample))

## Compilation Requirements

- CMake >= 3.15
- g++, gcc 8+ (C++ standard 17)

## Third-party Libraries Used

- [RapidJson 1.1.0](https://github.com/Tencent/rapidjson)
- [cpr 1.5.0](https://github.com/whoshuu/cpr)

## Compilation Steps

Users need USER_ID, USER_TOKEN, SERVER_URL, and FILE_SERVER_URL for compilation. If you don't have these, please contact us to obtain them.

```bash
mkdir build && cd build
cmake .. -DUSER_ID=<Your_USER_ID> -DUSER_TOKEN=<Your_ZH_TOKEN> -DSERVER_URL=<BASE_URL> -DFILE_SERVER_URL=<FILE_SERVER_URL>
make
```

Please note that if you change USER_ID, USER_TOKEN, SERVER_URL, or FILE_SERVER_URL, you need to delete the entire build folder and recompile; otherwise, CMakeCache may cause issues.

If you encounter compilation issues, please refer to our [CI instructions](.travis.yml).

## Example

- This example demonstrates:
  1. How to create a new task JSON
  2. How to create a new task on the server
  3. How to query task status from the server and wait for task completion
  4. How to retrieve task results
  5. How to parse task results
- The core function of the example is `segment_jaw` in `seg.cpp`. Please note that while we demonstrate how to perform a segmentation task here, other tasks follow similar patterns, and users can easily adapt them with simple modifications.
- The `main` function in this example demonstrates how to segment an STL file and save the results to a user-specified folder.

## Example Usage

1. Rename an STL file to be segmented as `l.stl` (if it's mandibular) or `u.stl` (if it's maxillary).
2. After compilation, there will be an executable `seg` file in the `build` directory. Execute `./seg <path_to_stl> <path_to_result_dir>`.
3. Once the command completes, the `result_dir` will contain preprocessed meshes corresponding to the segmented results. Note that you do not need to create `result_dir` manually; if the folder doesn't exist, it will be created automatically. If you reuse `result_dir`, previous results will be overwritten.

## Code License

This repository is open source under the AGPL v3.0 license. If you use code from this repository in your project, you must provide the source code to users (including SaaS users). If you are a paying customer of Chohotech, this code is licensed to you according to our subscription agreement, and you are not obligated to comply with the AGPL v3.0 open-source license.
