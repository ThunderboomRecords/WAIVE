#ifndef MODEL_UTILS_HPP
#define MODEL_UTILS_HPP

#include <vector>
#include <iostream>

#include "onnxruntime_cxx_api.h"

std::vector<std::vector<int64_t>> GetInputShapes(const std::unique_ptr<Ort::Session>& session);
std::vector<std::vector<int64_t>> GetOutputShapes(const std::unique_ptr<Ort::Session>& session);
std::vector<std::string> GetInputNames(const std::unique_ptr<Ort::Session>& session);
std::vector<std::string> GetOutputNames(const std::unique_ptr<Ort::Session>& session);

void PrintModelDetails(
    std::string modelName, 
    std::vector<std::string> inputNames, 
    std::vector<std::string> outputNames,
    std::vector<std::vector<int64_t>> inputShape,
    std::vector<std::vector<int64_t>> outputShape
);

#endif