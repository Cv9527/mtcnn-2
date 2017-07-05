#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "caffe.pb.h"
#include <fstream>  // NOLINT(readability/streams)
#include "caffe.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/text_format.h>
#include <sys/stat.h>
#include <iosfwd>
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

using namespace caffe;
using namespace std;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::GzipOutputStream;
using google::protobuf::Message;

using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#endif


struct info{
	int kernel_size;
	int num_output;
	int isfc;

	info(){ memset(this, 0, sizeof(*this)); }
	info(int kernelsize, int numoutput) :kernel_size(kernelsize), num_output(numoutput){}
};

int pos = 0;
void writeHand(FILE* f, const char* name){
	fprintf(f, "//--------------------------%s---------------------------------\nstatic const float model_weights_%s_[] = {", name, name);
	pos = 0;
}

void writeData(FILE* f, const float* data, int len){
	for (int i = 0; i < len; ++i, ++pos){
		fprintf(f, "%f,", data[i]);

		if (pos >= 15){
			pos = 0;
			fprintf(f, "\n");
		}
	}
}

void writeEnd(FILE* f){
	fprintf(f, "};\n\n\n");
}

bool loadDep(const char* file, Message* net){
	int fd = _open(file, O_RDONLY);
	if (fd == 0) return false;

	FileInputStream* input = new FileInputStream(fd);
	bool success = google::protobuf::TextFormat::Parse(input, net);
	delete input;
	_close(fd);
	return success;
}

bool loadCaffemodel(const char* file, Message* net){
	int fd = _open(file, O_RDONLY | O_BINARY);
	if (fd == 0) return false;

	ZeroCopyInputStream* raw_input = new FileInputStream(fd);
	CodedInputStream* coded_input = new CodedInputStream(raw_input);
	bool success = net->ParseFromCodedStream(coded_input);
	delete coded_input;
	delete raw_input;
	_close(fd);
	return success;
}

void main(){
	//12 P
	//24 R
	//48 O
	vector<string> names = { "PNet", "RNet", "ONet" };

	//注意这里的3个模型不支持原生mtcnn训练的模型，因为原生模型是matlab训练的，有转置，所以直接套用到mtcnn-light时会无效
	const char* pnet = "det1.caffemodel";
	const char* rnet = "det2.caffemodel";
	const char* onet = "det3.caffemodel";

	vector<string> caffemodel = {pnet, rnet, onet};
	FILE* fmodel = fopen("mtcnn_models.h", "wb");
	fprintf(fmodel, 
		"#ifndef MTCNN_MODELS_H\n"
		"#define MTCNN_MODELS_H\n"
		"\n\n\n");
	for (int i = 0; i < caffemodel.size(); ++i){
		writeHand(fmodel, names[i].c_str());
		printf("=======================%s===========================\n", names[i].c_str());
		NetParameter net;
		bool success = loadCaffemodel(caffemodel[i].c_str(), &net);
		if (!success){
			printf("读取错误啦.\n");
			return;
		}

		for (int i = 0; i < net.layer_size(); ++i){
			LayerParameter& param = *net.mutable_layer(i);
			int n = param.mutable_blobs()->size();
			if (n){
				const BlobProto& blob = param.blobs(0);
				printf("layer: %s weight(%d)", param.name().c_str(), blob.data_size());
				writeData(fmodel, blob.data().data(), blob.data_size());

				if (n > 1){
					const BlobProto& bais = param.blobs(1);
					printf(" bais(%d)", bais.data_size());
					writeData(fmodel, bais.data().data(), bais.data_size());
				}
				printf("\n");
			}
		}
		writeEnd(fmodel);
	}
	fprintf(fmodel, "#endif //MTCNN_MODELS_H");
	fclose(fmodel);
}