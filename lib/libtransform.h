//
// Created by triil on 1/6/2025.
//
#pragma once

// #define RAYGUI_IMPLEMENTATION //declare once for entire project
#include "torch/script.h"
#include "torch/torch.h"
#include "element.h"
#include <opencv2/opencv.hpp>


namespace libtrh{

    struct OutputValue {
        at::Tensor logit;
        at::Tensor prediction;
        at::Tensor probability;
        OutputValue(): logit(at::empty(0)), prediction(at::empty(0)), probability(at::empty(0)) {}
    };

    class LibTransform {
        elm::PathHandler get_data;
        OutputValue output_val;
        cv::Mat image{};
        at::Tensor tensor_img{at::ones(0)};
        at::Tensor mean_;
        at::Tensor std_;
        std::vector<torch::jit::IValue> inputs;
        at::Tensor output{at::ones(0)};
        std::shared_ptr<torch::jit::Module> module;
    public:
        bool LOADED{false};
        LibTransform();
        LibTransform& operator()(elm::PathHandler &data);
        LibTransform& toCvImage(); //get image and change to CV format
        LibTransform& resizeImage(); //resize image
        LibTransform& reverseImageForm(); //from BGR to RGB if needit
        LibTransform& toRangeTensor(); //change image format to tensor and change value range 0-1
        LibTransform& normalization();
        LibTransform& toInputValue(); //yang cocok dengan input ke model libtorch
        std::vector<torch::jit::IValue> getInput();
        int loadModel();
        void forward();
        [[nodiscard]] at::Tensor getRawOutput() const;
        [[nodiscard]] std::vector<std::array<float, 2>> getStdOutput() const;
        void testInput() const;
        ~LibTransform();
    protected:
        std::vector<std::array<float, 2>> tensorToVFloat() const;

    };
}