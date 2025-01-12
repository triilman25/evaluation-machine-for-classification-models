//
// Created by triil on 1/6/2025.
//
#include "libtransform.h"


namespace libtrh {
    LibTransform::LibTransform():
    get_data("","","",0),
    mean_(at::tensor({0.485, 0.456, 0.406}).view({3,1,1})), std_(at::tensor({0.229, 0.224, 0.225}).view({3,1,1}))
    {}

    LibTransform & LibTransform::operator()(elm::PathHandler &data) {
        this->get_data = std::move(data);
        return *this;
    }

    LibTransform & LibTransform::toCvImage() {
        // this->LOADED = true;
        this->image = cv::imread(this->get_data.image);
        if (image.empty()) {
            std::cerr << "Failed to load image: " << image << "\n";
            return *this;
        }
        return *this;
    }

    LibTransform & LibTransform::resizeImage() {
        cv::resize(this->image, this->image, cv::Size(this->get_data.spinner_v, this->get_data.spinner_v));

        return *this;
    }

    LibTransform & LibTransform::reverseImageForm() {
        cv::cvtColor(this->image, this->image, cv::COLOR_BGR2RGB);
        return *this;
    }

    LibTransform & LibTransform::toRangeTensor() {
        this->tensor_img = torch::from_blob(
            this->image.data,
            {this->image.rows, this->image.cols, 3},
            torch::kByte
            ).permute({2,0,1}).to(torch::kFloat).div(255.0);

        this->tensor_img = this->tensor_img.unsqueeze(0);
        std::cout <<"Input size: "<< this->tensor_img.dtype()<<" "<<tensor_img.sizes() << std::endl;
        return *this;
    }

    LibTransform & LibTransform::normalization() {
        this->tensor_img = (this->tensor_img - mean_)/std_; //z_transform
        return *this;
    }

    LibTransform & LibTransform::toInputValue() {
        this->inputs = std::vector<torch::jit::IValue>{std::move(tensor_img).to(torch::kFloat)}; //casting
        std::cout <<"Input torch: " <<this->inputs.size() << "\n";
        return *this;
    }

    std::vector<torch::jit::IValue> LibTransform::getInput() {
        return this->inputs;
    }

    int LibTransform::loadModel() {
        try {
            // Load Model
             module = std::make_shared<torch::jit::Module>(torch::jit::load(get_data.module));
            this->module->to(torch::kFloat);
            // std::cout << "load model : successfully (libtransform)"<<std::endl;
        }
        catch (const c10::Error& e) {
            std::cerr << "error loading the model\n"<<e.msg();
            return -1;
        }catch (const std::exception& e) {
            std::cerr << "Unexpected error: " << e.what() << "\n";
            return -1;
        }
        return 0;
    }

    void LibTransform::forward() {
        std::cout << "Input Tensor Shape: " << this->inputs.size() << "\n";
        try {
            this->output = this->module->forward(this->inputs).toTensor();
            std::cout << "Probability: " << torch::softmax(this->output, 1) << "\n";
            std::cout << "Probability: " << torch::argmax(this->output, 1) << "\n";
        } catch (const c10::Error& e) {
            std::cerr << "Inference error: " << e.what() << std::endl;
        }
        this->LOADED = false;
    }

    at::Tensor LibTransform::getRawOutput() const {

        return this->output;
    }

    std::vector<std::array<float, 2>> LibTransform::getStdOutput() const {
        return tensorToVFloat();
    }

    void LibTransform::testInput() const {
        std::cout << "image path: "<< get_data.image << std::endl;
        std::cout << "model path: "<< get_data.module << std::endl;
        std::cout << "Input size: "<< get_data.spinner_v << std::endl;
    }

    LibTransform::~LibTransform() = default;

    std::vector<std::array<float, 2>> LibTransform::tensorToVFloat() const {
        auto local_pred = torch::argmax(this->output, 1).item<int>();
        auto local_prob = torch::softmax(this->output, 1);
        std::vector<std::array<float, 2>> prob_return;
        std::cout << local_prob.size(1) << "\n";
        for (int i =0; i < local_prob.size(1); i++) {
            auto logit = std::move(local_prob[0][i].item<float>());
            if (i == local_pred) {
                prob_return.push_back({static_cast<float>(local_pred),logit});

            }else {

                prob_return.push_back({-1,logit});
            }
        }

        return prob_return;
    }
}
