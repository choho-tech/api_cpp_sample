#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>

#include <cpr/cpr.h>
#include "base64.h"
#include "rapidjson/document.h"
#include "rapidjson/allocators.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;
namespace fs = std::filesystem;

string dump_json(Document &doc)
{
  StringBuffer buffer;

  buffer.Clear();

  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);

  return string( buffer.GetString() );
}

void add_string_member(Document &doc, const string &key, const string &val){
    auto& doc_allocator = doc.GetAllocator();
    Value v;
    v.SetString(val.c_str(), doc_allocator);
    Value k;
    k.SetString(key.c_str(), doc_allocator);
    doc.AddMember(k, v, doc_allocator);
}

// This is a thread-safe function. You can start multiple threads and execute this function
bool segment_jaw(const string &stl_file_path, char jaw_type, string &stl_, vector<int> &label_,
                string &error_msg_){
    /* This is the function to segment a jaw using ChohoTech Cloud Service.

        Input:
            stl_file_path: path to the stl file
            jaw_type: must be either "L" or "U", standing for Lower Jaw and Upper Jaw
        Output:
            stl_: string containing preprocessed mesh data in STL format. This can directly be saved as *.stl file
            label_: segmentation labels corresponding to the output stl_
            error_msg_: error message if job failed
        Returns:
            boolean: true - job successful and results saved to stl_ and label_. false - check error_msg_ for error message

       NOTE: if return value is false, stl_, label_is meaningless, DO NOT USE!!!
    */

    // Step 1. make input
    ifstream infile(stl_file_path, ifstream::binary);
    if (!infile.is_open()) {
        cerr << "Could not open the file - '"
             << stl_file_path << "'" << endl;
        exit(EXIT_FAILURE);
    }

    string buffer((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();

    Document input_data(kObjectType);

    add_string_member(input_data, "mesh", base64_encode(buffer));
    add_string_member(input_data, "jaw_type", (jaw_type=='L')?"Lower":"Upper");


    Document request_body(kObjectType);

    auto& request_body_allocator = request_body.GetAllocator();
    add_string_member(request_body, "spec_group", "mesh-processing");
    add_string_member(request_body, "spec_name", "oral-seg");
    add_string_member(request_body, "spec_version", "1.0-snapshot");
    add_string_member(request_body, "user_group", "APIClient");
    add_string_member(request_body, "user_id", USER_ID);
    request_body.AddMember(
        "input_data",
        input_data,
        request_body_allocator);

    // Step 2. submit job
    cpr::Response r = cpr::Post(cpr::Url{string(SERVER_URL) + "/run"},
                                cpr::Body{dump_json(request_body)},
                                cpr::Header{{"Content-Type", "application/json"}, {"X-ZH-TOKEN", string(USER_TOKEN)}},
                                cpr::VerifySsl(0) // do not add this line in production for safty reason
                                );

    if (r.status_code > 300) {
        error_msg_ = "job creation request failed with error code: " + to_string(r.status_code);
        return false;
    }

    Document document;
    document.Parse(r.text.c_str());
    string job_id = document["run_id"].GetString();

    cout << "run id is: " << job_id << endl;


    // Step 3. check job
    bool status = false;

    while(!status){
        this_thread::sleep_for(chrono::milliseconds(3000)); // sleep 3s
        cpr::Response r_stat = cpr::Get(cpr::Url{string(SERVER_URL) + "/run/" + job_id},
                                        cpr::Header{{"X-ZH-TOKEN", USER_TOKEN}}, cpr::VerifySsl(0));
        if (r_stat.status_code > 300) {
            error_msg_ = "job status request failed with error code: " + to_string(r_stat.status_code);
            return false;
        }

        Document document_stat;
        document_stat.Parse(r_stat.text.c_str());

        status = document_stat["failed"].GetBool();
        if(status){
            error_msg_ = string("job failed with error: ") + document_stat["reason_public"].GetString();
            return false;
        }

        status = document_stat["completed"].GetBool();
    }

    // Step 4. get job result
    cpr::Response r_result = cpr::Get(cpr::Url{string(SERVER_URL) + "/data/" + job_id},
                                      cpr::Header{{"X-ZH-TOKEN", USER_TOKEN}}, cpr::VerifySsl(0));


    if (r_result.status_code > 300) {
        error_msg_ = "job result request failed with error code: " + to_string(r_result.status_code);
        return false;
    }

    // Step 5. parse result

    Document document_result;
    document_result.Parse(r_result.text.c_str());

    stl_ = base64_decode(string(document_result["mesh"].GetString()));

    label_.clear();
    for (auto& v : document_result["seg_labels"].GetArray()) label_.push_back(v.IsInt()?v.GetInt(): (int)(v.GetDouble() + 0.1));

    return true;
}

int main(int argc,char *argv[]){
    if(argc < 3) {
        cout << "Usage: ./seg PATH_TO_STL PATH_TO_RESULT_DIR" << endl;
        return 1;
    }
    string stl_path = string(argv[1]);
    char jaw_type = string(fs::path( stl_path ).filename())[0];

    if(jaw_type == 'L' || jaw_type == 'l'){
        jaw_type = 'L';
    } else if (jaw_type == 'U' || jaw_type == 'u'){
        jaw_type = 'U';
    } else{
        cout << "STL file name must be either u.stl for upper jaw or l.stl for lower jaw" << endl;
        return 1;
    }

    string result_stl, error_msg;
    vector<int> result_label;

    if(!segment_jaw(stl_path, jaw_type, result_stl, result_label, error_msg)){
        cout<< error_msg <<endl;
        return 1;
    }

    auto result_dir_path = fs::path( string(argv[2]) );

    if (!fs::is_directory(result_dir_path)) {
        fs::create_directory(result_dir_path);
    }

    ofstream ofs;
    ofs.open (result_dir_path / "result_mesh.stl", ofstream::out | ofstream::binary);
    ofs << result_stl;
    ofs.close();

    ofs.open (result_dir_path / "result_label.txt", ofstream::out);
    for (const auto &e : result_label) ofs << e << endl;
    ofs.close();

    ofs.close();

    return 0;
}