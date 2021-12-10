#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include<zip.h>
#include <cpr/cpr.h>
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;
namespace fs = std::filesystem;

// This is a thread-safe function. You can start multiple threads and execute this function
bool segment_jaw(const string &stl_file_path, char jaw_type, string &obj_, vector<int> &label_,
                 vector<vector<double>> &align_matrix_, string &error_msg_){
    /* This is the function to segment a jaw using DeepAlign Cloud Service.

        Input:
            stl_file_path: path to the stl file
            jaw_type: must be either "L" or "U", standing for Lower Jaw and Upper Jaw
        Output:
            obj_: string containing preprocessed mesh data in OBJ format. This can directly be saved as *.obj file
            label_: segmentation labels corresponding to the output obj_
            align_matrix_: align matrix of the jaw mesh - align to DeepAlign coordinate.
            error_msg_: error message if job failed
        Returns:
            boolean: true - job successful and results saved to obj_ and label_. false - check error_msg_ for error message

       NOTE: if return value is false, obj_, label_, align_matrix_ is meaningless, DO NOT USE!!!
    */

    // Step 1. create a zip file containing the mesh
    int errorp;
    string zip_name = tmpnam(nullptr);

    zip_t *zipper = zip_open(zip_name.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (zipper == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        error_msg_ = "Failed to open temp file " + zip_name + ": " + zip_error_strerror(&ziperror);
        return false;
    }

    zip_source_t *source = zip_source_file(zipper, stl_file_path.c_str(), 0, 0);
    if (source == nullptr) {
        error_msg_ = "Failed to add file to zip: " + string(zip_strerror(zipper));
        zip_close(zipper);
        return false;
    }

    if (zip_file_add(zipper, "mesh.stl", source, ZIP_FL_ENC_UTF_8) < 0) {
      zip_source_free(source);
      error_msg_ = "Failed to add file to zip: " + string(zip_strerror(zipper));
      zip_close(zipper);
      return false;
    }

    zip_close(zipper);

    // Step 2. submit job
    cpr::Response r = cpr::Post(cpr::Url{SERVER_URL},
                                cpr::Multipart{
                                    {"Jaw", (jaw_type=='L')?"Lower":"Upper"},
                                    {"Preprocess", "true"},
                                    {"StlBinary", cpr::File{zip_name}}
                                },
                                cpr::Authentication{APP_ID, APP_CODE});

    // delete zip file
    remove( zip_name.c_str() );

    if (r.status_code > 300) {
        error_msg_ = "job creation request failed with error code: " + to_string(r.status_code);
        return false;
    }

    Document document;
    document.Parse(r.text.c_str());
    string job_id = document["JobId"].GetString();


    // Step 3. check job
    string status = "Pending";

    while(status == "InProgress" || status == "Pending"){
        this_thread::sleep_for(chrono::milliseconds(30000)); // sleep 30s
        cpr::Response r_stat = cpr::Get(cpr::Url{string(SERVER_URL) + "/" + job_id},
                                        cpr::Authentication{APP_ID, APP_CODE});
        if (r_stat.status_code > 300) {
            error_msg_ = "job status request failed with error code: " + to_string(r_stat.status_code);
            return false;
        }

        Document document_stat;
        document_stat.Parse(r_stat.text.c_str());

        status = document_stat["SegmentationJob"]["JobStatus"].GetString();
        if(status == "Failed"){
            error_msg_ = string("job failed with error: ") + document_stat["SegmentationJob"]["Message"].GetString();
            return false;
        }
    }

    // Step 4. get job result
    cpr::Response r_result = cpr::Get(cpr::Url{string(SERVER_URL) + "/" + job_id + "/result"},
                                      cpr::Authentication{APP_ID, APP_CODE});

    if (r_result.status_code > 300) {
        error_msg_ = "job result request failed with error code: " + to_string(r_result.status_code);
        return false;
    }

    // Step 5. parse result

    Document document_result;
    document_result.Parse(r_result.text.c_str());

    obj_ = document_result["Mesh"].GetString();

    label_.clear();
    for (auto& v : document_result["Result"].GetArray()) label_.push_back(v.IsInt()?v.GetInt(): (int)(v.GetDouble() + 0.1));

    align_matrix_.clear();
    string temp_str = "";
    vector<double> temp_vec;
    string align_mat_str = document_result["AlignMatrix"].GetString();
    for (const char& c : align_mat_str){
        if(c=='[' || c==' ') continue;
        if(c==',' || c==']'){
            if(temp_str != ""){
                temp_vec.push_back(stod(temp_str));
                temp_str = "";
                if(c==']'){
                    align_matrix_.push_back(temp_vec);
                    temp_vec.clear();
                }
            }
            continue;
        }
        temp_str += c;
    }

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

    string result_obj, error_msg;
    vector<int> result_label;
    vector<vector<double>> result_align_matrix;

    if(!segment_jaw(stl_path, jaw_type, result_obj, result_label, result_align_matrix, error_msg)){
        cout<< error_msg <<endl;
        return 1;
    }

    auto result_dir_path = fs::path( string(argv[2]) );

    if (!fs::is_directory(result_dir_path)) {
        fs::create_directory(result_dir_path);
    }

    ofstream ofs;
    ofs.open (result_dir_path / "result_mesh.obj", ofstream::out | ofstream::binary);
    ofs << result_obj;
    ofs.close();

    ofs.open (result_dir_path / "result_label.txt", ofstream::out);
    for (const auto &e : result_label) ofs << e << " ";
    ofs.close();

    ofs.open (result_dir_path / "result_align_matrix.txt", ofstream::out);
    for (const auto &row : result_align_matrix){
        for (const double &e: row) ofs << e << " ";
        ofs << "\n";
    }

    ofs.close();

    return 0;
}