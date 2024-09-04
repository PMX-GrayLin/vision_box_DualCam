#pragma once

#ifndef __RESTFUL_CLIENT_H__
#define __RESTFUL_CLIENT_H__

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>


class RestfulClient {

public:
    RestfulClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);

        AddHeader("Content-Type: application/json");
        //AddHeader_Img("Content-Type: opencv/image"); // use strange type to prevent server to parse content
        AddHeader_Img("Content-Type: image/jpeg"); 
    }

    ~RestfulClient() {

        if (headers_Img) {
            curl_slist_free_all(headers_Img);
            headers_Img = nullptr;
        }

        if (headers) {
            curl_slist_free_all(headers);
            headers = nullptr;
        }

        curl_global_cleanup();
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void AddHeader(const std::string& header) {
        if (headers == nullptr) {
            headers = curl_slist_append(headers, header.c_str());
        }
    }

    void AddHeader_Img(const std::string& header) {
        if (headers_Img == nullptr) {
            headers_Img = curl_slist_append(headers_Img, header.c_str());
            headers_Img = curl_slist_append(headers_Img, "Expect:");
        }
    }
    
    bool Get(const std::string& url, std::string& response) {
        return performRequest(url, "GET", "", response);
    }

    bool Post(const std::string& url, const std::string& data, std::string& response) {
        return performRequest(url, "POST", data, response);
    }

    bool Put(const std::string& url, const std::string& data, std::string& response) {
        return performRequest(url, "PUT", data, response);
    }

    bool Del(const std::string& url, std::string& response) {
        return performRequest(url, "DELETE", "", response);
    }

    bool postImage(const std::string& url, const cv::Mat& image, std::string& response) {

        string strRet = "";
        std::vector<uchar> buf;
        std::vector<int> params{ cv::IMWRITE_JPEG_QUALITY, 30 };

        cv::imencode(".jpg", image, buf, params);

        char* memblock = reinterpret_cast<char*>(buf.data());
        long size = buf.size();
        response = send_char_array(url, memblock, size);

        return 0;
    }

private:
    struct curl_slist* headers = nullptr;
    struct curl_slist* headers_Img = nullptr;

    bool performRequest(const std::string& url, const std::string& method, const std::string& data, std::string& response) {

        CURL* curl = curl_easy_init();
        if (!curl) {
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());    //for rextyw debug
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        if (method == "POST" || method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, nullptr);    
        curl_easy_cleanup(curl);

        return (res == CURLE_OK);
    }

    string send_char_array(const std::string& url, const char* memblock, const long size)
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        static int cnt = 0;

        curl = curl_easy_init();

        if (curl) {

            //curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8000/streaming");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            if(headers_Img) {
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_Img);
            }

            // data
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, memblock);
            // data length
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);

            // response handler
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);

            /* Check for errors */
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
        }

        return readBuffer;
    }

};


class CurlHandler {
public:
    CurlHandler(const std::string& url) : readBuffer("") {

        curl_global_init(CURL_GLOBAL_DEFAULT);

        if (headers_Img == nullptr) {
            headers_Img = curl_slist_append(headers_Img, "Content-Type: image/jpeg");
            headers_Img = curl_slist_append(headers_Img, "Expect:");
        }

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            // Other setting...
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_Img);
        }

    }

    ~CurlHandler() {
        
        if (headers_Img) {
            curl_slist_free_all(headers_Img);
            headers_Img = nullptr;
        }
        if (curl) {
            curl_easy_cleanup(curl);
            curl = nullptr;
        }
        curl_global_cleanup();
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    bool postImage(const cv::Mat& image, std::string& response) {

        string strRet = "";
        std::vector<uchar> buf;
        std::vector<int> params{ cv::IMWRITE_JPEG_QUALITY, 30 };

        cv::imencode(".jpg", image, buf, params);

        char* memblock = reinterpret_cast<char*>(buf.data());
        long size = buf.size();
        response = send_char_array(memblock, size);

        return 0;
    }

    string send_char_array(const char* memblock, const long size) {
        if (curl && memblock && size > 0) {

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, memblock);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);

            CURLcode res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                // �i�઺�i�@�B���~�B�z...
            }
        }

        return readBuffer;
    }

private:
    CURL* curl;
    string readBuffer;
    struct curl_slist* headers_Img = nullptr;

};



//int main() {
//    RestClient rest_client;
//
//    rest_client.AddHeader("Content-Type: application/json");
//
//    std::string get_response;
//    if (rest_client.Get("https://api.example.com/data", get_response)) {
//        std::cout << "GET Response: " << get_response << std::endl;
//    }
//    else {
//        std::cout << "GET Request Failed" << std::endl;
//    }
//
//    std::string post_data = R"({"key": "value"})";
//    std::string post_response;
//    if (rest_client.Post("https://api.example.com/post", post_data, post_response)) {
//        std::cout << "POST Response: " << post_response << std::endl;
//    }
//    else {
//        std::cout << "POST Request Failed" << std::endl;
//    }
//
//    return 0;
//}


// Reciver
//from flask import Flask, request, jsonify
//
//app = Flask(__name__)
//
//@app.route('/data', methods = ['GET'])
//def get_data() :
//    return jsonify({ 'message': 'Hello, this is the response for GET request' })
//
//    @app.route('/post', methods = ['POST'])
//    def post_data() :
//    data = request.get_json()
//    return jsonify({ 'message': f"Received POST request with data: {data}" })
//
//    if __name__ == '__main__' :
//        app.run(host = '0.0.0.0', port = 5000)



#endif  //__RESTFUL_CLIENT_H__